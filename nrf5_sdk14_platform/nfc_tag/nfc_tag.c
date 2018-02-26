
#include "sdk_application_config.h"
#ifdef NRF5_SDK14_NFC
// Ruuvi Headers
#include "ruuvi_error.h"
#include "communication.h"

// NRF5 SDK
#include "nfc_t4t_lib.h"
#include "nfc_ndef_msg.h"
#include "nfc_uri_rec.h"
#include "nfc_text_rec.h"

static struct
{
  uint8_t connected;
  uint8_t  nfc_ndef_msg[NDEF_FILE_SIZE];
  uint8_t  m_ndef_msg_len;
}nrf5_sdk14_nfc_state;

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
      //  ret_code_t err_code;

      // bsp_board_led_on(BSP_BOARD_LED_1);

      // Schedule update of NDEF message in the flash file.
      // m_ndef_msg_len = dataLength;
      // err_code       = app_sched_event_put(NULL, 0, scheduler_ndef_file_update);
      // APP_ERROR_CHECK(err_code);
    }
    break;

  default:
    break;
  }
}

NFC_NDEF_MSG_DEF(nfc_ndef_msg, 4); // max 4 records

// Setup constant records
static uint8_t nfc_text_buf[NFC_TEXT_BUF_SIZE];
static size_t nfc_text_length;
static uint8_t nfc_uri_buf[NFC_URI_BUF_SIZE];
static size_t nfc_uri_length;
static uint8_t nfc_app_buf[NFC_APP_BUF_SIZE];
static size_t nfc_app_length;
static uint8_t nfc_data_buf[NFC_DATA_BUF_SIZE];
static size_t nfc_data_length;

ruuvi_status_t nfc_text_record_set(const uint8_t* text, size_t length)
{
  if(NULL == text) { return RUUVI_ERROR_NULL; }
  if(length >= NFC_TEXT_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  nfc_text_length = length;
  memcpy(nfc_text_buf, text, nfc_text_length);
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_uri_record_set(const uint8_t* uri, size_t length)
{
  if(NULL == uri) { return RUUVI_ERROR_NULL; }
  if(length >= NFC_URI_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  nfc_uri_length = length;
  memcpy(nfc_uri_buf, uri, nfc_uri_length);
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_app_record_set(const uint8_t scheme, const uint8_t* app, size_t length)
{
  if(NULL == app) { return RUUVI_ERROR_NULL; }
  if(length >= NFC_APP_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  nfc_app_length = length;
  memcpy(nfc_app_buf, app, nfc_app_length);
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_message_put(ruuvi_communication_message_t* message)
{
  if(NULL == message) { return RUUVI_ERROR_NULL; }
  if(message->payload_length >= NFC_DATA_BUF_SIZE) { return RUUVI_ERROR_DATA_SIZE; }
  if(nfc_data_length) { return RUUVI_ERROR_RESOURCES; } // No more space on FIFO
  nfc_data_length = message->payload_length;
  memcpy(nfc_data_buf, message->payload, nfc_data_length);
  return RUUVI_SUCCESS;
}

ruuvi_status_t nfc_process_asynchronous()
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

  ruuvi_status_t err_code = RUUVI_SUCCESS;
  err_code |= nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_ndef_msg),
                                     &NFC_NDEF_TEXT_RECORD_DESC(nfc_text_rec));

  err_code |= nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_ndef_msg),
                                     &NFC_NDEF_URI_RECORD_DESC(nfc_uri_rec));

  uint32_t msg_len = sizeof(nrf5_sdk14_nfc_state.nfc_ndef_msg);
  err_code |= nfc_ndef_msg_encode(&NFC_NDEF_MSG(nfc_ndef_msg),
                                 nrf5_sdk14_nfc_state.nfc_ndef_msg,
                                 &msg_len);

  return err_code;
}

ruuvi_status_t nfc_init(void)
{
  /* Set up NFC */
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  err_code |= nfc_t4t_setup(nfc_callback, NULL);
  //APP_ERROR_CHECK(err_code);

  /* Run Read-Write mode for Type 4 Tag platform */
  err_code |= nfc_t4t_ndef_rwpayload_set(nrf5_sdk14_nfc_state.nfc_ndef_msg, sizeof(nrf5_sdk14_nfc_state.nfc_ndef_msg));
  //APP_ERROR_CHECK(err_code);

  /* Start sensing NFC field */
  err_code |= nfc_t4t_emulation_start();
  //APP_ERROR_CHECK(err_code);


  return platform_to_ruuvi_error(&err_code);
}

ruuvi_status_t nfc_uninit(void)
{
  ruuvi_status_t err_code = nfc_t4t_emulation_stop();
  err_code |= nfc_t4t_done();
  return platform_to_ruuvi_error(&err_code);
}

// ruuvi_status_t nfc_on_connect(void);
// ruuvi_status_t nfc_on_diconnect(void);
// ruuvi_status_t nfc_is_connected(void);
// ruuvi_status_t nfc_process_asynchronous(void);
// ruuvi_status_t nfc_process_synchronous(void);
// ruuvi_status_t nfc_message_put(ruuvi_communication_message_t* msg);
// ruuvi_status_t nfc_message_get(ruuvi_communication_message_t* msg);

#endif