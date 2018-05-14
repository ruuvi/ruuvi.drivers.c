
#include "sdk_application_config.h"
#if NRF5_SDK15_NFC
#include "nfc_tag.h"
// Ruuvi Headers
#include "ruuvi_error.h"
#include "communication.h"

// NRF5 SDK
#include "nfc_t4t_lib.h"
#include "nfc_ndef_msg.h"
#include "nfc_uri_rec.h"
#include "nfc_text_rec.h"
#include "nfc_launchapp_rec.h"
#include "nfc_ndef_msg_parser.h"

#define PLATFORM_LOG_MODULE_NAME nfc_tag
#if NFC_TAG_INTERFACE_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       NFC_TAG_INTERFACE_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  NFC_TAG_INTERFACE_INFO_COLOR
#else
#define PLATFORM_LOG_LEVEL       0
#endif
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

#ifndef NFC_MAX_NUMBER_OF_RECORDS
#define NFC_MAX_NUMBER_OF_RECORDS 4
#endif

static struct
{
  volatile uint8_t connected;    //NFC field is active
  uint8_t initialized;
  volatile uint8_t rx_updated;   // New data received
  volatile uint8_t tx_updated;   // New data should be written to buffer
  volatile uint8_t configurable; // Allow NFC to write configuration
  uint8_t nfc_ndef_msg[NDEF_FILE_SIZE];
  volatile size_t  nfc_ndef_msg_len;
  ruuvi_communication_fp on_tx_cb;
  ruuvi_communication_fp on_rx_cb;
  ruuvi_communication_fp on_cn_cb;
  ruuvi_communication_fp on_dc_cb;
} nrf5_sdk15_nfc_state;

static ruuvi_status_t nfc_set_on_tx(ruuvi_communication_fp cb)
{
    nrf5_sdk15_nfc_state.on_tx_cb = cb;
    return RUUVI_SUCCESS;
}

static ruuvi_status_t nfc_set_on_rx(ruuvi_communication_fp cb)
{
    nrf5_sdk15_nfc_state.on_rx_cb = cb;
    return RUUVI_SUCCESS;
}

static ruuvi_status_t nfc_set_on_connect(ruuvi_communication_fp cb)
{
    nrf5_sdk15_nfc_state.on_cn_cb = cb;
    return RUUVI_SUCCESS;
}

static ruuvi_status_t nfc_set_on_disconnect(ruuvi_communication_fp cb)
{
    nrf5_sdk15_nfc_state.on_dc_cb = cb;
    return RUUVI_SUCCESS;
}

/**
 * @brief Callback function for handling NFC events.
 */
static void nfc_callback(void * context,
                         nfc_t4t_event_t event,
                         const uint8_t * data,
                         size_t          dataLength,
                         uint32_t        flags)
{
  (void)context;

  switch (event)
  {
  case NFC_T4T_EVENT_FIELD_ON:
    nrf5_sdk15_nfc_state.connected = true;
    if (NULL != nrf5_sdk15_nfc_state.on_cn_cb) { nrf5_sdk15_nfc_state.on_cn_cb(); }
    PLATFORM_LOG_DEBUG("Field detected, do not process data");
    break;

  case NFC_T4T_EVENT_FIELD_OFF:
    nrf5_sdk15_nfc_state.connected = false;
    if (NULL != nrf5_sdk15_nfc_state.on_dc_cb) { nrf5_sdk15_nfc_state.on_dc_cb(); }
    PLATFORM_LOG_DEBUG("Field lost, ok to process data now");
    break;

  case NFC_T4T_EVENT_NDEF_READ:
    PLATFORM_LOG_DEBUG("Data chunk sent");
    if (NULL != nrf5_sdk15_nfc_state.on_tx_cb) { nrf5_sdk15_nfc_state.on_tx_cb(); }
    break;

  // Update process generally sets length of field to 0 and
  // then updates. Once done, length is updated again.
  case NFC_T4T_EVENT_NDEF_UPDATED:
    if (dataLength > 0)
    {
      PLATFORM_LOG_INFO("Got new message with %d bytes", dataLength);
      nrf5_sdk15_nfc_state.nfc_ndef_msg_len = dataLength;
      nrf5_sdk15_nfc_state.rx_updated = true;
      // If tag is not configurable by NFC, set flag to overwrite received data.
      if (!nrf5_sdk15_nfc_state.configurable) { nrf5_sdk15_nfc_state.tx_updated = true;}
      // Do not process data in interrupt context, you should rather schedule data processing
      if (NULL != nrf5_sdk15_nfc_state.on_rx_cb) { nrf5_sdk15_nfc_state.on_rx_cb(); }
    }
    break;

  default:
    break;
  }
}

NFC_NDEF_MSG_DEF(nfc_ndef_msg, NFC_MAX_NUMBER_OF_RECORDS); // max NFC_MAX_NUMBER_OF_RECORDS records

// Setup constant records XXX move these to state?
static uint8_t nfc_text_buf[NFC_TEXT_BUF_SIZE];
static size_t nfc_text_length = 0;
static uint8_t nfc_uri_buf[NFC_URI_BUF_SIZE];
static size_t nfc_uri_length = 0;
static uint8_t nfc_app_buf[NFC_APP_BUF_SIZE];
static size_t nfc_app_length = 0;
static uint8_t nfc_tx_buf[NFC_DATA_BUF_SIZE];
static size_t nfc_tx_length = 0;
static uint8_t nfc_rx_buf[NFC_DATA_BUF_SIZE];
static size_t nfc_rx_length = 0;
static uint8_t desc_buf[NFC_NDEF_PARSER_REQIRED_MEMO_SIZE_CALC(NFC_MAX_NUMBER_OF_RECORDS)];
static size_t msg_index = 0; // Store index of our record



//Setup Ruuvi Message buffer

ruuvi_status_t nfc_text_record_set(const uint8_t* text, size_t length)
{
  if (NULL == text) { return RUUVI_ERROR_NULL; }
  if (length >= NFC_TEXT_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  nfc_text_length = length;
  memcpy(nfc_text_buf, text, nfc_text_length);
  nrf5_sdk15_nfc_state.tx_updated = true;
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_uri_record_set(const uint8_t* uri, size_t length)
{
  if (NULL == uri) { return RUUVI_ERROR_NULL; }
  if (length >= NFC_URI_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  nfc_uri_length = length;
  memcpy(nfc_uri_buf, uri, nfc_uri_length);
  nrf5_sdk15_nfc_state.tx_updated = true;
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_app_record_set(const uint8_t* app, size_t length)
{
  if (NULL == app) { return RUUVI_ERROR_NULL; }
  if (length >= NFC_APP_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  nfc_app_length = length;
  memcpy(nfc_app_buf, app, nfc_app_length);
  nrf5_sdk15_nfc_state.tx_updated = true;
  return RUUVI_SUCCESS;
}

/**
 *  Attempt to move data into NFC tx buffer.
 *  First reads incoming data from the NFC buffer into RX buffer
 *  Then updates TX buffer if applicable.
 *  Return error if new data has been received but not handled yet.
 *  Return error if NFC is not initialized or if NFC is currently connected
 *  Return error if buffers cannot accommodate data.
 *  Return success if buffers were successfully handled.
 */
static ruuvi_status_t nfc_process_asynchronous()
{
  // State check
  PLATFORM_LOG_DEBUG("Asynch process requested, checking state");
  if (!nrf5_sdk15_nfc_state.initialized) { return RUUVI_ERROR_INVALID_STATE; }
  if ( nrf5_sdk15_nfc_state.connected)   { return RUUVI_ERROR_INVALID_STATE; }

  // Return success if there is nothing to do
  PLATFORM_LOG_DEBUG("State ok, checking if there is something to be processed");
  if (!(nrf5_sdk15_nfc_state.rx_updated || nrf5_sdk15_nfc_state.tx_updated)) { return RUUVI_SUCCESS; }
  PLATFORM_LOG_DEBUG("Start processing");
  ruuvi_status_t err_code = RUUVI_SUCCESS;

  //Copy RX data to buffer before NFC buffer is overwritten with new data
  if (nrf5_sdk15_nfc_state.rx_updated)
  {
    PLATFORM_LOG_DEBUG("Moving received data into dedicated buffer");
    // Return error if RX buffer is not parsed yet
    if (nfc_rx_length) { return RUUVI_ERROR_RESOURCES; }
    if (nrf5_sdk15_nfc_state.nfc_ndef_msg_len > sizeof(nfc_rx_buf)) { return RUUVI_ERROR_DATA_SIZE; }
    PLATFORM_LOG_DEBUG("Move ok");
    // Skip T4T length bytes
    memcpy(nfc_rx_buf, &(nrf5_sdk15_nfc_state.nfc_ndef_msg[2]), nrf5_sdk15_nfc_state.nfc_ndef_msg_len);
  }

  //Update TX data only if program has written something, i.e. allow tag to be writeable by client.
  if (nrf5_sdk15_nfc_state.tx_updated)
  {
    /* Create NFC NDEF text record description in English */
    uint8_t lang_code[] = {'d', 't'}; //DATA
    NFC_NDEF_TEXT_RECORD_DESC_DEF(nfc_text_rec,
                                  UTF_8,
                                  lang_code,
                                  sizeof(lang_code),
                                  nfc_text_buf,
                                  nfc_text_length);

    NFC_NDEF_URI_RECORD_DESC_DEF(nfc_uri_rec,
                                 NFC_URI_HTTPS,
                                 nfc_uri_buf,
                                 nfc_uri_length);

    //Sorry Windows Phone users
    NFC_NDEF_ANDROID_LAUNCHAPP_RECORD_DESC_DEF(nfc_app_rec,
        nfc_app_buf,
        nfc_app_length);

    NFC_NDEF_RECORD_BIN_DATA_DEF(nfc_bin_rec,                                             \
                                 TNF_MEDIA_TYPE,                                    \
                                 NULL, 0,                                           \
                                 NULL,                                              \
                                 0,                                                 \
                                 nfc_tx_buf,                                        \
                                 nfc_tx_length);


    //Clear our record
    nfc_ndef_msg_clear(&NFC_NDEF_MSG(nfc_ndef_msg));
    // Add new records if applicable
    if (nfc_text_length)
    {
      err_code |= nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_ndef_msg),
                                          &NFC_NDEF_TEXT_RECORD_DESC(nfc_text_rec));
    }
    if (nfc_uri_length)
    {
      err_code |= nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_ndef_msg),
                                          &NFC_NDEF_URI_RECORD_DESC(nfc_uri_rec));
    }

    if (nfc_app_length) {
      err_code |= nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_ndef_msg),
                                          &NFC_NDEF_ANDROID_LAUNCHAPP_RECORD_DESC(nfc_app_rec));
    }
    if (nfc_tx_length) {
      err_code |= nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_ndef_msg),
                                          &NFC_NDEF_RECORD_BIN_DATA(nfc_bin_rec));
    }

    // Encode data to NFC buffer. NFC will transmit the buffer, i.e. data is updated immediately.
    uint32_t msg_len = sizeof(nrf5_sdk15_nfc_state.nfc_ndef_msg);
    err_code |= nfc_ndef_msg_encode(&NFC_NDEF_MSG(nfc_ndef_msg),
                                    nrf5_sdk15_nfc_state.nfc_ndef_msg,
                                    &msg_len);

    // TX Data processed, set update status to false
    // RX data must wait unitl application has parsed data out of RX buffer
    nrf5_sdk15_nfc_state.tx_updated = false;
  }

  return err_code;
}

// Stop NFC emulation
static ruuvi_status_t nfc_uninit(ruuvi_communication_channel_t* nfc_comms)
{
  if (!nrf5_sdk15_nfc_state.initialized) { return RUUVI_ERROR_INVALID_STATE; }

  ruuvi_status_t err_code = nfc_t4t_emulation_stop();
  err_code |= nfc_t4t_done();
  if (RUUVI_SUCCESS == err_code) { nrf5_sdk15_nfc_state.initialized = false; }

  return platform_to_ruuvi_error(&err_code);
}

// NFC is always polled by master, we cannot "push" data.
static ruuvi_status_t nfc_process_synchronous(void)
{
  return RUUVI_ERROR_NOT_SUPPORTED;
}

/* Read and parse RX buffer into records. Parse records into Ruuvi Communication messages.
 * RX buffer is flushed on error.
 *
 * param msg Ruuvi Communication message, received record payload is copied into message payload field.
 *
 * Return RUUVI_SUCCESS if payload was parsed into msg
 * Return RUUVI_ERROR_DATA_SIZE if received message could not fit into message paylosd
 */
static ruuvi_status_t nfc_message_get(ruuvi_communication_message_t* msg)
{
  //Input check
  PLATFORM_LOG_DEBUG("Getting message, state check");
  if (NULL == msg) { return RUUVI_ERROR_NULL; }
  // State check. Do not process data while connection is active, i.e. while
  // data might be received.
  if ( nrf5_sdk15_nfc_state.connected)   { return RUUVI_ERROR_INVALID_STATE; }
  // If new data is not received, return not found
  PLATFORM_LOG_DEBUG("Getting message, checking if there is new data");
  if (!nrf5_sdk15_nfc_state.rx_updated) { return RUUVI_ERROR_NOT_FOUND; }
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  PLATFORM_LOG_INFO("Getting message %d", msg_index);

  // If we're at index 0, parse message into records
  if (!msg_index)
  {
    uint32_t desc_buf_len = sizeof(desc_buf);
    uint32_t data_lenu32 = sizeof(nfc_rx_buf); //Skip NFCT4T length bytes?
    err_code = ndef_msg_parser(desc_buf,
                               &desc_buf_len,
                               nfc_rx_buf, //Skip NFCT4T length bytes
                               &data_lenu32);
    // Debug will only be printed out if NRF_LOG_INFO is enabled
    // for NFC_NDEF_MSG_PARSER_LOG
    // and NFC_NDEF_RECORD_PARSER_LOG
    // Also, printing out data leads to occasional NFC driver crash, race condition?
    //PLATFORM_LOG_HEXDUMP_INFO(nfc_rx_buf, data_lenu32);
    PLATFORM_LOG_INFO("Found %d messages", ((nfc_ndef_msg_desc_t*)desc_buf)->record_count);
    //ndef_msg_printout((nfc_ndef_msg_desc_t*) desc_buf);
  }

  //As long as there is a new message, parse the payload into Ruuvi Message.
  if (msg_index < ((nfc_ndef_msg_desc_t*)desc_buf)->record_count)
  {
    PLATFORM_LOG_INFO("Parsing message %d", msg_index);
    nfc_ndef_record_desc_t* const p_rec_desc = ((nfc_ndef_msg_desc_t*)desc_buf)->pp_record[msg_index];
    nfc_ndef_bin_payload_desc_t* p_bin_pay_desc = p_rec_desc->p_payload_descriptor;
    // Data length check
    if (p_bin_pay_desc->payload_length > msg->payload_length) { err_code = RUUVI_ERROR_DATA_SIZE; }
    else {
      memcpy(msg->payload, (uint8_t*)p_bin_pay_desc->p_payload, p_bin_pay_desc->payload_length);
      msg->payload_length = p_bin_pay_desc->payload_length;
    }
    msg_index++;
  }
  else { err_code = RUUVI_ERROR_NOT_FOUND; }

  // If no more records could/can be parsed, reset buffer and message counter
  if (RUUVI_SUCCESS != err_code || msg_index == ((nfc_ndef_msg_desc_t*)desc_buf)->record_count)
  {
    memset(nfc_rx_buf, 0, sizeof(nfc_rx_buf));
    msg_index = 0;
    nrf5_sdk15_nfc_state.rx_updated = false;
    PLATFORM_LOG_INFO("All messages are parsed");
  }

  return err_code;
}

static ruuvi_status_t nfc_is_connected(void)
{
  if(nrf5_sdk15_nfc_state.connected) { return RUUVI_SUCCESS; }
  return RUUVI_ERROR_INVALID_STATE;
}


/**
 *  Put a message to TX buffer.
 *  Overwrites previously written data message.
 *  Call process_asynchronous() to encode message into NFC.
 *  Return error if message is null or too long
 *  Return success if payload was copied to buffer.
 */
static ruuvi_status_t nfc_message_put(ruuvi_communication_message_t* msg)
{
  if (NULL == msg) { return RUUVI_ERROR_NULL; }
  if (msg->payload_length > nfc_tx_length) { return RUUVI_ERROR_DATA_SIZE; }
  // If message should be repeated, disable NFC message configuration by NFC writes. And vice versa
  nrf5_sdk15_nfc_state.configurable = !(msg->repeat);
  nrf5_sdk15_nfc_state.tx_updated = true;
  memcpy(nfc_tx_buf, msg->payload, msg->payload_length);
  return RUUVI_SUCCESS;
}

//Clear TX queue
static ruuvi_status_t nfc_flush_tx(void)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

//Clear RX queue
static ruuvi_status_t nfc_flush_rx(void)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t nfc_init(ruuvi_communication_channel_t* nfc_comms)
{
  if (nrf5_sdk15_nfc_state.initialized) { return RUUVI_ERROR_INVALID_STATE; }
  if (NULL == nfc_comms)                { return RUUVI_ERROR_NULL; }
  /* Set up NFC */
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  err_code |= nfc_t4t_setup(nfc_callback, NULL);
  //APP_ERROR_CHECK(err_code);

  memset(nrf5_sdk15_nfc_state.nfc_ndef_msg, 0, sizeof(nrf5_sdk15_nfc_state.nfc_ndef_msg));

  /* Run Read-Write mode for Type 4 Tag platform */
  err_code |= nfc_t4t_ndef_rwpayload_set(nrf5_sdk15_nfc_state.nfc_ndef_msg, sizeof(nrf5_sdk15_nfc_state.nfc_ndef_msg));
  //APP_ERROR_CHECK(err_code);

  /* Start sensing NFC field */
  err_code |= nfc_t4t_emulation_start();

  nrf5_sdk15_nfc_state.initialized = true;
  // Allow configuration by default
  nrf5_sdk15_nfc_state.configurable = true;

  // Setup communication abstraction fps
  nfc_comms->init                 = nfc_init;
  nfc_comms->uninit               = nfc_uninit;
  nfc_comms->is_connected         = nfc_is_connected;
  nfc_comms->process_asynchronous = nfc_process_asynchronous;
  nfc_comms->process_synchronous  = nfc_process_synchronous;
  nfc_comms->flush_tx             = nfc_flush_tx;
  nfc_comms->flush_rx             = nfc_flush_rx;
  nfc_comms->message_put          = nfc_message_put;
  nfc_comms->message_get          = nfc_message_get;
  nfc_comms->set_on_connect       = nfc_set_on_connect;
  nfc_comms->set_on_disconnect    = nfc_set_on_disconnect;
  nfc_comms->set_on_rx            = nfc_set_on_rx;
  nfc_comms->set_on_tx            = nfc_set_on_tx;

  return platform_to_ruuvi_error(&err_code);
}

#endif