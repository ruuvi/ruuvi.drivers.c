/**
 * Ruuvi BLE data advertising.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#if RUUVI_NRF5_SDK15_ADV_ENABLED

#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_nrf5_sdk15_communication_radio.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_interface_log.h"
#include "nordic_common.h"
#include "nrf_ble_scan.h"
#include "nrf_nvic.h"
#include "nrf_queue.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "ble_types.h"
#include "sdk_errors.h"
#include "nrf_log.h"

#include <stdint.h>

#ifndef RUUVI_NRF5_SDK15_ADV_LOG_LEVEL
#define LOG_LEVEL RI_LOG_LEVEL_INFO
#else
#define LOG_LEVEL RUUVI_NRF5_SDK15_ADV_LOG_LEVEL
#endif
static inline void LOG (const char * const msg)
{
    ri_log (LOG_LEVEL, msg);
}

static inline void LOGD (const char * const msg)
{
    ri_log (RI_LOG_LEVEL_DEBUG, msg);
}

static inline void LOGI (const char * const msg)
{
    ri_log (RI_LOG_LEVEL_INFO, msg);
}

static inline void LOGW (const char * const msg)
{
    ri_log (RI_LOG_LEVEL_WARNING, msg);
}

static inline void LOGE (const char * const msg)
{
    ri_log (RI_LOG_LEVEL_ERROR, msg);
}

#define DEFAULT_ADV_INTERVAL_MS      (1010U)
#define MIN_ADV_INTERVAL_MS          (100U)
#define MAX_ADV_INTERVAL_MS          (10000U)
#define NONEXTENDED_ADV_MAX_LEN      (31U)
#define NONEXTENDED_PAYLOAD_MAX_LEN  (24U)

#define APP_BLE_OBSERVER_PRIO   3U //!< Used in concat macro which fails on braces.
NRF_BLE_SCAN_DEF (m_scan);

typedef struct
{
    uint8_t adv_data[NONEXTENDED_ADV_MAX_LEN];  //!< Buffer for advertisement
    uint8_t scan_data[NONEXTENDED_ADV_MAX_LEN]; //!< Buffer for scan response.
    ble_gap_adv_data_t   data;   //!< Encoded data, both advertisement and scan response.
    ble_gap_adv_params_t params; //!< Parameters of advertisement, such as channels, PHY.
    int8_t tx_pwr;               //!< Transmission power for this advertisement.
} advertisement_t;               //!< Advertisement to be sent.

/** Create queue for outgoing advertisements. */
NRF_QUEUE_DEF (advertisement_t, m_adv_queue, RUUVI_NRF5_SDK15_ADV_QUEUE_LENGTH,
               NRF_QUEUE_MODE_NO_OVERFLOW);
/** Create queue for incoming advertisements. */
NRF_QUEUE_DEF (ri_comm_message_t, m_scan_queue, RUUVI_NRF5_SDK15_SCAN_QUEUE_LENGTH,
               NRF_QUEUE_MODE_NO_OVERFLOW);

static uint16_t
m_advertisement_interval_ms; //!< Interval of advertisements, not including random delay by BLE spec.
static uint16_t m_manufacturer_id;           //!< Manufacturer ID for advertisements.
static ri_comm_channel_t *
m_channel;        //!< Communication channel interface instance.
static int8_t m_tx_power;                    //!< Power configured to radio.
static bool m_scannable;                     //!< Should scan responses be included.
static ble_gap_conn_sec_mode_t m_security;   //!< BLE security mode.
/** @brief Human-readable device name, e.g. "Ruuvi ABCD". */
static char m_name[NONEXTENDED_PAYLOAD_MAX_LEN];
/** @brief Advertise UUID of Nordic UART Service in scan response. */
static bool m_advertise_nus;
static ri_adv_type_t m_type;                 //!< Type, configured by user.
static ri_radio_channels_t m_radio_channels; //!< Enabled channels to send
static bool m_is_rx_le_1m_phy_enabled;       //!< Is 1 MBit/s PHY enabled.
static bool m_is_rx_le_2m_phy_enabled;       //!< Is 2 MBit/s PHY enabled.
static bool m_is_rx_le_coded_phy_enabled;    //!< Is 125 kBit/s PHY enabled.
static uint8_t m_max_adv_length = 0;         //!< Maximum length of advertisement.

/** @brief Advertising handle used to identify an advertising set. */
static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
/** @brief Flag for initialization **/
static bool m_advertisement_is_init = false;
/** @brief Flag for advertising in process **/
static bool m_advertising = false;
/** @brief flag for including Ruuvi Sensor data service UUID in the advertisement **/
static bool m_include_service_uuid = false;
/** @brief 16-bit Bluetooth Service UUID to advertise, Ruuvi's UUID by default. */
static uint16_t m_service_uuid = 0xFC98;

/**< Universally unique service identifier of Nordic UART Service */
#if RUUVI_NRF5_SDK15_GATT_ENABLED
#include "ble_nus.h"
static ble_uuid_t m_adv_uuids[] =
{
    {BLE_UUID_NUS_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}
};
#endif

static rd_status_t prepare_tx()
{
    ret_code_t nrf_code = NRF_SUCCESS;

    // XXX: Use atomic compare-and-swap
    if (!nrf_queue_is_empty (&m_adv_queue) && !m_advertising)
    {
        static advertisement_t adv;
        nrf_queue_pop (&m_adv_queue, &adv);
        // Pointers have been invalidated in queuing, refresh.
        adv.data.adv_data.p_data = adv.adv_data;

        if (adv.data.scan_rsp_data.len > 0)
        {
            adv.data.scan_rsp_data.p_data = adv.scan_data;
        }

        nrf_code |= sd_ble_gap_adv_set_configure (&m_adv_handle,
                    &adv.data,
                    &adv.params);
        nrf_code |= sd_ble_gap_tx_power_set (BLE_GAP_TX_POWER_ROLE_ADV,
                                             m_adv_handle,
                                             adv.tx_pwr);
        nrf_code |= sd_ble_gap_adv_start (m_adv_handle,
                                          RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG);

        if (NRF_SUCCESS == nrf_code)
        {
            m_advertising = true;
        }
        else
        {
            // Recursion depth is limited to depth of m_adv_queue.
            prepare_tx();
        }
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_code);
}

/** @brief terminate advertising set, notify application */
static void notify_adv_stop (const ri_comm_evt_t evt)
{
    m_advertising = false; // Runs at highest app interrupt level, no need for atomic.

    if ( (NULL != m_channel)  && (NULL != m_channel->on_evt))
    {
        m_channel->on_evt (evt, NULL, 0);
    }
}

// Register a handler for advertisement events.
static void ble_advertising_on_ble_evt_isr (ble_evt_t const * p_ble_evt, void * p_context)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            nrf_queue_reset (&m_adv_queue);

            if (CONNECTABLE_SCANNABLE == m_type)
            {
                m_type = NONCONNECTABLE_SCANNABLE;
            }

            if (CONNECTABLE_NONSCANNABLE == m_type)
            {
                m_type = NONCONNECTABLE_NONSCANNABLE;
            }

            notify_adv_stop (RI_COMM_ABORTED);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            nrf_queue_reset (&m_adv_queue);
            notify_adv_stop (RI_COMM_ABORTED);
            break;

        // Upon terminated advertising (time-out), start next and notify application TX complete.
        case BLE_GAP_EVT_ADV_SET_TERMINATED:
            notify_adv_stop (RI_COMM_SENT);
            prepare_tx();
            break;

        default:
            break;
    }
}

typedef struct ble_adv_mac_addr_str_t
{
#if RI_LOG_ENABLED
    char buf[BLE_MAC_ADDRESS_LENGTH * 2 + (BLE_MAC_ADDRESS_LENGTH - 1) + 1];
#else
    char buf[1];
#endif
} ble_adv_mac_addr_str_t;

static ble_adv_mac_addr_str_t ble_adv_mac_addr_to_str (const uint8_t * const p_mac)
{
    ble_adv_mac_addr_str_t mac_addr_str = {0};
#if RI_LOG_ENABLED
    snprintf (mac_addr_str.buf, sizeof (mac_addr_str.buf),
              "%02x:%02x:%02x:%02x:%02x:%02x",
              p_mac[5], p_mac[4], p_mac[3], p_mac[2], p_mac[1], p_mac[0]);
#endif
    return mac_addr_str;
}


NRF_SDH_BLE_OBSERVER (m_ble_observer, APP_BLE_OBSERVER_PRIO,
                      ble_advertising_on_ble_evt_isr, NULL);

// Register a handler for scan events.
static void on_advertisement (scan_evt_t const * p_scan_evt)
{
    switch (p_scan_evt->scan_evt_id)
    {
        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
            LOGI ("Scan timeout\r\n");
            m_channel->on_evt (RI_COMM_TIMEOUT,
                               NULL, 0);
            break;

        // Data which matches the configured filter - todo
        case NRF_BLE_SCAN_EVT_FILTER_MATCH:
            LOGD ("Matching data\r\n");
            // p_scan_evt->params.filter_match.p_adv_report; // Data should be here
            break;

        // All the data, pass to application
        case NRF_BLE_SCAN_EVT_NOT_FOUND:
            LOGD ("Unknown data\r\n");

            if ( (NULL != m_channel)  && (NULL != m_channel->on_evt))
            {
                ri_radio_modulation_t modulation = RI_RADIO_BLE_1MBPS;
                ri_radio_get_modulation (&modulation);

                if ( (RI_RADIO_BLE_1MBPS == modulation) && (!m_is_rx_le_1m_phy_enabled)
                        && (BLE_GAP_PHY_1MBPS == p_scan_evt->params.p_not_found->primary_phy)
                        && (BLE_GAP_PHY_NOT_SET == p_scan_evt->params.p_not_found->secondary_phy))
                {
                    NRF_LOG_INFO (
                        "on_advertisement: 1M PHY disabled, discard adv from "
                        "addr=%s: len=%d, primary_phy=%d, secondary_phy=%d, chan=%d",
                        ble_adv_mac_addr_to_str (p_scan_evt->params.p_not_found->peer_addr.addr).buf,
                        p_scan_evt->params.p_not_found->data.len,
                        p_scan_evt->params.p_not_found->primary_phy,
                        p_scan_evt->params.p_not_found->secondary_phy,
                        p_scan_evt->params.p_not_found->ch_index);
                    break;
                }

                // Send advertisement report
                ri_adv_scan_t scan;
                const uint8_t max_len = m_max_adv_length ? m_max_adv_length : sizeof (scan.data);

                if (p_scan_evt->params.p_not_found->data.len > max_len)
                {
                    NRF_LOG_WARNING ("on_advertisement: discard adv from addr=%s: len=%d is too long (>%d)",
                                     ble_adv_mac_addr_to_str (p_scan_evt->params.p_not_found->peer_addr.addr).buf,
                                     p_scan_evt->params.p_not_found->data.len,
                                     max_len);
                    break;
                }

                const bool is_coded_phy = (RI_RADIO_BLE_125KBPS == modulation) ? true : false;
                NRF_LOG_INFO (
                    "on_advertisement: recv adv from addr=%s: len=%d, "
                    "is_coded_phy=%d, primary_phy=%d, secondary_phy=%d, chan=%d",
                    ble_adv_mac_addr_to_str (p_scan_evt->params.p_not_found->peer_addr.addr).buf,
                    p_scan_evt->params.p_not_found->data.len,
                    is_coded_phy,
                    p_scan_evt->params.p_not_found->primary_phy,
                    p_scan_evt->params.p_not_found->secondary_phy,
                    p_scan_evt->params.p_not_found->ch_index);
                scan.addr[0] = p_scan_evt->params.p_not_found->peer_addr.addr[5];
                scan.addr[1] = p_scan_evt->params.p_not_found->peer_addr.addr[4];
                scan.addr[2] = p_scan_evt->params.p_not_found->peer_addr.addr[3];
                scan.addr[3] = p_scan_evt->params.p_not_found->peer_addr.addr[2];
                scan.addr[4] = p_scan_evt->params.p_not_found->peer_addr.addr[1];
                scan.addr[5] = p_scan_evt->params.p_not_found->peer_addr.addr[0];
                scan.rssi    = p_scan_evt->params.p_not_found->rssi;
                scan.is_coded_phy = is_coded_phy;
                scan.primary_phy = p_scan_evt->params.p_not_found->primary_phy;
                scan.secondary_phy = p_scan_evt->params.p_not_found->secondary_phy;
                scan.ch_index = p_scan_evt->params.p_not_found->ch_index;
                scan.tx_power = p_scan_evt->params.p_not_found->tx_power;
                memcpy (scan.data, p_scan_evt->params.p_not_found->data.p_data,
                        p_scan_evt->params.p_not_found->data.len);
                scan.data_len = p_scan_evt->params.p_not_found->data.len;
                nrf_queue_push (&m_scan_queue, &scan);
                m_channel->on_evt (RI_COMM_RECEIVED,
                                   &scan,
                                   sizeof (ri_adv_scan_t));
            }

            break;

        default:
            LOGW ("Unknown event\r\n");
    }
}

NRF_SDH_BLE_OBSERVER (m_scan_observer, APP_BLE_OBSERVER_PRIO, nrf_ble_scan_on_ble_evt,
                      NULL);

rd_status_t ri_adv_tx_interval_set (const uint32_t ms)
{
    rd_status_t err_code = RD_SUCCESS;

    if (MIN_ADV_INTERVAL_MS > ms || MAX_ADV_INTERVAL_MS < ms)
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }
    else
    {
        m_advertisement_interval_ms = ms;
    }

    return err_code;
}

rd_status_t ri_adv_tx_interval_get (uint32_t * ms)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == ms)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        *ms = m_advertisement_interval_ms;
    }

    return err_code;
}

rd_status_t ri_adv_manufacturer_id_set (const uint16_t id)
{
    m_manufacturer_id = id;
    return RD_SUCCESS;
}

rd_status_t ri_adv_channels_set (const ri_radio_channels_t channels)
{
    m_radio_channels = channels;
    return RD_SUCCESS;
}

static rd_status_t format_adv (const ri_comm_message_t * const p_message,
                               advertisement_t * const p_adv)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == p_message) || (NULL == p_adv))
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        // Build specification for data into ble_advdata_t advdata
        ble_advdata_t advdata = {0};
        // Only valid flag
        uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED
                              | BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE;
        // Build manufacturer specific data
        ble_advdata_manuf_data_t manuf_specific_data = {0};
        // Build UUID data
        ble_uuid_t m_adv_uuids[] =
        {
            {m_service_uuid, BLE_UUID_TYPE_BLE}
        };
        // Preserve const of data passed to us.
        uint8_t manufacturer_data[NONEXTENDED_PAYLOAD_MAX_LEN];
        memcpy (manufacturer_data, p_message->data, p_message->data_length);
        manuf_specific_data.data.p_data = manufacturer_data;
        manuf_specific_data.data.size   = p_message->data_length;
        manuf_specific_data.company_identifier = m_manufacturer_id;
        // Point to manufacturer data and flags set earlier
        advdata.flags                 = flags;
        advdata.p_manuf_specific_data = &manuf_specific_data;

        if (m_include_service_uuid)
        {
            advdata.uuids_more_available.p_uuids = m_adv_uuids;
            advdata.uuids_more_available.uuid_cnt = 1;
        }

        // If manufacturer data is not set, assign "UNKNOWN"
        if (0 == m_manufacturer_id)
        {
            manuf_specific_data.company_identifier = 0xFFFF;
        }

        // Encode data
        err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (
                        ble_advdata_encode (&advdata, p_adv->data.adv_data.p_data,
                                            & (p_adv->data.adv_data.len)));
    }

    return err_code;
}

static rd_status_t format_scan_rsp (advertisement_t * const p_adv)
{
    rd_status_t err_code = RD_SUCCESS;
    ble_advdata_t scanrsp = {0};

    if (NULL == p_adv)
    {
        err_code |= RD_ERROR_NULL;
    }
    // Add scan response
    else
    {
        // Name will be read from the GAP data
        scanrsp.name_type = BLE_ADVDATA_FULL_NAME;
        uint8_t len = strlen (m_name);
        err_code |= sd_ble_gap_device_name_set (&m_security, (uint8_t *) m_name, len);

        if (m_advertise_nus)
        {
#           if RUUVI_NRF5_SDK15_GATT_ENABLED
            scanrsp.uuids_complete.uuid_cnt = 1;
            scanrsp.uuids_complete.p_uuids = & (m_adv_uuids[0]);
#           else
            err_code |= RD_ERROR_NOT_SUPPORTED;
#           endif
        }

        // Encode data
        err_code |= ble_advdata_encode (&scanrsp, p_adv->data.scan_rsp_data.p_data,
                                        & (p_adv->data.scan_rsp_data.len));
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

static rd_status_t format_msg (const ri_comm_message_t * const p_message,
                               advertisement_t * const p_adv)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == p_message) || (NULL == p_adv))
    {
        err_code |= RD_ERROR_NULL;
    }
    else if ( (p_message->data_length) > RI_COMM_MESSAGE_MAX_LENGTH)
    {
        err_code |= RD_ERROR_DATA_SIZE;
    }
    else
    {
        err_code |= format_adv (p_message, p_adv);

        if (m_scannable)
        {
            err_code |= format_scan_rsp (p_adv);
        }
    }

    return err_code;
}

/** @brief Determine appropriate PHY and message type.
 *
 */
static rd_status_t set_phy_type (const ri_comm_message_t * const p_message,
                                 advertisement_t * const p_adv)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_radio_modulation_t modulation;
    bool extended_required = false;
    bool sec_phy_required = false;

    if ( (NULL == p_message) || (NULL == p_adv))
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        err_code |= ri_radio_get_modulation (&modulation);

        if (p_message->data_length > NONEXTENDED_PAYLOAD_MAX_LEN)
        {
            sec_phy_required = true;
            extended_required = true;
        }

        switch (modulation)
        {
            case RI_RADIO_BLE_125KBPS:
                p_adv->params.primary_phy = BLE_GAP_PHY_CODED;

                if (sec_phy_required)
                {
                    p_adv->params.secondary_phy = BLE_GAP_PHY_CODED;
                }

                extended_required = true;
                break;

            case RI_RADIO_BLE_1MBPS:
                p_adv->params.primary_phy = BLE_GAP_PHY_1MBPS;

                if (sec_phy_required)
                {
                    p_adv->params.secondary_phy = BLE_GAP_PHY_1MBPS;
                }

                break;

            case RI_RADIO_BLE_2MBPS:
                p_adv->params.primary_phy = BLE_GAP_PHY_1MBPS;

                if (sec_phy_required)
                {
                    p_adv->params.secondary_phy = BLE_GAP_PHY_2MBPS;
                    extended_required = true;
                }

                break;
        }

        switch (m_type)
        {
            case NONCONNECTABLE_NONSCANNABLE:
                if (extended_required)
                {
                    p_adv->params.properties.type =
                        BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
                }
                else
                {
                    p_adv->params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
                }

                break;

            case NONCONNECTABLE_SCANNABLE:
                if (extended_required)
                {
                    p_adv->params.properties.type =
                        BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
                }
                else
                {
                    p_adv->params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
                }

                break;

            case CONNECTABLE_SCANNABLE:
                if (extended_required
                        && sec_phy_required
                        && p_message->data_length > NONEXTENDED_ADV_MAX_LEN)
                {
                    // Cannot put extended payload and scan response into secondary PHY at the same time.
                    err_code |= RD_ERROR_DATA_SIZE;
                    LOGE ("Error: Too long data for a scannable packet.\r\n");
                }
                else
                {
                    p_adv->params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
                }

                break;

            case CONNECTABLE_NONSCANNABLE:
                if (extended_required)
                {
                    p_adv->params.properties.type =
                        BLE_GAP_ADV_TYPE_EXTENDED_CONNECTABLE_NONSCANNABLE_UNDIRECTED;
                }
                else
                {
                    // Cannot have non-scannable connectable event on primary PHY, fallback on extended advertisement.
                    LOGW ("Warning: Fallback to BLE 5.0 done.\r\n");
                    p_adv->params.properties.type =
                        BLE_GAP_ADV_TYPE_EXTENDED_CONNECTABLE_NONSCANNABLE_UNDIRECTED;
                }

                break;

            default:
                LOGW ("Unknown advertisement type\r\n");
                break;
        }
    }

    return err_code;
}

/**
 *  @brief Asynchronous transfer function. Puts/gets message in driver queue
 *
 *  @param[in, out] msg A message to put/get to/from driver queue
 *  @return RD_MORE_AVAILABLE if data was read from queue and there is more data available.
 *  @return RD_SUCCESS if queue operation was successful.
 *  @return RD_ERROR_NULL if message is NULL.
 *  @return RD_ERROR_DATA_SIZE if message length is larger than queue supports.
 *  @return RD_ERROR_NO_MEM if queue is full and new data cannot be queued.
 *  @return RD_ERROR_NOT_FOUND if queue is empty and no more data can be read.
 */
static rd_status_t ri_adv_send (ri_comm_message_t * message)
{
    rd_status_t err_code = RD_SUCCESS;
    ret_code_t nrf_code = NRF_SUCCESS;

    if (NULL == message)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        // Create message
        advertisement_t adv = {0};
        adv.data.adv_data.p_data = adv.adv_data;
        adv.data.adv_data.len = sizeof (adv.adv_data);

        if (m_scannable)
        {
            adv.data.scan_rsp_data.p_data = adv.scan_data;
            adv.data.scan_rsp_data.len = sizeof (adv.scan_data);
        }
        else
        {
            adv.data.scan_rsp_data.p_data = NULL;
            adv.data.scan_rsp_data.len = 0;
        }

        err_code |= format_msg (message, &adv);
        err_code |= set_phy_type (message, &adv);
        adv.params.max_adv_evts = message->repeat_count;
        adv.params.duration = 0; // Do not timeout, use repeat_count.
        adv.params.filter_policy = BLE_GAP_ADV_FP_ANY;
        adv.params.interval = MSEC_TO_UNITS (m_advertisement_interval_ms, UNIT_0_625_MS);
        ruuvi_nrf5_sdk15_radio_channels_set (adv.params.channel_mask, m_radio_channels);
        adv.tx_pwr = m_tx_power;
        nrf_code |= nrf_queue_push (&m_adv_queue, &adv);
        err_code |= prepare_tx();
    }

    return err_code | ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_code);
}

static rd_status_t ri_adv_receive (ri_comm_message_t * message)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_adv_init (ri_comm_channel_t * const channel)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == channel)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (!ri_radio_is_init() || m_advertisement_is_init)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        m_advertisement_is_init = true;
        m_channel = channel;
        m_channel->init    = ri_adv_init;
        m_channel->uninit  = ri_adv_uninit;
        m_channel->send    = ri_adv_send;
        m_channel->read    = ri_adv_receive;
        m_scannable = false;
        // Enable channels by default.
        m_radio_channels.channel_37 = true;
        m_radio_channels.channel_38 = true;
        m_radio_channels.channel_39 = true;
    }

    return err_code;
}

rd_status_t ri_adv_uninit (ri_comm_channel_t * const channel)
{
    rd_status_t err_code = RD_SUCCESS;

    // Stop advertising
    if (true == m_advertising)
    {
        sd_ble_gap_adv_stop (m_adv_handle);
        m_advertising = false;
    }

    m_advertisement_is_init = false;
    // Clear function pointers, including on event
    memset (channel, 0, sizeof (ri_comm_channel_t));
    m_scannable = false;
    m_tx_power = 0;
    // Flush TX buffer.
    nrf_queue_reset (&m_adv_queue);
    return err_code;
}

void ri_adv_rx_ble_phy_enabled_set (const bool is_le_1m_phy_enabled,
                                    const bool is_le_2m_phy_enabled,
                                    const bool is_le_coded_phy_enabled)
{
    m_is_rx_le_1m_phy_enabled = is_le_1m_phy_enabled;
    m_is_rx_le_2m_phy_enabled = is_le_2m_phy_enabled;
    m_is_rx_le_coded_phy_enabled = is_le_coded_phy_enabled;
}

void ri_adv_rx_set_max_advertisement_data_length (const uint8_t max_adv_length)
{
    m_max_adv_length = max_adv_length;
}

rd_status_t ri_adv_scan_start (const uint32_t window_interval_ms,
                               const uint32_t window_size_ms)
{
    ret_code_t status = NRF_SUCCESS;
    rd_status_t err_code = RD_SUCCESS;
    nrf_ble_scan_init_t scan_init_params = {0};
    ble_gap_scan_params_t scan_params = {0};
    uint8_t scan_phys = ruuvi_nrf5_sdk15_radio_phy_get();
    scan_params.active = 0; // Do not scan for scan responses
    ruuvi_nrf5_sdk15_radio_channels_set (scan_params.channel_mask, m_radio_channels);
    scan_params.extended =
        m_is_rx_le_2m_phy_enabled || m_is_rx_le_coded_phy_enabled;
#if defined(RUUVI_NRF5_SDK15_ADV_EXTENDED_ENABLED) && RUUVI_NRF5_SDK15_ADV_EXTENDED_ENABLED
    {
        // 2MBit/s not allowed on primary channel,
        // extended advertisement on secondary channel is automatically
        // scanned with all supported PHYs.
        if (BLE_GAP_PHY_2MBPS == scan_phys)
        {
            scan_phys = BLE_GAP_PHY_1MBPS;
        }
    }
#endif
    NRF_LOG_INFO ("ri_adv_scan_start: NRF modulation: 0x%02x, ext_adv=%d",
                  scan_phys, scan_params.extended);

    if (RD_SUCCESS == err_code)
    {
        scan_params.interval = MSEC_TO_UNITS (window_interval_ms, UNIT_0_625_MS);
        scan_params.report_incomplete_evts = 0;
        scan_params.scan_phys = scan_phys;
        scan_params.window =  MSEC_TO_UNITS (window_size_ms, UNIT_0_625_MS);
        scan_params.timeout = ri_radio_num_channels_get (m_radio_channels) *
                              MSEC_TO_UNITS (window_interval_ms, UNIT_10_MS);
        scan_init_params.p_scan_param = &scan_params;
        status |= nrf_ble_scan_init (&m_scan,           // Scan control structure
                                     &scan_init_params, // Default params for NULL values.
                                     on_advertisement); // Callback on data
        status |= nrf_ble_scan_start (&m_scan);
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (status) | err_code;
}

rd_status_t ri_adv_scan_stop (void)
{
    nrf_ble_scan_stop();
    return RD_SUCCESS;
}

rd_status_t ri_adv_tx_power_set (int8_t * dbm)
{
    ret_code_t err_code = NRF_SUCCESS;

    if (NULL == dbm)
    {
        err_code |= NRF_ERROR_NULL;
    }
    else if (!m_advertisement_is_init)
    {
        err_code |= NRF_ERROR_INVALID_STATE;
    }
    else
    {
        if (*dbm <= -40)      { m_tx_power = -40; }
        else if (*dbm <= -20) { m_tx_power = -20; }
        else if (*dbm <= -16) { m_tx_power = -16; }
        else if (*dbm <= -12) { m_tx_power = -12; }
        else if (*dbm <= -8)  { m_tx_power = -8; }
        else if (*dbm <= -4)  { m_tx_power = -4; }
        else if (*dbm <= 0)   { m_tx_power = 0; }
        else if (*dbm <= 4)   { m_tx_power = 4; }

#ifdef NRF52840_XXAA
        else if (*dbm <= 8)   { m_tx_power = 8; }

#endif
        else
        {
            err_code |= NRF_ERROR_INVALID_PARAM;
        }

        if (RD_SUCCESS == err_code)
        {
            *dbm = m_tx_power;
        }
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

rd_status_t ri_adv_tx_power_get (int8_t * dbm)
{
    *dbm = m_tx_power;
    return RD_SUCCESS;
}

rd_status_t ri_adv_scan_response_setup (const char * const name,
                                        const bool advertise_nus)
{
    ret_code_t err_code = NRF_SUCCESS;

    if (NULL != name)
    {
        strcpy (m_name, name);
    }

    m_scannable = true;
    m_advertise_nus = advertise_nus;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

rd_status_t ri_adv_type_set (ri_adv_type_t type)
{
    rd_status_t err_code = RD_SUCCESS;
    m_type = type;

    if ( (type == NONCONNECTABLE_NONSCANNABLE)
            || (type == CONNECTABLE_NONSCANNABLE))
    {
        m_scannable = false;
    }
    else
    {
        m_scannable = true;
    }

    return err_code;
}

rd_status_t ri_adv_stop()
{
    // SD returns error if advertisement wasn't ongoing, ignore error.
    (void) ruuvi_nrf5_sdk15_to_ruuvi_error (sd_ble_gap_adv_stop (
            m_adv_handle));
    m_advertising = false;
    nrf_queue_reset (&m_adv_queue);
    return RD_SUCCESS;
}

uint16_t ri_adv_parse_manuid (uint8_t * const data,
                              const size_t data_length)
{
    uint8_t * manuf_id;
    manuf_id = ble_advdata_parse (data, data_length,
                                  BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);

    if (manuf_id == NULL)
    {
        return 0;
    }
    else
    {
        return (manuf_id[1] << 8 | manuf_id[0]);
    }
}

void ri_adv_enable_uuid (const bool enable_uuid)
{
    m_include_service_uuid = enable_uuid;
}

void ri_adv_set_service_uuid (const uint16_t uuid)
{
    m_service_uuid = uuid;
}

#endif