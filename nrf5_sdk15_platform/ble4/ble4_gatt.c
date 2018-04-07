/*
 * Interface to Nordic UART Service, implements Ruuvi Communication protocol
 */

#include "sdk_application_config.h"
#if NRF5_SDK15_BLE4_GATT
#include "ble_gatt.h"
#include "communication.h"
#include "ruuvi_error.h"
#include "ringbuffer.h"

#include "app_timer.h"
#include "ble_conn_params.h"
#include "ble_nus.h"
#include "sdk_errors.h"
#include "nrf_error.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_qwr.h"
#include "nrf_ble_gatt.h"

#define PLATFORM_LOG_MODULE_NAME ble4_gatt
#if BLE4_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       BLE4_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  BLE4_INFO_COLOR
#else
#define PLATFORM_LOG_LEVEL       0
#endif
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

/** These structures contain BLE GATT data **/
typedef struct {
  uint8_t  data[BLE_NUS_MAX_DATA_LEN];
  uint8_t  data_len;
  bool     repeat;   // If true, this element will be put back to FIFO after send
} ble_gattdata_storage_t;

NRF_BLE_GATT_DEF(m_gatt);                                                            /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                              /**< Context for the Queued Write module.*/
BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                    /**< BLE NUS service instance. */
static uint16_t       m_conn_handle          = BLE_CONN_HANDLE_INVALID;              /**< Handle of the current connection. */

static uint8_t        incoming[sizeof(ble_gattdata_storage_t) * BLE4_MAXIMUM_GATT_MESSAGES];
static uint8_t        outgoing[sizeof(ble_gattdata_storage_t) * BLE4_MAXIMUM_GATT_MESSAGES];
static ringbuffer_t   incoming_buffer;
static ringbuffer_t   outgoing_buffer;
static bool           gatt_is_init = false;
/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions.
 */
static ret_code_t gap_params_init(void)
{
  ret_code_t              err_code;
  ble_gap_conn_params_t   gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency     = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
  return err_code;
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
  PLATFORM_LOG_ERROR("Conn params fail");
  APP_ERROR_HANDLER(nrf_error);
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
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
  uint32_t err_code;
  PLATFORM_LOG_INFO("Conn params event");
  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
  {
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
}


/**@brief Function for initializing the Connection Parameters module.
 */
static ret_code_t conn_params_init(void)
{
  uint32_t               err_code;
  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params                  = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail             = false;
  cp_init.evt_handler                    = on_conn_params_evt;
  cp_init.error_handler                  = conn_params_error_handler;

  err_code = ble_conn_params_init(&cp_init);
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
static void nus_data_handler(ble_nus_evt_t * p_evt)
{
  if (p_evt->type == BLE_NUS_EVT_RX_DATA)
  {

    PLATFORM_LOG_INFO("Received data from BLE NUS.");
    if (ringbuffer_full(&incoming_buffer))
    {
      PLATFORM_LOG_ERROR("Could not store incoming data");
      return;
    }
    //PLATFORM_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

    ble_gattdata_storage_t msg;
    memcpy(msg.data, p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
    msg.data_len = p_evt->params.rx_data.length;
    ringbuffer_push(&incoming_buffer, &msg);
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
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
  uint32_t err_code;

  switch (p_ble_evt->header.evt_id)
  {
  case BLE_GAP_EVT_CONNECTED:
    PLATFORM_LOG_INFO("Connected");
    m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GAP_EVT_DISCONNECTED:
    PLATFORM_LOG_INFO("Disconnected");

    m_conn_handle = BLE_CONN_HANDLE_INVALID;
    break;

  case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
  {
    PLATFORM_LOG_DEBUG("PHY update request.");
    ble_gap_phys_t const phys =
    {
      .rx_phys = BLE_GAP_PHY_AUTO,
      .tx_phys = BLE_GAP_PHY_AUTO,
    };
    err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
    APP_ERROR_CHECK(err_code);
  } break;

  case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
    // Pairing not supported
    err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GATTS_EVT_SYS_ATTR_MISSING:
    // No system attributes have been stored.
    err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GATTC_EVT_TIMEOUT:
    // Disconnect on GATT Client timeout event.
    err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                     BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break;

  case BLE_GATTS_EVT_TIMEOUT:
    // Disconnect on GATT Server timeout event.
    err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                     BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    APP_ERROR_CHECK(err_code);
    break;

  default:
    // No implementation needed.
    break;
  }
}

/**@brief Function for handling events from the GATT library. */
static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt)
{
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED))
    {
        //m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        PLATFORM_LOG_ERROR("Changing MTU size is not supported by application");
    }
    PLATFORM_LOG_DEBUG("ATT MTU exchange completed. central 0x%x peripheral 0x%x",
                  p_gatt->att_mtu_desired_central,
                  p_gatt->att_mtu_desired_periph);
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    PLATFORM_LOG_ERROR("QWR Error");
    APP_ERROR_HANDLER(nrf_error);
}

ruuvi_status_t ble4_gatt_init(void)
{

  ret_code_t err_code = NRF_SUCCESS;
  nrf_ble_qwr_init_t qwr_init = {0};

  if (!gatt_is_init)
  {
    ringbuffer_init(&incoming_buffer, BLE4_MAXIMUM_GATT_MESSAGES, sizeof(ble_gattdata_storage_t), &incoming);
    ringbuffer_init(&outgoing_buffer, BLE4_MAXIMUM_GATT_MESSAGES, sizeof(ble_gattdata_storage_t), &outgoing);
  }

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

  err_code |= gap_params_init();
  PLATFORM_LOG_INFO("GAP Init status %X", err_code);

  err_code |= nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
  PLATFORM_LOG_INFO("GATT Init status %X", err_code);

  err_code |= nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
  PLATFORM_LOG_INFO("MTU setup status %X", err_code);

  // Initialize Queued Write Module.
  qwr_init.error_handler = nrf_qwr_error_handler;

  err_code |= nrf_ble_qwr_init(&m_qwr, &qwr_init);
  PLATFORM_LOG_INFO("QWR Init status %X", err_code);

  err_code |= conn_params_init();
  PLATFORM_LOG_INFO("Conn params Init status %X", err_code);

  if (NRF_SUCCESS == err_code)
  {
    gatt_is_init = true;
  }

  return platform_to_ruuvi_error(&err_code);
}

ruuvi_status_t ble4_nus_init(void)
{
  uint32_t           err_code;
  ble_nus_init_t     nus_init;

  // Initialize NUS.
  memset(&nus_init, 0, sizeof(nus_init));

  nus_init.data_handler = nus_data_handler;

  err_code = ble_nus_init(&m_nus, &nus_init);
  return platform_to_ruuvi_error(&err_code);
}

ruuvi_status_t ble4_nus_uninit(void)
{
  return RUUVI_ERROR_NOT_SUPPORTED;
}

bool ble4_nus_is_connected(void)
{
  ret_code_t err_code = NRF_SUCCESS;
  ble_nus_client_context_t * p_client;

  err_code = blcm_link_ctx_get(m_nus.p_link_ctx_storage, m_conn_handle, (void *) &p_client);
  if (NRF_SUCCESS != err_code)
  {
    return false;
  }

  if ((m_conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
  {
    return false;
  }

  if (!p_client->is_notification_enabled)
  {
    return false;
  }
  return true;
}

/**
 *  Queue messages from buffer into SD buffer
 */
ruuvi_status_t ble4_nus_process_asynchronous(void)
{
  ret_code_t err_code = NRF_SUCCESS;
  ble_gattdata_storage_t* p_msg;

  //While there remains data, and data was queued successfully
  while (NRF_SUCCESS == err_code && ringbuffer_get_count(&outgoing_buffer))
  {
    // Get pointer to data
    p_msg = ringbuffer_peek_at(&outgoing_buffer, 0);
    // Queue DATA to SD
    uint16_t data_len = p_msg->data_len;
    err_code = ble_nus_data_send(&m_nus, p_msg->data, &data_len, m_conn_handle);
    if (NRF_SUCCESS == err_code)
    {
      // TODO: optimize unnecessary memory copy out?
      ringbuffer_popqueue(&outgoing_buffer, p_msg);
      // Put message back to queue if it is repeated.
      // It's not a bug to allow saturation of tx channel, this can
      // be used to test throughput.
      if (p_msg->repeat)
      {
        ringbuffer_push(&outgoing_buffer, p_msg);
      }
    }
  }
  return RUUVI_SUCCESS;
}

ruuvi_status_t ble4_nus_process_synchronous(void)
{
  ret_code_t err_code = NRF_SUCCESS;
  ble_gattdata_storage_t* p_msg;

  //While there remains data
  while (ringbuffer_get_count(&outgoing_buffer))
  {
    // Get pointer to data
    p_msg = ringbuffer_peek_at(&outgoing_buffer, 0);
    // Queue DATA to SD
    uint16_t data_len = p_msg->data_len;
    err_code = ble_nus_data_send(&m_nus, p_msg->data, &data_len, m_conn_handle);
    if (NRF_SUCCESS == err_code)
    {
      // TODO: optimize unnecessary memory copy out?
      ringbuffer_popqueue(&outgoing_buffer, p_msg);
      // Put message back to queue if it is repeated.
      // Because this is a synchronous function, return
      // after putting message back to avoid getting stuck in eternal loop
      if (p_msg->repeat)
      {
        ringbuffer_push(&outgoing_buffer, p_msg);
        return RUUVI_SUCCESS;
      }
    }
  }
  return RUUVI_SUCCESS;
}

// Drop outgoing buffer
ruuvi_status_t ble4_nus_flush_tx(void)
{
  ble_gattdata_storage_t msg;
  while (ringbuffer_get_count(&outgoing_buffer)) { ringbuffer_popqueue(&outgoing_buffer, &msg); }
  return RUUVI_SUCCESS;
}

// Drop incoming buffer
ruuvi_status_t ble4_nus_flush_rx(void)
{
  ble_gattdata_storage_t msg;
  while (ringbuffer_get_count(&incoming_buffer)) { ringbuffer_popqueue(&incoming_buffer, &msg); }
  return RUUVI_SUCCESS;
}

// Copy message into ringbuffer. Note: this has two memcpy opsm optimize one out?
ruuvi_status_t ble4_nus_message_put(ruuvi_communication_message_t* msg)
{
  if (NULL == msg || NULL == msg->payload)        { return RUUVI_ERROR_NULL; }
  if (msg->payload_length > BLE_NUS_MAX_DATA_LEN) { return RUUVI_ERROR_INVALID_LENGTH; }
  if (ringbuffer_full(&outgoing_buffer))          { return RUUVI_ERROR_NO_MEM; }

  ble_gattdata_storage_t gatt_msg;
  memcpy(gatt_msg.data, msg->payload, msg->payload_length);
  gatt_msg.data_len = msg->payload_length;
  gatt_msg.repeat = msg->repeat;

  ringbuffer_push(&outgoing_buffer, &gatt_msg);

  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t ble4_nus_message_get(ruuvi_communication_message_t* msg)
{
  if (NULL == msg || NULL == msg->payload)        { return RUUVI_ERROR_NULL; }
  if (msg->payload_length > BLE_NUS_MAX_DATA_LEN) { return RUUVI_ERROR_INVALID_LENGTH; }
  if (ringbuffer_empty(&incoming_buffer))         { return RUUVI_ERROR_NOT_FOUND; }

  ble_gattdata_storage_t gatt_msg;
  ringbuffer_popqueue(&incoming_buffer, &gatt_msg);
  memcpy(msg->payload, gatt_msg.data, gatt_msg.data_len);
  msg->payload_length = gatt_msg.data_len;

  return RUUVI_SUCCESS;
}

// Other GATT services
ruuvi_status_t ble4_dis_init(void)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t ble4_dfu_init(void)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

#endif