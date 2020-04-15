/**
 * @file ruuvi_nrf5_sdk15_communication_radio.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-09-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Commmon definitions and functions for all radio operations.
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_communication_radio.h"
#if RUUVI_NRF5_SDK15_RADIO_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#include "ruuvi_interface_communication_ble_gatt.h"


#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdm.h"
#include "ble_advdata.h"
#include "ble_radio_notification.h"
#include "sdk_errors.h"

/** @brief Application callback for radio events */
static ri_radio_activity_interrupt_fp_t on_radio_activity_callback = NULL;
static ri_radio_modulation_t m_modulation;
static ri_radio_modulation_t m_modulation; //<! Modulation for radio ops.

/**
 * @brief Task to run on radio activity
 * Calls event handlers of radio modules and a common radio event handler.
 * This function is in interrupt context, avoid long processing or using peripherals.
 * Schedule any long tasks in application callbacks.
 *
 * @param[in] active True if radio is going to be active after event, false if radio was turned off (after tx/rx)
 */
static void on_radio_evt (bool active)
{
    // Convert to Ruuvi enum
    ri_radio_activity_evt_t evt = active ?
                                  RI_RADIO_BEFORE : RI_RADIO_AFTER;

    // Call common event handler if set
    if (NULL != on_radio_activity_callback) { on_radio_activity_callback (evt); }
}

rd_status_t ri_radio_init (const ri_radio_modulation_t modulation)
{
    rd_status_t status = RD_SUCCESS;
    ret_code_t err_code = NRF_SUCCESS;

    if (ri_radio_is_init())
    {
        status |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        err_code = nrf_sdh_enable_request();
        RD_ERROR_CHECK (err_code, NRF_SUCCESS);
        // Configure the BLE stack using the default settings.
        // Fetch the start address of the application RAM.
        uint32_t ram_start = 0;
        // As of SD 6.1.1, only one advertising configuration is allowed.
        err_code |= nrf_sdh_ble_default_cfg_set (RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG,
                    &ram_start);
        RD_ERROR_CHECK (err_code, NRF_SUCCESS);
        // TODO - find the correct way to define large enough GATT queue for extended GATT event.
        ble_cfg_t conn_cfg = { 0 };
        conn_cfg.conn_cfg.conn_cfg_tag = RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG;
        err_code |= sd_ble_cfg_set (BLE_CONN_CFG_GATTS, &conn_cfg, ram_start);
        // Enable BLE stack.
        err_code |= nrf_sdh_ble_enable (&ram_start);
        // Enable connection event extension for faster data rate
        static ble_opt_t  opt = {0};
        opt.common_opt.conn_evt_ext.enable = true;
        err_code |= sd_ble_opt_set (BLE_COMMON_OPT_CONN_EVT_EXT, &opt);
        RD_ERROR_CHECK (err_code, NRF_SUCCESS);
        // Initialize radio interrupts
        err_code |= ble_radio_notification_init (RUUVI_NRF5_SDK15_RADIO_IRQ_PRIORITY,
                    NRF_RADIO_NOTIFICATION_DISTANCE_800US,
                    on_radio_evt);
        // Store desired modulation
        m_modulation = modulation;
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code) | status;
}

rd_status_t ri_radio_uninit (void)
{
    nrf_sdh_disable_request();
    on_radio_activity_callback = NULL;
    return RD_SUCCESS;
}

rd_status_t ri_radio_address_get (uint64_t * const address)
{
    uint32_t status = NRF_SUCCESS;
    rd_status_t err_code = RD_SUCCESS;

    if (!ri_radio_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        uint64_t mac = 0;
        ble_gap_addr_t addr;
        addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
        status |= sd_ble_gap_addr_get (&addr);
        mac |= (uint64_t) (addr.addr[5]) << 40;
        mac |= (uint64_t) (addr.addr[4]) << 32;
        mac |= (uint64_t) (addr.addr[3]) << 24;
        mac |= (uint64_t) (addr.addr[2]) << 16;
        mac |= (uint64_t) (addr.addr[1]) << 8;
        mac |= (uint64_t) (addr.addr[0]) << 0;
        *address = mac;
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (status) | err_code;
}

rd_status_t ri_radio_address_set (const uint64_t address)
{
    ble_gap_addr_t addr;
    addr.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
    addr.addr[0] = address >> 0;
    addr.addr[1] = address >> 8;
    addr.addr[2] = address >> 16;
    addr.addr[3] = address >> 24;
    addr.addr[4] = address >> 32;
    addr.addr[5] = address >> 40;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (sd_ble_gap_addr_set (&addr));
}


void ri_radio_activity_callback_set (const ri_radio_activity_interrupt_fp_t handler)
{
    // Warn user if CB is not NULL and non-null pointer is set, do not overwrite previous pointer.
    if (NULL != handler && NULL != on_radio_activity_callback)
    {
        RD_ERROR_CHECK (RD_ERROR_INVALID_STATE, ~RD_ERROR_FATAL);
    }
    else
    {
        on_radio_activity_callback = handler;
    }
}

bool ri_radio_is_init (void)
{
    return nrf_sdh_is_enabled();
}

rd_status_t ri_radio_get_modulation (ri_radio_modulation_t * const p_modulation)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == p_modulation)
    {
        err_code = RD_ERROR_NULL;
    }
    else if (!ri_radio_is_init())
    {
        err_code = RD_ERROR_INVALID_STATE;
    }
    else
    {
        *p_modulation = m_modulation;
    }

    return RD_SUCCESS;
}


uint8_t ruuvi_nrf5_sdk15_radio_phy_get (void)
{
    uint8_t nrf_modulation = BLE_GAP_PHY_NOT_SET;
    ri_radio_modulation_t modulation;
    rd_status_t err_code = ri_radio_get_modulation (&modulation);

    if (RD_SUCCESS == err_code)
    {
        switch (modulation)
        {
            case RI_RADIO_BLE_125KBPS:
                nrf_modulation = BLE_GAP_PHY_CODED;

            case RI_RADIO_BLE_1MBPS:
                nrf_modulation = BLE_GAP_PHY_1MBPS;

            case RI_RADIO_BLE_2MBPS:
                nrf_modulation = BLE_GAP_PHY_2MBPS;

            default:
                nrf_modulation = BLE_GAP_PHY_NOT_SET;
                break;
        }
    }

    return modulation;
}

void ruuvi_nrf5_sdk15_radio_channels_set (uint8_t * const nrf_channels,
        const ri_radio_channels_t channels)
{
    memset (nrf_channels, 0, sizeof (ble_gap_ch_mask_t));
    nrf_channels[4] |= (!channels.channel_37) << 5;
    nrf_channels[4] |= (!channels.channel_38) << 6;
    nrf_channels[4] |= (!channels.channel_39) << 7;
}

#endif
