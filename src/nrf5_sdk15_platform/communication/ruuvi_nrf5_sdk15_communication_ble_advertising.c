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
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_interface_log.h"
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_ble_scan.h"
#include "nrf_nvic.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "ble_types.h"
#include "sdk_errors.h"

#ifndef RUUVI_NRF5_SDK15_ADV_LOG_LEVEL
#define LOG_LEVEL RI_LOG_LEVEL_DEBUG
#else
#define LOG_LEVEL RUUVI_NRF5_SDK15_ADV_LOG_LEVEL
#endif
#define LOG(msg)  (ri_log(LOG_LEVEL, msg))

#define APP_BLE_OBSERVER_PRIO 3
NRF_BLE_SCAN_DEF (m_scan);

typedef struct
{
    uint32_t advertisement_interval_ms;
    int8_t advertisement_power_dbm;
    uint16_t manufacturer_id;
    ri_communication_t * channel;
} ruuvi_platform_ble4_advertisement_state_t;

/** Buffers for scan data. Data has to be double-buffered for live switching of data */
static uint8_t  m_advertisement0[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint16_t m_adv0_len;
static uint8_t  m_advertisement1[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint16_t m_adv1_len;
static uint8_t  m_scan0[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint16_t m_scan0_len;
static uint8_t  m_scan1[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint16_t m_scan1_len;
static int8_t   m_tx_power = 0;

static bool advertisement_odd = false;
static bool m_scannable = false;

static ble_gap_adv_data_t m_adv_data;

static ble_gap_conn_sec_mode_t m_security;

#define DEFAULT_ADV_INTERVAL_MS 1010
#define MIN_ADV_INTERVAL_MS     100
#define MAX_ADV_INTERVAL_MS     10000
/** @brief Parameters to be passed to the stack when starting advertising. */
static ble_gap_adv_params_t m_adv_params;
/** @brief Advertising handle used to identify an advertising set. */
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
/** @brief Flag for initialization **/
static bool                 m_advertisement_is_init = false;
/** @brief Flag for advertising in process **/
static bool                 m_advertising = false;
static ruuvi_platform_ble4_advertisement_state_t m_adv_state;

/**< Universally unique service identifier of Nordic UART Service */
#if RUUVI_NRF5_SDK15_COMMUNICATION_BLE4_GATT_ENABLED
#include "ble_nus.h"
static ble_uuid_t m_adv_uuids[] =
{
    {BLE_UUID_NUS_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}
};
#endif

// Register a handler for advertisement events.
void ble_advertising_on_ble_evt (ble_evt_t const * p_ble_evt, void * p_context)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            // TODO: Stop advertising
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            // TODO: Resume advertising
            break;

        // Upon terminated advertising (time-out), notify application TX complete.
        case BLE_GAP_EVT_ADV_SET_TERMINATED:
            if ( (NULL != m_adv_state.channel)  && (NULL != m_adv_state.channel->on_evt))
            {
                m_adv_state.channel->on_evt (RI_COMMUNICATION_SENT, NULL, 0);
            }

            break;

        default:
            break;
    }
}
NRF_SDH_BLE_OBSERVER (m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_advertising_on_ble_evt,
                      NULL);

// Register a handler for scan events.
static void on_advertisement (scan_evt_t const * p_scan_evt)
{
    switch (p_scan_evt->scan_evt_id)
    {
        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
            LOG ("Scan timeout\r\n");
            m_adv_state.channel->on_evt (RI_COMMUNICATION_TIMEOUT,
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

            if ( (NULL != m_adv_state.channel)  && (NULL != m_adv_state.channel->on_evt))
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
                m_adv_state.channel->on_evt (RI_COMMUNICATION_RECEIVED,
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

// Update BLE settings, takes effect immidiately
static rd_status_t update_settings (void)
{
    if (!m_advertisement_is_init) { return RD_ERROR_INVALID_STATE; }

    ret_code_t err_code = NRF_SUCCESS;

    // Stop advertising for setting update
    if (m_advertising)
    {
        err_code |= sd_ble_gap_adv_stop (m_adv_handle);
    }

    err_code |= sd_ble_gap_adv_set_configure (&m_adv_handle, &m_adv_data, &m_adv_params);

    if (m_advertising)
    {
        err_code = sd_ble_gap_adv_start (m_adv_handle, RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG);
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

// Callback on before / after TX.
static void ri_adv_activity_handler (const ri_radio_activity_evt_t evt)
{
    // No action needed.
}

rd_status_t ri_adv_tx_interval_set (const uint32_t ms)
{
    if (MIN_ADV_INTERVAL_MS > ms || MAX_ADV_INTERVAL_MS < ms) { return RD_ERROR_INVALID_PARAM; }

    m_adv_state.advertisement_interval_ms = ms;
    m_adv_params.interval = MSEC_TO_UNITS (m_adv_state.advertisement_interval_ms,
                                           UNIT_0_625_MS);
    return RD_SUCCESS;
}

rd_status_t ri_adv_tx_interval_get (uint32_t * ms)
{
    *ms = m_adv_state.advertisement_interval_ms;
    return RD_SUCCESS;
}

rd_status_t ri_adv_manufacturer_id_set (const uint16_t id)
{
    m_adv_state.manufacturer_id = id;
    return RD_SUCCESS;
}

// Set manufacturer specific data to advertise. Clears previous data.
rd_status_t ri_adv_data_set (const uint8_t * data, const uint8_t data_length)
{
    if (NULL == data)     { return RD_ERROR_NULL; }

    if (24 < data_length) { return RD_ERROR_INVALID_LENGTH; }

    // Build specification for data into ble_advdata_t advdata
    ble_advdata_t advdata = {0};
    // Only valid flag
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
    // Build manufacturer specific data
    ble_advdata_manuf_data_t manuf_specific_data;
    ret_code_t err_code = NRF_SUCCESS;
    // Preserve const of data passed to us.
    uint8_t manufacturer_data[24];
    memcpy (manufacturer_data, data, data_length);
    manuf_specific_data.data.p_data = manufacturer_data;
    manuf_specific_data.data.size   = data_length;
    manuf_specific_data.company_identifier = m_adv_state.manufacturer_id;
    // Point to manufacturer data and flags set earlier
    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    // If manufacturer data is not set, assign "UNKNOWN"
    if (0 == m_adv_state.manufacturer_id) { manuf_specific_data.company_identifier = 0xFFFF; }

    // Same buffer must not be passed to the SD on data update.
    ble_gap_adv_data_t * p_adv_data = &m_adv_data;
    uint8_t * p_advertisement       = (advertisement_odd) ? m_advertisement0 :
                                      m_advertisement1;
    uint16_t * p_adv_len            = (advertisement_odd) ? &m_adv0_len      : &m_adv1_len;
    uint8_t * p_scan                = (advertisement_odd) ? m_scan0 :
                                      m_scan1;
    uint16_t * p_scan_len           = (advertisement_odd) ? &m_scan0_len     : &m_scan1_len;
    m_adv0_len = sizeof (m_advertisement0);
    m_adv1_len = sizeof (m_advertisement1);
    advertisement_odd = !advertisement_odd;
    // Encode data
    err_code |= ble_advdata_encode (&advdata, p_advertisement, p_adv_len);
    p_adv_data->adv_data.p_data     = p_advertisement;
    p_adv_data->adv_data.len        = *p_adv_len;

    if (m_scannable)
    {
        p_adv_data->scan_rsp_data.p_data = p_scan;
        p_adv_data->scan_rsp_data.len    = *p_scan_len;
    }
    else
    {
        p_adv_data->scan_rsp_data.p_data = NULL;
        p_adv_data->scan_rsp_data.len    = 0;
    }

    if (m_advertising)
    {
        err_code |= sd_ble_gap_adv_set_configure (&m_adv_handle, p_adv_data, NULL);
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

rd_status_t ri_adv_send (ri_communication_message_t * message)
{
    if (NULL == message) { return RD_ERROR_NULL; }

    // Advertising might be stopped by external event, such as GATT connection.
    if (false == m_advertising)
    {
        return RD_ERROR_INVALID_STATE;
    }

    return ri_adv_data_set (message->data, message->data_length);
}

static rd_status_t ri_adv_receive (ri_communication_message_t * message)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

/**
 * @brief Initialize Advertising module and scanning module.
 *
 * @param[out] channel Interface used for communicating through advertisements.
 * @param[in]  adv_channels Physical channels used by radio. One or more.
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_NULL Channel is NULL.
 * @retval RD_ERROR_INVALID_STATE if radio is not already initialized.
 * @return error code from stack if modulation is invalid.
 *
 * @note Modulation used on the advertisement depends on how radio was initialized.
 */
rd_status_t ri_adv_init (ri_communication_t * const channel)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == channel)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (!ri_radio_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        // Initialize advertising parameters (used when starting advertising).
        memset (&m_adv_params, 0, sizeof (m_adv_params));
        m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
        m_adv_params.duration        = 0;       // Never time out.
        m_adv_params.max_adv_evts    = 0;       // Inifinite repeats
        ri_radio_modulation_t modulation;
        err_code |= ri_radio_get_modulation (&modulation);

        if (RD_SUCCESS != err_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            if (RI_RADIO_BLE_125KBPS == modulation)
            {
                m_adv_params.primary_phy     = BLE_GAP_PHY_CODED;
                m_adv_params.properties.type =
                    BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
            }
            // 2 MBit / s starts out as 1 MBit / s and switches to 2 MBit / s on connection.
            else if (RI_RADIO_BLE_2MBPS == modulation)
            {
                m_adv_params.primary_phy     = BLE_GAP_PHY_AUTO;
                m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
            }
            else if (RI_RADIO_BLE_1MBPS == modulation)
            {
                m_adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;
                m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
            }
            else
            {
                err_code |= RD_ERROR_INVALID_PARAM;
            }

            if (RD_SUCCESS == err_code)
            {
                m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
                m_adv_params.interval        = MSEC_TO_UNITS (DEFAULT_ADV_INTERVAL_MS, UNIT_0_625_MS);
                m_advertisement_is_init = true;
                m_adv_state.advertisement_interval_ms = DEFAULT_ADV_INTERVAL_MS;
                m_adv_state.channel = channel;
                channel->init    = ri_adv_init;
                channel->uninit  = ri_adv_uninit;
                channel->send    = ri_adv_send;
                channel->read    = ri_adv_receive;
                //channel->on_evt  = NULL; Do not NULL the channel, let application control it.
                memset (&m_adv_data, 0, sizeof (m_adv_data));
                memset (&m_advertisement0, 0, sizeof (m_advertisement0));
                memset (&m_advertisement1, 0, sizeof (m_advertisement1));
                m_adv0_len = 0;
                m_adv1_len = 0;
                m_scannable = false;
                err_code |= update_settings();
            }
        }
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

/*
 * Uninitializes radio hardware, advertising module and scanning module
 *
 * Returns RUUVI_DIRVER_SUCCESS on success or if radio was not initialized.
 * Returns RD_ERROR_INVALID_STATE if radio hardware was initialized by another radio module.
 */
rd_status_t ri_adv_uninit (ri_communication_t * const channel)
{
    rd_status_t err_code = RD_SUCCESS;

    // Stop advertising
    if (true == m_advertising)
    {
        sd_ble_gap_adv_stop (m_adv_handle);
        m_advertising = false;
    }

    // Clear advertisement parameters
    memset (&m_adv_params, 0, sizeof (m_adv_params));
    // Release radio
    err_code |= ri_radio_uninit ();
    m_advertisement_is_init = false;
    // Clear function pointers, including on event
    memset (channel, 0, sizeof (ri_communication_t));
    memset (&m_adv_state, 0, sizeof (m_adv_state));
    return err_code;
}

rd_status_t ri_adv_rx_interval_set (
    const uint32_t window_interval_ms, const uint32_t window_size_ms)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_adv_rx_interval_get (
    uint32_t * window_interval_ms, uint32_t * window_size_ms)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}
rd_status_t ri_adv_scan_start (void)
{
    ret_code_t status = NRF_SUCCESS;
    status |= nrf_ble_scan_init (&m_scan,          // Scan control structure
                                 NULL,              // Default params
                                 on_advertisement); // Callback on data
    status |= nrf_ble_scan_start (&m_scan);
    return ruuvi_nrf5_sdk15_to_ruuvi_error (status);
}
rd_status_t ri_adv_scan_stop (void)
{
    nrf_ble_scan_stop();
    return RD_SUCCESS;
}
rd_status_t ri_adv_tx_power_set (
    int8_t * dbm)
{
    ret_code_t err_code = NRF_SUCCESS;

    if (*dbm <= -40) { m_tx_power = -40; }
    else if (*dbm <= -20) { m_tx_power = -20; }
    else if (*dbm <= -16) { m_tx_power = -16; }
    else if (*dbm <= -12) { m_tx_power = -12; }
    else if (*dbm <= -8) { m_tx_power = -8; }
    else if (*dbm <= -4) { m_tx_power = -4; }
    else if (*dbm <= 0) { m_tx_power = 0; }
    else if (*dbm <= 4) { m_tx_power = 4; }

#ifdef S140
    else if (*dbm <= 8) { m_tx_power = 8; }

#endif
    else { return RD_ERROR_INVALID_PARAM; }

    err_code = sd_ble_gap_tx_power_set (BLE_GAP_TX_POWER_ROLE_ADV,
                                        m_adv_handle,
                                        m_tx_power
                                       );

    if (RD_SUCCESS == err_code) { *dbm = m_tx_power; }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}
rd_status_t ri_adv_tx_power_get (
    int8_t * dbm)
{
    *dbm = m_tx_power;
    return RD_SUCCESS;
}
rd_status_t ri_adv_scan_response_setup
(const char * const name,
 const bool advertise_nus)
{
    rd_status_t err_code = RD_SUCCESS;
    memset (&m_scan0, 0, sizeof (m_scan0));
    memset (&m_scan1, 0, sizeof (m_scan1));
    ble_advdata_t scanrsp = {0};

    if (NULL != name)
    {
        // Name will be read from the GAP data
        scanrsp.name_type = BLE_ADVDATA_FULL_NAME;
        uint8_t len = strlen (name);
        err_code |= sd_ble_gap_device_name_set (&m_security, (uint8_t *) name, len);
    }

    // Add scan response
    if (advertise_nus)
    {
#if RUUVI_NRF5_SDK15_COMMUNICATION_BLE4_GATT_ENABLED
        scanrsp.uuids_complete.uuid_cnt = 1;
        scanrsp.uuids_complete.p_uuids = & (m_adv_uuids[0]);
#else
        err_code |= RD_ERROR_NOT_SUPPORTED;
#endif
    }

    // Encode data
    m_adv_data.scan_rsp_data.len = sizeof (m_scan0);
    err_code |= ble_advdata_encode (&scanrsp, m_scan0, &m_adv_data.scan_rsp_data.len);
    err_code |= ble_advdata_encode (&scanrsp, m_scan1, &m_adv_data.scan_rsp_data.len);
    // Store encoded length
    m_scan0_len = m_adv_data.scan_rsp_data.len;
    m_scan1_len = m_adv_data.scan_rsp_data.len;

    if (NRF_SUCCESS == err_code) { m_scannable = true; }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}
rd_status_t ri_adv_type_set (ri_adv_type_t type)
{
    rd_status_t err_code = RD_SUCCESS;

    switch (type)
    {
        case NONCONNECTABLE_NONSCANNABLE:
            m_adv_params.properties.type = (BLE_GAP_PHY_AUTO == m_adv_params.primary_phy) ?
                                           BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED :
                                           BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
            break;

        case NONCONNECTABLE_SCANNABLE:
            m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
            break;

        case CONNECTABLE_NONSCANNABLE:
            m_adv_params.properties.type =
                BLE_GAP_ADV_TYPE_EXTENDED_CONNECTABLE_NONSCANNABLE_UNDIRECTED;
            break;

        case CONNECTABLE_SCANNABLE:
            m_adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
            break;

        default:
            err_code = RD_ERROR_INVALID_PARAM;
    }

    return err_code;
}
void ri_adv_notify_stop (void)
{
    m_advertising = false;
}
rd_status_t ri_adv_start()
{
    if (m_advertising) { return RD_ERROR_INVALID_STATE; }

    rd_status_t err_code = RD_SUCCESS;

    if (NULL == m_adv_data.scan_rsp_data.p_data) { m_adv_data.scan_rsp_data.len = 0; }

    err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (sd_ble_gap_adv_set_configure (&m_adv_handle,
                &m_adv_data, &m_adv_params));
    err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (sd_ble_gap_adv_start (m_adv_handle,
                RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG));

    if (RD_SUCCESS == err_code) { m_advertising = true; }

    return err_code;
}
rd_status_t ri_adv_stop()
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (sd_ble_gap_adv_stop (m_adv_handle));
    m_advertising =
        false; // SD returns error if advertisement wasn't ongoing, ignore error and mark as stopped.
    return err_code;
}
rd_status_t ri_adv_ongoing (void)
{
    return m_advertising;
}
rd_status_t ri_adv_send_raw (
    uint8_t * data, size_t data_length)
{
    rd_status_t err_code = RD_SUCCESS;
    ble_gap_adv_data_t * p_adv_data = &m_adv_data;
    uint8_t * p_advertisement       = (advertisement_odd) ? m_advertisement0 :
                                      m_advertisement1;
    advertisement_odd = !advertisement_odd;
    // Copy data
    memcpy (p_advertisement, data, data_length);
    p_adv_data->adv_data.p_data     = p_advertisement;
    p_adv_data->adv_data.len        = data_length;
    p_adv_data->scan_rsp_data.p_data = NULL;
    p_adv_data->scan_rsp_data.len    = 0;
    m_adv_params.max_adv_evts        = 1;
    err_code |= sd_ble_gap_adv_set_configure (&m_adv_handle, p_adv_data, &m_adv_params);
    err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (sd_ble_gap_adv_start (m_adv_handle,
                RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG));
    return err_code;
}
#endif