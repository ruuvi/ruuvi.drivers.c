#include "sdk_application_config.h"

#if NRF5_SDK15_PININTERRUPT
#include "boards.h"
#include "gpio.h"
#include "pin_interrupt.h"

#include <stdbool.h>
#include "nrf.h"
#include "nrf_drv_gpiote.h"


/**
 *  Enables GPIOTE which is required for pin interrupts
 */
ruuvi_status_t platform_pin_interrupt_init()
{
  /* Driver initialization
     The GPIOTE driver is a shared resource that can be used by multiple modules in an application.
     Therefore, it can be initialized only once. If a module is using the driver,
     it must check if it has already been initialized by calling the function nrf_drv_gpiote_is_init.
     If this function returns false, the module must initialize the driver by calling the function nrf_drv_gpiote_init.

     The following code example shows how to initialize the driver:
  */

  ret_code_t err_code = NRF_SUCCESS;
  if (!nrf_drv_gpiote_is_init())
  {
    err_code = nrf_drv_gpiote_init();
  }
  return platform_to_ruuvi_error(&err_code);
}

//Look-up table for event handlers
static pin_interrupt_fp pin_event_handlers[32] = {0};
static void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  //Call event handler.
  ruuvi_gpio_evt_t event;
  if (NULL != pin_event_handlers[pin])
  {
    switch (action)
    {
    case NRF_GPIOTE_POLARITY_LOTOHI:
      event.slope = RUUVI_GPIO_SLOPE_LOTOHI;
      break;

    case NRF_GPIOTE_POLARITY_HITOLO:
      event.slope = RUUVI_GPIO_SLOPE_HITOLO;
      break;

    default:
      event.slope = RUUVI_GPIO_SLOPE_UNKNOWN;
      break;
    }
    event.pin = pin;
    (pin_event_handlers[pin])(event);
  }
}
/**
 *  Enable interrput on pin.
 *  Polarity can be defined:
 *  NRF_GPIOTE_POLARITY_LOTOHI
 *  NRF_GPIOTE_POLARITY_HITOLO
 *  NRF_GPIOTE_POLARITY_TOGGLE
 *
 *  Message handler is called with an empty message on event.
 */
ruuvi_status_t platform_pin_interrupt_enable(uint8_t pin, ruuvi_gpio_slope_t slope, ruuvi_gpio_mode_t mode, pin_interrupt_fp handler)
{
  // PLATFORM_LOG_INFO("Enabling\r\n");
  ret_code_t err_code = NRF_SUCCESS;
  nrf_gpiote_polarity_t polarity = NRF_GPIOTE_POLARITY_TOGGLE;
  if (RUUVI_GPIO_SLOPE_LOTOHI == slope) { polarity = NRF_GPIOTE_POLARITY_LOTOHI; }
  else if (RUUVI_GPIO_SLOPE_HITOLO == slope) { polarity = NRF_GPIOTE_POLARITY_HITOLO; }

  nrf_gpio_pin_pull_t pull = NRF_GPIO_PIN_NOPULL;
  if (RUUVI_GPIO_MODE_INPUT_PULLUP == mode)          { pull = NRF_GPIO_PIN_PULLUP; }
  else if (RUUVI_GPIO_MODE_INPUT_PULLDOWN == mode)   { pull = NRF_GPIO_PIN_PULLDOWN; }
  else if (RUUVI_GPIO_MODE_HIGH_Z == mode)           { return RUUVI_ERROR_INVALID_PARAM; }
  else if (RUUVI_GPIO_MODE_OUTPUT_STANDARD == mode)  { return RUUVI_ERROR_INVALID_PARAM; }
  else if (RUUVI_GPIO_MODE_OUTPUT_HIGHDRIVE == mode) { return RUUVI_ERROR_INVALID_PARAM; }

  nrf_drv_gpiote_in_config_t in_config = { .is_watcher = false,  \
                                           .hi_accuracy = false, \
                                           .pull = pull,         \
                                           .sense = polarity     \
                                         };

  pin_event_handlers[pin] = handler;
  err_code |= nrf_drv_gpiote_in_init(pin, &in_config, in_pin_handler);

  nrf_drv_gpiote_in_event_enable(pin, true);

  return platform_to_ruuvi_error(&err_code);
}

#endif