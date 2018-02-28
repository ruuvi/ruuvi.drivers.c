
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

#define NRF_LOG_MODULE_NAME nfc_tag
#if NFC_NDEF_MSG_PARSER_LOG_ENABLED
#define NRF_LOG_LEVEL       NFC_NDEF_MSG_PARSER_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  NFC_NDEF_MSG_PARSER_INFO_COLOR
#else // NFC_NDEF_MSG_PARSER_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // NFC_NDEF_MSG_PARSER_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

static struct
{
  uint8_t connected;
  uint8_t initialized;
  uint8_t updated;
  uint8_t  nfc_ndef_msg[NDEF_FILE_SIZE];
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
      nrf5_sdk14_nfc_state.updated = true;
    }
    break;

  default:
    break;
  }
}

NFC_NDEF_MSG_DEF(nfc_ndef_msg, 4); // max 4 records

// Setup constant records
static uint8_t nfc_text_buf[NFC_TEXT_BUF_SIZE];
static size_t nfc_text_length = 0;
static uint8_t nfc_uri_buf[NFC_URI_BUF_SIZE];
static size_t nfc_uri_length = 0;
static uint8_t nfc_app_buf[NFC_APP_BUF_SIZE];
static size_t nfc_app_length = 0;
static uint8_t nfc_data_buf[NFC_DATA_BUF_SIZE];
static size_t nfc_data_length = 0;

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
  if (nfc_data_length) { return RUUVI_ERROR_RESOURCES; } // No more space on FIFO
  nfc_data_length = message->payload_length;
  memcpy(nfc_data_buf, message->payload, nfc_data_length);
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_process_asynchronous()
{
  if (!nrf5_sdk14_nfc_state.initialized) { return RUUVI_ERROR_INVALID_STATE; }
  if (nrf5_sdk14_nfc_state.updated) { nfc_message_get(NULL); }
  if( nrf5_sdk14_nfc_state.connected) { return RUUVI_ERROR_INVALID_STATE; }
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
  // NRF_LOG_HEXDUMP_INFO(nrf5_sdk14_nfc_state.nfc_ndef_msg, msg_len);
  // nfc_message_get(NULL);

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

ruuvi_status_t nfc_message_get(ruuvi_communication_message_t* msg)
{
  uint8_t desc_buf[NFC_NDEF_PARSER_REQIRED_MEMO_SIZE_CALC(10)];
  uint32_t desc_buf_len = sizeof(desc_buf);
  uint32_t data_lenu32 = sizeof(nrf5_sdk14_nfc_state.nfc_ndef_msg);
  ret_code_t err_code = ndef_msg_parser(desc_buf,
                  &desc_buf_len,
                  &(nrf5_sdk14_nfc_state.nfc_ndef_msg[2]), //Skip NFCT4T length bytes
                  &data_lenu32);


  /**
   * @brief Function for printing the parsed contents of an NDEF message.
   *
   * @param[in] p_msg_desc Pointer to the descriptor of the message that should be printed.
   */
  ndef_msg_printout((nfc_ndef_msg_desc_t*) desc_buf);
  NRF_LOG_INFO("parse: %d", err_code);

  nrf5_sdk14_nfc_state.updated = false;

  return RUUVI_SUCCESS;
}

// ruuvi_status_t nfc_on_connect(void);
// ruuvi_status_t nfc_on_diconnect(void);
// ruuvi_status_t nfc_is_connected(void);
//
// ruuvi_status_t nfc_message_put(ruuvi_communication_message_t* msg);
//

#endif