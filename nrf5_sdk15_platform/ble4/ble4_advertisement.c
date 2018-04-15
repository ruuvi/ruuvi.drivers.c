#include "sdk_application_config.h"
#if NRF5_SDK15_BLE4_STACK

#include "ble4_advertisement.h"
#include "ruuvi_error.h"
#include "ringbuffer.h"

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "ble_nus.h"
#include "ble_types.h"
#include "sdk_errors.h"
#include "app_util.h"

#define PLATFORM_LOG_MODULE_NAME ble4_adv
#if BLE4_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       BLE4_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  BLE4_INFO_COLOR
#else // ANT_BPWR_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       0
#endif // ANT_BPWR_LOG_ENABLED
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

/** These structures contain encoded advertisement data **/
typedef struct {
    uint8_t advertisement[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
    uint16_t adv_len;
    uint8_t response[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
    uint16_t rsp_len;
    bool repeat;   // If true, this element will be put back to FIFO after send
} ble_advdata_storage_t;

typedef struct {
    int16_t advertisement_interval_ms;
    int8_t advertisement_power_dbm;
    ble4_advertisement_type_t advertisement_type;
    uint16_t manufacturer_id;
} ble4_advertisement_state_t;

static ble_uuid_t m_adv_uuids[] =                                          /**< Universally unique service identifier. */
{
    {BLE_UUID_NUS_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN}
};

#define MAXIMUM_ADVERTISEMENTS 5
static ble_gap_adv_params_t m_adv_params;                                  /**< Parameters to be passed to the stack when starting advertising. */
static uint8_t              m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an advertising set. */
static bool                 advertisement_is_init = false;                 /**< Flag for initialization **/
static bool                 advertising = false;                           /**< Flag for advertising in process **/
static bool                 scan_response_uuid = false;                    /**< Advertise NUS in scan response **/

static ble4_advertisement_state_t m_adv_state;
static uint8_t      advertisements[sizeof(ble_advdata_storage_t) * MAXIMUM_ADVERTISEMENTS];
static ringbuffer_t               advertisement_buffer;

static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = NULL,
        .len    = 0
    },
    .scan_rsp_data =
    {
        .p_data = NULL,
        .len    = 0

    }
};

// Update BLE settings, takes effect immidiately
static ruuvi_status_t update_settings(void)
{
    if (!advertisement_is_init) { return RUUVI_ERROR_INVALID_STATE; }
    ret_code_t err_code = NRF_SUCCESS;
    // Stop advertising for setting update
    if (advertising)
    {
        err_code |= sd_ble_gap_adv_stop(m_adv_handle);
    }
    err_code |= sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
    if (advertising)
    {
        err_code = sd_ble_gap_adv_start(m_adv_handle, BLE_CONN_CFG_TAG_DEFAULT);
    }
    return platform_to_ruuvi_error(&err_code);
}
// Functions for setting up advertisement constants
// Each change takes effect immediately
ruuvi_status_t ble4_advertisement_set_interval(int16_t ms)
{
    if (100 > ms || 10001 < ms) { return RUUVI_ERROR_INVALID_PARAM; }
    m_adv_state.advertisement_interval_ms = ms;
    m_adv_params.interval = MSEC_TO_UNITS(m_adv_state.advertisement_interval_ms, UNIT_0_625_MS);
    return update_settings();
}

// TODO: HW-specific TX powers
// Supported tx_power values: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm and +4dBm.
ruuvi_status_t ble4_advertisement_set_power(int8_t dbm)
{
    int8_t  tx_power = 0;
    ret_code_t err_code = NRF_SUCCESS;
    if (dbm <= -40) { tx_power = -40; }
    else if (dbm <= -20) { tx_power = -20; }
    else if (dbm <= -16) { tx_power = -16; }
    else if (dbm <= -12) { tx_power = -12; }
    else if (dbm <= -8 ) { tx_power = -8; }
    else if (dbm <= -4 ) { tx_power = -4; }
    else if (dbm <= 0  ) { tx_power = 0; }
    else if (dbm <= 4  ) { tx_power = 4; }
    else { return RUUVI_ERROR_INVALID_PARAM; }
    err_code = sd_ble_gap_tx_power_set (BLE_GAP_TX_POWER_ROLE_ADV,
                                        m_adv_handle,
                                        tx_power
                                       );
    return platform_to_ruuvi_error(&err_code);
}

ruuvi_status_t ble4_advertisement_set_type(ble4_advertisement_type_t advertisement_type)
{
    ret_code_t err_code = NRF_SUCCESS;
    switch (advertisement_type)
    {
    case NON_CONNECTABLE_NON_SCANNABLE:
        m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
        m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
        break;

    case NON_CONNECTABLE_SCANNABLE:
        m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
        m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
        break;

    case CONNECTABLE_SCANNABLE:
        m_adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
        m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
        break;

    default:
        err_code |= RUUVI_ERROR_INVALID_PARAM;
        break;
    }
    if (NRF_SUCCESS == err_code)
    {
        m_adv_state.advertisement_type = advertisement_type;
        return update_settings();
    }
    return platform_to_ruuvi_error(&err_code);;
}
ruuvi_status_t ble4_advertisement_set_manufacturer_id(uint16_t id)
{
    m_adv_state.manufacturer_id = id;
    return RUUVI_SUCCESS;
}

ruuvi_status_t ble4_advertisement_scan_response_nus_advertise(bool advertise)
{
    scan_response_uuid = advertise;
    return RUUVI_SUCCESS;
}

/*
 * Initialize advertisement buffer and default values to params.
 */
ruuvi_status_t ble4_advertisement_init(void)
{
    ruuvi_status_t err_code = RUUVI_SUCCESS;
    if (!advertisement_is_init)
    {
        ringbuffer_init(&advertisement_buffer, MAXIMUM_ADVERTISEMENTS, sizeof(ble_advdata_storage_t), &advertisements);
    }
    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.duration        = 0;       // Never time out.
    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr     = NULL;    // Undirected advertisement.
    m_adv_params.interval        = APP_ADV_INTERVAL;

    advertisement_is_init = true;

    return err_code;
}

/*
 * stop advertising.
 */
ruuvi_status_t ble4_advertisement_uninit(void)
{
    return RUUVI_ERROR_NOT_IMPLEMENTED;
}

/*
 * Starts advertising, TX queue is processed as packets leave.
 * Returns success if advertising was started.
 * If no more data remains in queue last data is repeated.
 */
ruuvi_status_t ble4_advertisement_process_asynchronous(void)
{
    // Second parameter is a handle identifying advertising set, unused in non-connectable non-scannable advertisements.
    ret_code_t err_code = NRF_SUCCESS;
    if (!advertising)
    {
        err_code = sd_ble_gap_adv_start(m_adv_handle, BLE_CONN_CFG_TAG_DEFAULT);
        if (NRF_SUCCESS == err_code) { advertising = true; }
    }
    return platform_to_ruuvi_error(&err_code);
}

void ble4_advertisement_restart(void)
{
    sd_ble_gap_adv_start(m_adv_handle, BLE_CONN_CFG_TAG_DEFAULT);
}

// we have to wait for TX windows
ruuvi_status_t ble4_advertisement_process_synchronous(void)
{
    return RUUVI_ERROR_NOT_SUPPORTED;
}

/*
 * Drop TX buffer. Last data will repeat advertising.
 */
ruuvi_status_t ble4_advertisement_flush_tx(void)
{
    ble_advdata_storage_t data;
    while (!ringbuffer_empty(&advertisement_buffer))
    {
        ringbuffer_popqueue(&advertisement_buffer, &data);
    }
    return RUUVI_SUCCESS;
}

/*
 * Put data to TX buffer XXX Handle data being emptied from buffer
 */
ruuvi_status_t ble4_advertisement_message_put(ruuvi_communication_message_t* msg)
{
    if (NULL == msg) { return RUUVI_ERROR_NULL; }
    if (!advertisement_is_init) { return RUUVI_ERROR_INVALID_STATE; }
    if (BLE_GAP_ADV_SET_DATA_SIZE_MAX < msg->payload_length) { return RUUVI_ERROR_INVALID_LENGTH; }
    if (ringbuffer_full(&advertisement_buffer)) { return RUUVI_ERROR_NO_MEM; }
    PLATFORM_LOG_HEXDUMP_DEBUG(msg->payload, msg->payload_length);

    ble_advdata_storage_t data = {0};
    data.adv_len = sizeof(data.advertisement);
    data.rsp_len = sizeof(data.response);
    ble_advdata_t advdata = {0};
    ble_advdata_t rspdata = {0};
    uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
    ble_advdata_manuf_data_t manuf_specific_data;
    ret_code_t err_code = NRF_SUCCESS;
    //Payload will be encoded and copied to separate buffer
    manuf_specific_data.data.p_data = msg->payload;
    manuf_specific_data.data.size   = msg->payload_length;
    manuf_specific_data.company_identifier = m_adv_state.manufacturer_id;

    advdata.flags                 = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;
    // Encode data
    err_code |= ble_advdata_encode(&advdata, data.advertisement, &data.adv_len);
    PLATFORM_LOG_INFO("ADV data status: 0x%X", err_code);

    // Scan response
    rspdata.name_type = BLE_ADVDATA_FULL_NAME;
    if (scan_response_uuid)
    {
        rspdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
        rspdata.uuids_complete.p_uuids  = m_adv_uuids;
    }
    err_code |= ble_advdata_encode(&rspdata, data.response, &data.rsp_len);
    PLATFORM_LOG_INFO("RSP data status: 0x%X", err_code);
    // Store data
    ringbuffer_push(&advertisement_buffer, &data);

    //Setup pointers to data
    size_t index = ringbuffer_get_count(&advertisement_buffer) - 1;
    PLATFORM_LOG_INFO("Advertising at slot %d", index);
    ble_advdata_storage_t* p_data = ringbuffer_peek_at(&advertisement_buffer, index);
    m_adv_data.adv_data.p_data      = p_data->advertisement;
    m_adv_data.adv_data.len         = p_data->adv_len;
    m_adv_data.scan_rsp_data.p_data = p_data->response;
    m_adv_data.scan_rsp_data.len    = p_data->rsp_len;
    PLATFORM_LOG_DEBUG("Set up advertisement data with length of %d and response %d", m_adv_data.adv_data.len, m_adv_data.scan_rsp_data.len);
    PLATFORM_LOG_HEXDUMP_DEBUG(m_adv_data.adv_data.p_data, m_adv_data.adv_data.len);
    PLATFORM_LOG_HEXDUMP_DEBUG(m_adv_data.scan_rsp_data.p_data, m_adv_data.scan_rsp_data.len);

    // Configure advertisement, do not allow parameter update if already advertising.
    ble_gap_adv_params_t* p_adv_params = &m_adv_params;
    if (advertising) { p_adv_params = NULL; }
    err_code |= sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, p_adv_params);
    PLATFORM_LOG_INFO("Configure status: 0x%X", err_code);

    return platform_to_ruuvi_error(&err_code);
}

#endif