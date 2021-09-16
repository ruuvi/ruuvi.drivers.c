/**
 * Copyright (c) 2016 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * Ruuvi BLE GATT profile implementation
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_communication_ble_gatt.h"
#if RUUVI_NRF5_SDK15_GATT_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_interface_flash.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_timer.h"
#include "ruuvi_interface_power.h"
#include "ruuvi_interface_yield.h"

#include "app_timer.h"
#include "ble_advdata.h"
#include "ble_types.h"
#include "ble_conn_params.h"
#include "ble_dfu.h"
#include "ble_dis.h"
#include "ble_nus.h"
#include "fds.h"
#include "sdk_errors.h"
#include "nrf_error.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_qwr.h"
#include "nrf_ble_gatt.h"
#include "peer_manager.h"

#include <stdio.h>
#include <string.h>

#define CENTRAL_LINK_COUNT               0                     /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                     /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define APP_BLE_OBSERVER_PRIO            3                     /**< Application's BLE observer priority. You shouldn't need to modify this value. */

#define NEXT_CONN_PARAMS_UPDATE_DELAY_MS      (30000) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY_TICKS  APP_TIMER_TICKS(5000)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY_TICKS   APP_TIMER_TICKS(NEXT_CONN_PARAMS_UPDATE_DELAY_MS) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */

#define MAX_CONN_PARAMS_UPDATE_COUNT     3                     /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   0                     /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                     /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                     /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                     /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE  /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                     /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                     /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                    /**< Maximum encryption key size. */

#ifndef RUUVI_NRF5_SDK15_COMMUNICATION_BLE4_GATT_LOG_LEVEL
#define RUUVI_NRF5_SDK15_COMMUNICATION_BLE4_GATT_LOG_LEVEL RI_LOG_LEVEL_DEBUG
#endif
#define LOG(msg) ri_log(RUUVI_NRF5_SDK15_COMMUNICATION_BLE4_GATT_LOG_LEVEL, msg)
#define LOGD(msg) ri_log(RI_LOG_LEVEL_DEBUG, msg)
#define LOGW(msg) ri_log(RI_LOG_LEVEL_WARNING, msg)
#define LOGHEX(msg, len) ri_log_hex(RUUVI_NRF5_SDK15_COMMUNICATION_BLE4_GATT_LOG_LEVEL, msg, len)

APP_TIMER_DEF (
    m_conn_param_retry_timer); //<! Timer for retrying comm param renegotiation.

NRF_BLE_GATT_DEF (m_gatt); /**< GATT module instance. */
NRF_BLE_QWR_DEF (m_qwr);   /**< Context for the Queued Write module.*/
BLE_NUS_DEF (m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT); /**< BLE NUS service instance. */
static uint16_t m_conn_handle =
    BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */
static bool     m_gatt_is_init = false;
/**< Pointer to application communication interface, given at initialization */
static ri_comm_channel_t * channel = NULL;

static ble_gap_conn_params_t m_gatt_params; //!< Holder for GATT params.

static uint8_t m_gatt_retries;

/** @brief Supported PHYs, start at 1MBPS */
static ble_gap_phys_t m_phys =
{
    .rx_phys = BLE_GAP_PHY_1MBPS, //BLE_GAP_PHY_2MBPS, BLE_GAP_PHY_CODED
    .tx_phys = BLE_GAP_PHY_1MBPS
};

/** @brief print PHY enum as string */
static char const * phy_str (ble_gap_phys_t phys)
{
    static char const * str[] =
    {
        "1 Mbps",
        "2 Mbps",
        "Coded",
        "Unknown"
    };

    switch (phys.tx_phys)
    {
        case BLE_GAP_PHY_1MBPS:
            return str[0];

        case BLE_GAP_PHY_2MBPS:
        case BLE_GAP_PHY_2MBPS | BLE_GAP_PHY_1MBPS:
        case BLE_GAP_PHY_2MBPS | BLE_GAP_PHY_1MBPS | BLE_GAP_PHY_CODED:
            return str[1];

        case BLE_GAP_PHY_CODED:
            return str[2];

        default:
            return str[3];
    }
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions.
 */
static ret_code_t gap_params_init (void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN (&sec_mode);
    memset (&gap_conn_params, 0, sizeof (gap_conn_params));
    gap_conn_params.min_conn_interval = MSEC_TO_UNITS (RI_GATT_MIN_INTERVAL_STANDARD_MS,
                                        UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS (RI_GATT_MAX_INTERVAL_STANDARD_MS,
                                        UNIT_1_25_MS);
    gap_conn_params.slave_latency     = RI_GATT_SLAVE_LATENCY_STANDARD;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS (RI_GATT_CONN_SUP_TIMEOUT_MS,
                                        UNIT_10_MS);
    err_code = sd_ble_gap_ppcp_set (&gap_conn_params);
    return err_code;
}

/** @brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler (uint32_t nrf_error)
{
    RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_error),
                    RD_SUCCESS);
}

/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt (ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect (m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code),
                        RD_SUCCESS);
    }
}


/**@brief Function for initializing the Connection Parameters module.
 */
static ret_code_t conn_params_init (void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;
    memset (&cp_init, 0, sizeof (cp_init));
    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY_TICKS;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY_TICKS;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    err_code = ble_conn_params_init (&cp_init);
    return err_code;
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler (ble_nus_evt_t * p_evt)
{
    if (NULL == channel || NULL == channel->on_evt)
    {
        return;
    }

    switch (p_evt->type)
    {
        case BLE_NUS_EVT_RX_DATA:
            LOGD ("NUS RX \r\n");
            channel->on_evt (RI_COMM_RECEIVED,
                             (void *) p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
            break;

        case BLE_NUS_EVT_COMM_STARTED:
            LOG ("NUS Started\r\n");
            channel->on_evt (RI_COMM_CONNECTED, NULL, 0);
            break;

        case BLE_NUS_EVT_COMM_STOPPED:
            LOG ("NUS Finished\r\n");
            channel->on_evt (RI_COMM_DISCONNECTED, NULL, 0);
            break;

        case BLE_NUS_EVT_TX_RDY:
            LOGD ("NUS TX Done\r\n");
            channel->on_evt (RI_COMM_SENT, NULL, 0);
            break;

        default:
            break;
    }
}

/**@brief Function for handling BLE events.
 *
 * TODO: Error handling
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 *
 */
static void ble_evt_handler (ble_evt_t const * p_ble_evt, void * p_context)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign (&m_qwr, m_conn_handle);
            LOG ("BLE Connected \r\n");
            char msg[128];
            sprintf (msg, "PHY: %s.\r\n", phy_str (m_phys));
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code),
                            RD_SUCCESS);
#           if 0
            // Request preferred PHY on connection -
            // Crashes connection on older iOS devices and Macs.
            err_code = sd_ble_gap_phy_update (p_ble_evt->evt.gap_evt.conn_handle, &m_phys);
            char msg[128];
            snprintf (msg, sizeof (msg), "Request PHY update to %s.\r\n", phy_str (m_phys));
            LOG (msg);
#           endif
            break;

        case BLE_GAP_EVT_TIMEOUT:
            LOG ("BLE GAP timeout \r\n");
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            LOG ("BLE Disconnected \r\n");
            // ble_nus.c does not call NUS callback on disconnect, call it here.
            ble_nus_evt_t evt = { 0 };
            evt.type = BLE_NUS_EVT_COMM_STOPPED;
            nus_data_handler (&evt);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            err_code |= app_timer_stop (m_conn_param_retry_timer);
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code),
                            RD_SUCCESS);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
            err_code = sd_ble_gap_phy_update (p_ble_evt->evt.gap_evt.conn_handle, &m_phys);
            LOG ("BLE PHY update requested \r\n");
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code),
                            RD_SUCCESS);
            break;

        case BLE_GAP_EVT_PHY_UPDATE:
        {
            ble_gap_evt_phy_update_t const * p_phy_evt = &p_ble_evt->evt.gap_evt.params.phy_update;

            if (p_phy_evt->status == BLE_HCI_STATUS_CODE_LMP_ERROR_TRANSACTION_COLLISION)
            {
                LOGW ("BLE LL transaction collision during PHY update.\r\n");
                break;
            }

            ble_gap_phys_t evt_phys = {0};
            evt_phys.tx_phys = p_phy_evt->tx_phy;
            evt_phys.rx_phys = p_phy_evt->rx_phy;
            char msg[128];
            snprintf (msg, sizeof (msg), "PHY update %s. PHY set to %s.\r\n",
                      (p_phy_evt->status == BLE_HCI_STATUS_CODE_SUCCESS) ?
                      "accepted" : "rejected",
                      phy_str (evt_phys));
            LOG (msg);
        }
        break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            LOG ("BLE security parameters requested - denying \r\n");
            err_code = sd_ble_gap_sec_params_reply (m_conn_handle,
                                                    BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                                    NULL, NULL);
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code),
                            RD_SUCCESS);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            LOG ("BLE System attributes missing\r\n");
            err_code = sd_ble_gatts_sys_attr_set (m_conn_handle, NULL, 0, 0);
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code),
                            RD_SUCCESS);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            LOG ("BLE GATT Client timeout\r\n");
            err_code = sd_ble_gap_disconnect (p_ble_evt->evt.gattc_evt.conn_handle,
                                              BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code),
                            RD_SUCCESS);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            LOG ("BLE GATT Server timeout\r\n");
            err_code = sd_ble_gap_disconnect (p_ble_evt->evt.gatts_evt.conn_handle,
                                              BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code),
                            RD_SUCCESS);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            LOGD ("BLE Notification sent\r\n");
            break;

        default:
            // No implementation needed.
            LOGD ("BLE Unknown event\r\n");
            break;
    }
}

/**@brief Function for handling events from the GATT library. */
static void gatt_evt_handler (nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ( (m_conn_handle == p_evt->conn_handle)
            && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        LOGD ("Changing MTU size is not supported\r\n");
    }
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler (uint32_t nrf_error)
{
    RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_error),
                    RD_SUCCESS);
}

// YOUR_JOB: Update this code if you want to do anything given a DFU event (optional).
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler (ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
            // YOUR_JOB: Disconnect all bonded devices that currently are connected.
            //           This is required to receive a service changed indication
            //           on bootup after a successful (or aborted) Device Firmware Update.
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        default:
            break;
    }
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler (pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply (p_evt->conn_handle, &conn_sec_config);
        }
        break;

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            err_code = fds_gc();

            if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK (err_code);
            }
        }
        break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (
                                p_evt->params.peer_data_update_failed.error), RD_SUCCESS);
        }
        break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (
                                p_evt->params.peer_delete_failed.error), RD_SUCCESS);
        }
        break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (
                                p_evt->params.peers_delete_failed_evt.error), RD_SUCCESS);
        }
        break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (
                                p_evt->params.error_unexpected.error), RD_SUCCESS);
        }
        break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}



/**
 * @brief Function for the Peer Manager initialization.
 */
static ret_code_t peer_manager_init()
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code = NRF_SUCCESS;
    err_code |= pm_init();

    // Failed because storage cannot be initialized.
    if (NRF_SUCCESS != err_code)
    {
        ri_flash_purge();
        ri_power_reset();
    }

    memset (&sec_param, 0, sizeof (ble_gap_sec_params_t));
    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 0;
    sec_param.kdist_own.id   = 0;
    sec_param.kdist_peer.enc = 0;
    sec_param.kdist_peer.id  = 0;
    err_code = pm_sec_params_set (&sec_param);
    err_code |= pm_register (pm_evt_handler);
    return err_code;
}

/**
 * @brief Configure preferred PHYs.
 *
 * If peer doesn't support given PHY, softdevice
 * selects automatically PHY both devices are compatible with.
 */
static rd_status_t setup_phys (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_radio_modulation_t modulation;
    err_code |= ri_radio_get_modulation (&modulation);

    if (RD_SUCCESS == err_code)
    {
        if (ri_radio_supports (modulation))
        {
            switch (modulation)
            {
                case RI_RADIO_BLE_125KBPS:
                    m_phys.rx_phys = BLE_GAP_PHY_CODED;
                    m_phys.tx_phys = BLE_GAP_PHY_CODED;
                    break;

                case RI_RADIO_BLE_2MBPS:
                    m_phys.rx_phys = BLE_GAP_PHY_2MBPS;
                    m_phys.tx_phys = BLE_GAP_PHY_2MBPS;
                    break;

                case RI_RADIO_BLE_1MBPS:
                default:
                    m_phys.rx_phys = BLE_GAP_PHY_1MBPS;
                    m_phys.tx_phys = BLE_GAP_PHY_1MBPS;
                    break;
            }
        }
        else
        {
            err_code |= RD_ERROR_NOT_SUPPORTED;
        }
    }

    return err_code;
}

static void gatt_params_request (void * const params)
{
    ret_code_t nrf_status = NRF_SUCCESS;

    if (BLE_CONN_HANDLE_INVALID == m_conn_handle)
    {
        m_gatt_retries = 0;
        LOG ("No connection, ignore param change\r\n");
    }
    else
    {
        nrf_status = ble_conn_params_change_conn_params (m_conn_handle, params);
        LOG ("Requesting param change\r\n");
    }

    if (NRF_SUCCESS != nrf_status)
    {
        nrf_status = app_timer_start (m_conn_param_retry_timer,
                                      NEXT_CONN_PARAMS_UPDATE_DELAY_TICKS,
                                      params);
        RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_status),
                        RD_SUCCESS);
        m_gatt_retries++;
        LOG ("Param change failed, retry queued\r\n");
    }

    if (m_gatt_retries > MAX_CONN_PARAMS_UPDATE_COUNT)
    {
        // Something is preventing update, disconnect.
        nrf_status = sd_ble_gap_disconnect (m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_status),
                        RD_SUCCESS);
        LOG ("Param change errored too many times, cut connection\r\n");
    }
}

rd_status_t ri_gatt_init (void)
{
    ret_code_t err_code = NRF_SUCCESS;
    static bool qwr_is_init = false;

    if (m_gatt_is_init)
    {
        return RD_ERROR_INVALID_STATE;
    }

    if (!ri_radio_is_init())
    {
        return RD_ERROR_INVALID_STATE;
    }

    // Connection param module requires timers
    if (!ri_timer_is_init())
    {
        LOGW ("NRF5 SDK15 BLE4 GATT module requires initialized timers\r\n");
        return RD_ERROR_INVALID_STATE;
    }
    else
    {
        // Running timer cannot be recreated.
        // Stopped timer can be created again, so stop, ignore error, (re)create
        (void) app_timer_stop (m_conn_param_retry_timer);
        err_code |= app_timer_create (&m_conn_param_retry_timer, APP_TIMER_MODE_SINGLE_SHOT,
                                      &gatt_params_request);
        RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code), ~RD_ERROR_FATAL);
    }

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER (m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
    err_code |= gap_params_init();
    err_code |= nrf_ble_gatt_init (&m_gatt, gatt_evt_handler);
    err_code |= nrf_ble_gatt_att_mtu_periph_set (&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code), ~RD_ERROR_FATAL);
    // Queued Write Module, peer manager cannot be uninitialized, initialize only once.

    if (!qwr_is_init)
    {
        nrf_ble_qwr_init_t qwr_init = {0};
        qwr_init.error_handler = nrf_qwr_error_handler;
        err_code |= nrf_ble_qwr_init (&m_qwr, &qwr_init);
        err_code |= peer_manager_init();
        RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code), ~RD_ERROR_FATAL);
        qwr_is_init = true;
    }

    err_code |= conn_params_init();
    RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code), ~RD_ERROR_FATAL);
    err_code |= setup_phys();
    RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code), ~RD_ERROR_FATAL);

    if ( (NRF_SUCCESS == err_code))
    {
        m_gatt_is_init = true;
    }

    RD_ERROR_CHECK (ruuvi_nrf5_sdk15_to_ruuvi_error (err_code), ~RD_ERROR_FATAL);
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

rd_status_t ri_gatt_uninit (void)
{
    rd_status_t err_code = RD_SUCCESS;

    // Radio must be completely disabled to uninit GATT.
    if (ri_radio_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    // GATT is uninit when radio is uninit.
    else
    {
        m_gatt_is_init = false;
    }

    return err_code;
}

rd_status_t ri_gatt_nus_uninit (ri_comm_channel_t * const _channel)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == _channel)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        memset (_channel, 0, sizeof (ri_comm_channel_t));

        // disconnect
        if (BLE_CONN_HANDLE_INVALID != m_conn_handle)
        {
            err_code |= sd_ble_gap_disconnect (m_conn_handle,
                                               BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
        }
    }

    // Services cannot be uninitialized, GATT must be re-initialized.
    return err_code;
}

/**
 *
 */
static rd_status_t ri_gatt_nus_send (ri_comm_message_t * const message)
{
    rd_status_t err_code = RD_SUCCESS;
    ret_code_t nrf_code = NRF_SUCCESS;

    if (NULL == message)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (BLE_NUS_MAX_DATA_LEN < message->data_length)
    {
        err_code |= RD_ERROR_DATA_SIZE;
    }
    else if (BLE_CONN_HANDLE_INVALID == m_conn_handle)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (message->repeat_count > 1)
    {
        err_code |= RD_ERROR_NOT_IMPLEMENTED;
    }
    else
    {
        uint16_t data_len = message->data_length;
        nrf_code |= ble_nus_data_send (&m_nus, message->data, &data_len, m_conn_handle);
    }

    return err_code | ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_code);
}

static rd_status_t ri_gatt_nus_read (ri_comm_message_t * const message)
{
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_gatt_nus_init (ri_comm_channel_t * const _channel)
{
    if (NULL == _channel) { return RD_ERROR_NULL; }

    uint32_t           err_code;
    ble_nus_init_t     nus_init;
    // Initialize NUS.
    memset (&nus_init, 0, sizeof (nus_init));
    nus_init.data_handler = nus_data_handler;
    err_code = ble_nus_init (&m_nus, &nus_init);
    channel = _channel;
    channel->init   = ri_gatt_nus_init;
    channel->uninit = ri_gatt_nus_uninit;
    channel->send   = ri_gatt_nus_send;
    channel->read   = ri_gatt_nus_read;
    m_gatt_is_init = true;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

bool ble_nus_is_connected (void)
{
    ret_code_t err_code = NRF_SUCCESS;
    ble_nus_client_context_t * p_client;
    err_code = blcm_link_ctx_get (m_nus.p_link_ctx_storage, m_conn_handle,
                                  (void *) &p_client);

    if (NRF_SUCCESS != err_code)
    {
        return false;
    }

    if ( (m_conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return false;
    }

    if (!p_client->is_notification_enabled)
    {
        return false;
    }

    return true;
}

rd_status_t ri_gatt_dfu_init (void)
{
    ret_code_t err_code = NRF_SUCCESS;
    ble_dfu_buttonless_init_t dfus_init = {0};
    // Initialize the async SVCI interface to bootloader.
    err_code = ble_dfu_buttonless_async_svci_init();

    if (NRF_SUCCESS != err_code) {}

    dfus_init.evt_handler = ble_dfu_evt_handler;
    err_code = ble_dfu_buttonless_init (&dfus_init);

    if (NRF_SUCCESS != err_code) {}

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

/**
 * @brief Initialize BLE Device Information Service
 *
 * @param[in] dis pointer to data which should be presented over DIS. Memory will be deep-copied
 */
rd_status_t ri_gatt_dis_init (const ri_comm_dis_init_t * const p_dis)
{
    if (NULL == p_dis) { return RD_ERROR_NULL; }

    ret_code_t err_code = NRF_SUCCESS;
    ble_dis_init_t dis_init;
    ri_comm_dis_init_t dis_local;
    memset (&dis_init, 0, sizeof (dis_init));
    memcpy (&dis_local, p_dis, sizeof (dis_local));
    ble_srv_ascii_to_utf8 (&dis_init.manufact_name_str, dis_local.manufacturer);
    ble_srv_ascii_to_utf8 (&dis_init.model_num_str, dis_local.model);
    ble_srv_ascii_to_utf8 (&dis_init.serial_num_str, dis_local.deviceid);
    ble_srv_ascii_to_utf8 (&dis_init.hw_rev_str, dis_local.hw_version);
    ble_srv_ascii_to_utf8 (&dis_init.fw_rev_str, dis_local.fw_version);
    // Read security level 1, mode 1. OPEN, i.e. anyone can read without encryption.
    // Write not allowed.
    dis_init.dis_char_rd_sec = SEC_OPEN;
    err_code = ble_dis_init (&dis_init);
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

rd_status_t ri_gatt_params_request (const ri_gatt_params_t params,
                                    const uint16_t delay_ms)
{
    rd_status_t err_code = RD_SUCCESS;
    ret_code_t nrf_status = NRF_SUCCESS;
    ble_gap_conn_params_t gap_conn_params = {0};
    gap_conn_params.conn_sup_timeout = MSEC_TO_UNITS (RI_GATT_CONN_SUP_TIMEOUT_MS,
                                       UNIT_10_MS);

    switch (params)
    {
        case RI_GATT_TURBO:
            LOG ("RI_GATT_TURBO\r\n");
            gap_conn_params.slave_latency = RI_GATT_SLAVE_LATENCY_TURBO;
            gap_conn_params.min_conn_interval = MSEC_TO_UNITS (RI_GATT_MIN_INTERVAL_TURBO_MS,
                                                UNIT_1_25_MS);
            gap_conn_params.max_conn_interval = MSEC_TO_UNITS (RI_GATT_MAX_INTERVAL_TURBO_MS,
                                                UNIT_1_25_MS);
            break;

        case RI_GATT_LOW_POWER:
            LOG ("RI_GATT_LOW_POWER\r\n");
            gap_conn_params.slave_latency = RI_GATT_SLAVE_LATENCY_LOW_POWER;
            gap_conn_params.min_conn_interval = MSEC_TO_UNITS (RI_GATT_MIN_INTERVAL_LOW_POWER_MS,
                                                UNIT_1_25_MS);
            gap_conn_params.max_conn_interval = MSEC_TO_UNITS (RI_GATT_MAX_INTERVAL_LOW_POWER_MS,
                                                UNIT_1_25_MS);
            break;

        case RI_GATT_STANDARD:
        default:
            LOG ("RI_GATT_STANDARD\r\n");
            gap_conn_params.slave_latency = RI_GATT_SLAVE_LATENCY_STANDARD;
            gap_conn_params.min_conn_interval = MSEC_TO_UNITS (RI_GATT_MIN_INTERVAL_STANDARD_MS,
                                                UNIT_1_25_MS);
            gap_conn_params.max_conn_interval = MSEC_TO_UNITS (RI_GATT_MAX_INTERVAL_STANDARD_MS,
                                                UNIT_1_25_MS);
            break;
    }

    err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (app_timer_stop (m_conn_param_retry_timer));
    memcpy (&m_gatt_params, &gap_conn_params, sizeof (gap_conn_params));

    if (0 == delay_ms)
    {
        nrf_status = ble_conn_params_change_conn_params (m_conn_handle, &gap_conn_params);

        if (NRF_SUCCESS != nrf_status)
        {
            err_code |=  ruuvi_nrf5_sdk15_to_ruuvi_error (app_timer_start (m_conn_param_retry_timer,
                         NEXT_CONN_PARAMS_UPDATE_DELAY_TICKS, &m_gatt_params));
            m_gatt_retries = 1;
            RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
            LOG ("Retrying new params soon\r\n");
        }
        else
        {
            LOG ("Switched to new params\r\n");
        }
    }
    else
    {
        err_code |=  ruuvi_nrf5_sdk15_to_ruuvi_error (app_timer_start (m_conn_param_retry_timer,
                     APP_TIMER_TICKS (delay_ms), &m_gatt_params));
        m_gatt_retries = 0;
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        LOG ("New params set after delay\r\n");
    }

    return err_code;
}

#endif