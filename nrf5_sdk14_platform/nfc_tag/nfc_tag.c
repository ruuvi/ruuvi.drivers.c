
#include "sdk_application_config.h"
#ifdef NRF5_SDK14_NFC
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
#else // ANT_BPWR_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       0
#endif // ANT_BPWR_LOG_ENABLED
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

#ifndef NFC_MAX_NUMBER_OF_RECORDS
#define NFC_MAX_NUMBER_OF_RECORDS 4
#endif

static struct
{
  uint8_t connected;
  uint8_t initialized;
  uint8_t rx_updated;
  uint8_t tx_updated;
  uint8_t nfc_ndef_msg[NDEF_FILE_SIZE];
  size_t  nfc_ndef_msg_len;
} nrf5_sdk14_nfc_state;

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
    nrf5_sdk14_nfc_state.connected = true;
    break;

  case NFC_T4T_EVENT_FIELD_OFF:
    nrf5_sdk14_nfc_state.connected = false;
    break;

  case NFC_T4T_EVENT_NDEF_READ:
    //bsp_board_led_on(BSP_BOARD_LED_3);
    break;

  // Update process generally sets length of field to 0 and
  // then updates. Once done, length is updated again.
  case NFC_T4T_EVENT_NDEF_UPDATED:
    if (dataLength > 0)
    {
      nrf5_sdk14_nfc_state.nfc_ndef_msg_len = dataLength;
      nrf5_sdk14_nfc_state.rx_updated = true;
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
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_uri_record_set(const uint8_t* uri, size_t length)
{
  if (NULL == uri) { return RUUVI_ERROR_NULL; }
  if (length >= NFC_URI_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  nfc_uri_length = length;
  memcpy(nfc_uri_buf, uri, nfc_uri_length);
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_app_record_set(const uint8_t* app, size_t length)
{
  if (NULL == app) { return RUUVI_ERROR_NULL; }
  if (length >= NFC_APP_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  nfc_app_length = length;
  memcpy(nfc_app_buf, app, nfc_app_length);
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_message_put(ruuvi_communication_message_t* message)
{
  if (NULL == message) { return RUUVI_ERROR_NULL; }
  if (message->payload_length >= NFC_DATA_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  if (nfc_tx_length) { return RUUVI_ERROR_RESOURCES; } // No more space on FIFO
  nfc_tx_length = message->payload_length;
  memcpy(nfc_tx_buf, message->payload, nfc_tx_length);
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_process_asynchronous()
{
  // State check
  if (!nrf5_sdk14_nfc_state.initialized) { return RUUVI_ERROR_INVALID_STATE; }
  if ( nrf5_sdk14_nfc_state.connected)   { return RUUVI_ERROR_INVALID_STATE; }

  // Return success if there is nothing to do
  if (!(nrf5_sdk14_nfc_state.rx_updated || nrf5_sdk14_nfc_state.tx_updated)) { return RUUVI_SUCCESS; }

  //Copy RX data to buffer before NFC buffer is overwritten with new data
  if (nrf5_sdk14_nfc_state.rx_updated)
  {
    // Return error if RX buffer is not parsed yet
    if (nfc_rx_length) { return RUUVI_ERROR_RESOURCES; }
    memcpy(nfc_rx_buf, nrf5_sdk14_nfc_state.nfc_ndef_msg, nrf5_sdk14_nfc_state.nfc_ndef_msg_len);
  }

  //Update TX data only if program has written something, i.e. allow tag to be writeable.

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

  ruuvi_status_t err_code = RUUVI_SUCCESS;

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

  // Encode data to NFC buffer. NFC will transmit the buffer, i.e. data is updated immediately.
  uint32_t msg_len = sizeof(nrf5_sdk14_nfc_state.nfc_ndef_msg);
  err_code |= nfc_ndef_msg_encode(&NFC_NDEF_MSG(nfc_ndef_msg),
                                  nrf5_sdk14_nfc_state.nfc_ndef_msg,
                                  &msg_len);

  return err_code;
}

ruuvi_status_t nfc_init(void)
{
  if (nrf5_sdk14_nfc_state.initialized) { return RUUVI_ERROR_INVALID_STATE; }
  /* Set up NFC */
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  err_code |= nfc_t4t_setup(nfc_callback, NULL);
  //APP_ERROR_CHECK(err_code);

  memset(nrf5_sdk14_nfc_state.nfc_ndef_msg, 0, sizeof(nrf5_sdk14_nfc_state.nfc_ndef_msg));

  /* Run Read-Write mode for Type 4 Tag platform */
  err_code |= nfc_t4t_ndef_rwpayload_set(nrf5_sdk14_nfc_state.nfc_ndef_msg, sizeof(nrf5_sdk14_nfc_state.nfc_ndef_msg));
  //APP_ERROR_CHECK(err_code);

  /* Start sensing NFC field */
  err_code |= nfc_t4t_emulation_start();
  //APP_ERROR_CHECK(err_code);

  nrf5_sdk14_nfc_state.initialized = true;

  return platform_to_ruuvi_error(&err_code);
}

ruuvi_status_t nfc_uninit(void)
{
  if (!nrf5_sdk14_nfc_state.initialized) { return RUUVI_ERROR_INVALID_STATE; }

  ruuvi_status_t err_code = nfc_t4t_emulation_stop();
  err_code |= nfc_t4t_done();
  if (RUUVI_SUCCESS == err_code) { nrf5_sdk14_nfc_state.initialized = false; }

  return platform_to_ruuvi_error(&err_code);
}

// NFC is always polled by master, we cannot "push" data.
ruuvi_status_t nfc_process_synchronous(void)
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
ruuvi_status_t nfc_message_get(ruuvi_communication_message_t* msg)
{
  if (NULL == msg) { return RUUVI_ERROR_NULL; }
  ruuvi_status_t err_code = RUUVI_SUCCESS;

  // If we're at index 0, parse message into records
  if (!msg_index)
  {
    uint32_t desc_buf_len = sizeof(desc_buf);
    uint32_t data_lenu32 = sizeof(nfc_rx_buf);
    err_code = ndef_msg_parser(desc_buf,
                               &desc_buf_len,
                               &(nfc_rx_buf[2]), //Skip NFCT4T length bytes
                               &data_lenu32);
    // XXX debug depending on NRF_LOG
    ndef_msg_printout((nfc_ndef_msg_desc_t*) desc_buf);
  }

  //As long as there is a new message, parse the payload into Ruuvi Message.
  if (msg_index++ < ((nfc_ndef_msg_desc_t*)desc_buf)->record_count)
  {

    nfc_ndef_record_desc_t* const p_rec_desc = ((nfc_ndef_msg_desc_t*)desc_buf)->pp_record[msg_index];
    nfc_ndef_bin_payload_desc_t* p_bin_pay_desc = p_rec_desc->p_payload_descriptor;
    // Data length check
    if (p_bin_pay_desc->payload_length > msg->payload_length) { err_code = RUUVI_ERROR_DATA_SIZE; }
    else {
      memcpy(msg->payload, (uint8_t*)p_bin_pay_desc->p_payload, p_bin_pay_desc->payload_length);
      msg->payload_length = p_bin_pay_desc->payload_length;
    }
  }
  else { err_code = RUUVI_ERROR_NOT_FOUND; }
  // If no more records could be parsed, reset buffer and message counter
  if (RUUVI_SUCCESS != err_code)
  {
    memset(nfc_rx_buf, 0, sizeof(nfc_rx_buf));
    msg_index = 0;
    nrf5_sdk14_nfc_state.rx_updated = false;
  }

  return err_code;
}

// ruuvi_status_t nfc_on_connect(void);
// ruuvi_status_t nfc_on_diconnect(void);
// ruuvi_status_t nfc_is_connected(void);
ruuvi_status_t nfc_message_put(ruuvi_communication_message_t* msg);

#endif