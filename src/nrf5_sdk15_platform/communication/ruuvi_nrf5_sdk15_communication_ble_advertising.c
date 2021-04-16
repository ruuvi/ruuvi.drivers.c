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

#include <stdint.h>

#ifndef RUUVI_NRF5_SDK15_ADV_LOG_LEVEL
#define LOG_LEVEL RI_LOG_LEVEL_DEBUG
#else
#define LOG_LEVEL RUUVI_NRF5_SDK15_ADV_LOG_LEVEL
#endif
static inline void LOG (const char * const msg)
{
    ri_log (LOG_LEVEL, msg);
}

static inline void LOGW (const char * const msg)
{
    ri_log (RI_LOG_LEVEL_WARNING, msg);
}

static inline void LOGE (const char * const msg)
{
    ri_log (RI_LOG_LEVEL_ERROR, msg);
}

#define DEFAULT_ADV_INTERVAL_MS (1010U)
#define MIN_ADV_INTERVAL_MS     (100U)
#define MAX_ADV_INTERVAL_MS     (10000U)
#define NONEXTENDED_ADV_MAX_LEN (24U)

#define APP_BLE_OBSERVER_PRIO   3U //!< Used in concat macro which fails on braces.
NRF_BLE_SCAN_DEF (m_scan);

typedef struct
{
    uint8_t adv_data[RUUVI_NRF5_SDK15_ADV_LENGTH];  //!< Buffer for advertisement
    uint8_t scan_data[RUUVI_NRF5_SDK15_SCAN_LENGTH]; //!< Buffer for scan response.
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
static char m_name[NONEXTENDED_ADV_MAX_LEN];
/** @brief Advertise UUID of Nordic UART Service in scan response. */
static bool m_advertise_nus;
static ri_adv_type_t m_type;                 //!< Type, configured by user.
static ri_radio_channels_t m_radio_channels; //!< Enabled channels to send

/** @brief Advertising handle used to identify an advertising set. */
static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
/** @brief Flag for initialization **/
static bool m_advertisement_is_init = false;
/** @brief Flag for advertising in process **/
static bool m_advertising = false;

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

NRF_SDH_BLE_OBSERVER (m_ble_observer, APP_BLE_OBSERVER_PRIO,
                      ble_advertising_on_ble_evt_isr, NULL);

// Register a handler for scan events.
static void on_advertisement (scan_evt_t const * p_scan_evt)
{
    switch (p_scan_evt->scan_evt_id)
    {
        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
            LOG ("Scan timeout\r\n");
            m_channel->on_evt (RI_COMM_TIMEOUT,
                               NULL, 0);
            break;

        // Data which matches the configured filter - todo
        case NRF_BLE_SCAN_EVT_FILTER_MATCH:
            LOG ("Matching data\r\n");
            // p_scan_evt->params.filter_match.p_adv_report; // Data should be here
            break;

        // All the data, pass to application
        case NRF_BLE_SCAN_EVT_NOT_FOUND:
            LOG ("Unknown data\r\n");

            if ( (NULL != m_channel)  && (NULL != m_channel->on_evt))
            {
                // Send advertisement report
                ri_adv_scan_t scan;
                scan.addr[0] = p_scan_evt->params.p_not_found->peer_addr.addr[5];
                scan.addr[1] = p_scan_evt->params.p_not_found->peer_addr.addr[4];
                scan.addr[2] = p_scan_evt->params.p_not_found->peer_addr.addr[3];
                scan.addr[3] = p_scan_evt->params.p_not_found->peer_addr.addr[2];
                scan.addr[4] = p_scan_evt->params.p_not_found->peer_addr.addr[1];
                scan.addr[5] = p_scan_evt->params.p_not_found->peer_addr.addr[0];
                scan.rssi    = p_scan_evt->params.p_not_found->rssi;
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
            LOG ("Unknown event\r\n");
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
        ble_advdata_manuf_data_t manuf_specific_data;
        // Preserve const of data passed to us.
        uint8_t manufacturer_data[RI_COMM_MESSAGE_MAX_LENGTH];
        memcpy (manufacturer_data, p_message->data, p_message->data_length);
        manuf_specific_data.data.p_data = manufacturer_data;
        manuf_specific_data.data.size   = p_message->data_length;
        manuf_specific_data.company_identifier = m_manufacturer_id;
        // Point to manufacturer data and flags set earlier
        advdata.flags                 = flags;
        advdata.p_manuf_specific_data = &manuf_specific_data;

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

        if (p_message->data_length > NONEXTENDED_ADV_MAX_LEN)
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

    // Other than 1 MBit / s require extended advertising.
    if (BLE_GAP_PHY_1MBPS == scan_phys)
    {
        scan_params.extended = 0;
    }
    else if (RUUVI_NRF5_SDK15_ADV_EXTENDED_ENABLED)
    {
        scan_params.extended = 1;

        // 2MBit/s not allowed on primary channel,
        // extended advertisement on secondary channel is automatically
        // scanned with all supported PHYs.
        if (BLE_GAP_PHY_2MBPS == scan_phys)
        {
            scan_phys = BLE_GAP_PHY_1MBPS;
        }
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

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

#endif