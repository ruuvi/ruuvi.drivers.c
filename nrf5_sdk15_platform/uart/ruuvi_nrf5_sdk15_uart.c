#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_UART_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_uart.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_nrf5_sdk15_gpio.h"
#include "nrfx_uarte.h"

#ifndef RUUVI_NRF5_SDK15_UART_LOG_LEVEL
  #define LOG_LEVEL RUUVI_INTERFACE_LOG_INFO
#else
  #define LOG_LEVEL RUUVI_NRF5_SDK15_UART_LOG_LEVEL
#endif
#define LOG(msg)  (ruuvi_interface_log(LOG_LEVEL, msg))
#define LOGD(msg)  (ruuvi_interface_log(RUUVI_INTERFACE_LOG_DEBUG, msg))

static bool m_uart_is_init = false;
static nrfx_uarte_t m_uart = NRFX_UARTE_INSTANCE(0);

// Convert constants from Ruuvi to nRF52 SDK
static nrf_uarte_baudrate_t ruuvi_to_nrf_baudrate(const ruuvi_interface_uart_baud_t baud)
{
  switch(baud)
  {
    case RUUVI_INTERFACE_UART_BAUD_9600:
      return NRF_UARTE_BAUDRATE_9600;

    case RUUVI_INTERFACE_UART_BAUD_115200:
    default:
      return NRF_UARTE_BAUDRATE_115200;
  }
}

// Handle UART events
static void uart_handler(nrfx_uarte_event_t const* p_event, void* p_context)
{
  switch(p_event->type)
  {
    case NRFX_UARTE_EVT_TX_DONE: ///< Requested TX transfer completed.
      LOG("TX\r\n");
      break;

    case NRFX_UARTE_EVT_RX_DONE: ///< Requested RX transfer completed.
      LOG("RX\r\n");
      break;

    case NRFX_UARTE_EVT_ERROR:   ///< Error reported by UART peripheral.
      LOG("!\r\n");
      break;

    default:
      LOG("?\r\n");
      break;
  }
}

ruuvi_driver_status_t ruuvi_interface_uart_init(const ruuvi_interface_uart_init_config_t*
    const config)
{
  nrfx_err_t nrf_status = NRF_SUCCESS;
  nrfx_uarte_config_t nrf_config = NRFX_UARTE_DEFAULT_CONFIG;
  nrf_config.baudrate = ruuvi_to_nrf_baudrate(config->baud);
  nrf_config.pselcts = ruuvi_to_nrf_pin_map(config->cts);
  nrf_config.pselcts = ruuvi_to_nrf_pin_map(config->rts);
  nrf_config.pselcts = ruuvi_to_nrf_pin_map(config->tx);
  nrf_config.pselcts = ruuvi_to_nrf_pin_map(config->rx);
  nrf_config.parity  = config->parity ? NRF_UARTE_PARITY_INCLUDED :
                       NRF_UARTE_PARITY_EXCLUDED;
  nrf_config.hwfc    = config->hwfc ? NRF_UARTE_HWFC_ENABLED : NRF_UARTE_HWFC_DISABLED;
  nrf_status |= nrfx_uarte_init(&m_uart, &nrf_config, uart_handler);

  if(NRF_SUCCESS == nrf_status)
  {
    m_uart_is_init = true;
  }

  return ruuvi_nrf5_sdk15_to_ruuvi_error(nrf_status);
}

bool ruuvi_interface_uart_is_init()
{
  return m_uart_is_init;
}

ruuvi_driver_status_t ruuvi_interface_uart_uninit()
{
  nrfx_uarte_uninit(&m_uart);
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_uart_send_blocking(const uint8_t* const p_tx,
    const size_t tx_len)
{
  if(NULL == p_tx) { return RUUVI_DRIVER_ERROR_NULL; }

  nrfx_err_t err_code = NRF_SUCCESS;
  err_code |= nrfx_uarte_tx(&m_uart, p_tx, tx_len);

  while(NRF_SUCCESS == err_code && nrfx_uarte_tx_in_progress(&m_uart));

  return err_code;
}

#endif