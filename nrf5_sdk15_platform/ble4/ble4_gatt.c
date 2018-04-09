/*
 * Interface to Nordic UART Service, implements Ruuvi Communication protocol
 */

#include "sdk_application_config.h"
#if NRF5_SDK15_BLE4_GATT
#include "ble_gatt.h"
#include "communication.h"
#include "ruuvi_error.h"
#include "ringbuffer.h"
#include "boards.h" //Device information Service data
#include "timer.h"

#include "app_timer.h"
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

// YOUR_JOB: Update this code if you want to do anything given a DFU event (optional).
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
            PLATFORM_LOG_INFO("Device is preparing to enter bootloader mode.");
            // YOUR_JOB: Disconnect all bonded devices that currently are connected.
            //           This is required to receive a service changed indication
            //           on bootup after a successful (or aborted) Device Firmware Update.
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            PLATFORM_LOG_INFO("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            PLATFORM_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            PLATFORM_LOG_ERROR("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;

        default:
            PLATFORM_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
            break;
    }
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            NRF_LOG_INFO("Connected to a previously bonded device.");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
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
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

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
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
            // This can happen when the local DB has changed.
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}


/**@brief Function for the Peer Manager initialization.
 */
static ret_code_t peer_manager_init()
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code = NRF_SUCCESS;

    err_code |= pm_init();
    if(NRF_SUCCESS != err_code) { PLATFORM_LOG_ERROR("Peer manager init error:%d", err_code); }

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    if(NRF_SUCCESS != err_code) { PLATFORM_LOG_ERROR("Peer manager sec param init error:%d", err_code); }

    err_code = pm_register(pm_evt_handler);
    if(NRF_SUCCESS != err_code) { PLATFORM_LOG_ERROR("Peer manager event handler init error:%d", err_code); }

    return err_code;
}

ruuvi_status_t ble4_gatt_init(void)
{

  ret_code_t err_code = NRF_SUCCESS;
  // Connection param modile requires timers
  if(!platform_timers_is_init()) 
  {
    PLATFORM_LOG_WARNING("Initialize timers before GATT, Initializing now");
    platform_timers_init();
  }
  nrf_ble_qwr_init_t qwr_init = {0};

  if (!gatt_is_init)
  {
    ringbuffer_init(&incoming_buffer, BLE4_MAXIMUM_GATT_MESSAGES, sizeof(ble_gattdata_storage_t), &incoming);
    ringbuffer_init(&outgoing_buffer, BLE4_MAXIMUM_GATT_MESSAGES, sizeof(ble_gattdata_storage_t), &outgoing);
  }

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

  err_code |= peer_manager_init();
  PLATFORM_LOG_INFO("Peer manager init status %X", err_code);

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

  ret_code_t err_code = NRF_SUCCESS;

#define SERIAL_LENGTH      9 //64 bits as hex + trailing null

  // Create pseudo-unique name
  // TODO: Use deviceaddr instead?
  // TODO: Move into drivers / utils
  unsigned int mac0 =  NRF_FICR->DEVICEID[0];
  unsigned int mac1 =  NRF_FICR->DEVICEID[1];
  char serial[SERIAL_LENGTH];
  uint8_t index = 0;
  sprintf(&serial[index], "%x", mac0);
  index += 4;
  sprintf(&serial[index], "%x", mac1);
  index += 4;
  serial[index++] = 0x00;

  ble_dis_init_t dis_init;
  memset(&dis_init, 0, sizeof(dis_init));
  ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, DEVICE_MANUFACTURER);
  ble_srv_ascii_to_utf8(&dis_init.model_num_str, DEVICE_MODEL);
  ble_srv_ascii_to_utf8(&dis_init.serial_num_str, serial);
  ble_srv_ascii_to_utf8(&dis_init.hw_rev_str, DEVICE_HWREV);
  ble_srv_ascii_to_utf8(&dis_init.fw_rev_str, DEVICE_FWREV);
  ble_srv_ascii_to_utf8(&dis_init.sw_rev_str, DEVICE_SWREV);

  // Read security level 1, mode 1. OPEN, i.e. anyone can read without encryption.
  // Write not allowed.
  dis_init.dis_attr_md.read_perm.sm = 1;
  dis_init.dis_attr_md.read_perm.lv = 1;

  err_code = ble_dis_init(&dis_init);
  PLATFORM_LOG_DEBUG("DIS init, status %d\r\n", err_code);
  if (err_code != NRF_SUCCESS)
  {
    PLATFORM_LOG_ERROR("Failed to init DIS\r\n");
  }

  return platform_to_ruuvi_error(&err_code);
}

ruuvi_status_t ble4_dfu_init(void)
{

  ret_code_t err_code = NRF_SUCCESS;

      ble_dfu_buttonless_init_t dfus_init = {0};


    // Initialize the async SVCI interface to bootloader.
    err_code = ble_dfu_buttonless_async_svci_init();
    if(NRF_SUCCESS != err_code) { PLATFORM_LOG_ERROR("BLE DFU SVCI Init error:%d", err_code); }

    dfus_init.evt_handler = ble_dfu_evt_handler;

    err_code = ble_dfu_buttonless_init(&dfus_init);
    if(NRF_SUCCESS != err_code) { PLATFORM_LOG_ERROR("BLE DFU Init error:%d", err_code); }

  return platform_to_ruuvi_error(&err_code);
}

#endif