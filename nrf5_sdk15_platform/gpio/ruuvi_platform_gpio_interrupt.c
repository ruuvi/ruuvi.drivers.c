/**
 * GPIO interrupt implementations on Nordic SDK15.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_ENABLED 
#include "ruuvi_platform_external_includes.h"

#if NRF5_SDK15_GPIO_INTERRUPT_ENABLED
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"

#include <stdbool.h>
#include "nrf.h"
#include "nrf_drv_gpiote.h"

//Pointer to look-up table for event handlers
static ruuvi_interface_gpio_interrupt_fp_t* pin_event_handlers;
static uint8_t max_interrupts = 0;

ruuvi_driver_status_t ruuvi_platform_gpio_interrupt_init(ruuvi_interface_gpio_interrupt_fp_t* interrupt_table, uint8_t interrupt_table_size)
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
  pin_event_handlers = interrupt_table;
  max_interrupts = interrupt_table_size;
  return ruuvi_platform_to_ruuvi_error(&err_code);
}

static void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  if(max_interrupts <= pin) { return; }

  //Call event handler.
  ruuvi_interface_gpio_evt_t event;
  if (NULL != pin_event_handlers[pin])
  {
    switch (action)
    {
    case NRF_GPIOTE_POLARITY_LOTOHI:
      event.slope = RUUVI_INTERFACE_GPIO_SLOPE_LOTOHI;
      break;

    case NRF_GPIOTE_POLARITY_HITOLO:
      event.slope = RUUVI_INTERFACE_GPIO_SLOPE_HITOLO;
      break;

    default:
      event.slope = RUUVI_INTERFACE_GPIO_SLOPE_UNKNOWN;
      break;
    }
    event.pin = pin;
    (pin_event_handlers[pin])(event);
  }
}

ruuvi_driver_status_t ruuvi_platform_gpio_interrupt_enable(uint8_t pin, 
                                                           ruuvi_interface_gpio_slope_t slope, 
                                                           ruuvi_interface_gpio_mode_t mode, 
                                                           ruuvi_interface_gpio_interrupt_fp_t handler)
{
  if(max_interrupts <= pin) { return RUUVI_DRIVER_ERROR_INVALID_PARAM; }
  ret_code_t err_code = NRF_SUCCESS;
  nrf_gpiote_polarity_t polarity;
  nrf_gpio_pin_pull_t pull;
  switch(slope)
  {
    case RUUVI_INTERFACE_GPIO_SLOPE_TOGGLE:
      polarity = NRF_GPIOTE_POLARITY_TOGGLE;
      break;

    case RUUVI_INTERFACE_GPIO_SLOPE_LOTOHI:
      polarity = NRF_GPIOTE_POLARITY_LOTOHI;
      break;

    case RUUVI_INTERFACE_GPIO_SLOPE_HITOLO:
      polarity = NRF_GPIOTE_POLARITY_HITOLO;
      break;

    default:
      return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  switch(mode)
  {
    case RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL:
       pull = NRF_GPIO_PIN_NOPULL;
       break;
     
     case RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP:
       pull = NRF_GPIO_PIN_PULLUP;
       break;

     case RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN:
       pull = NRF_GPIO_PIN_PULLDOWN;
       break;

     default:
       return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  //  high-accuracy mode consumes excess power
  //  is_watcher is used if we track an output pin. 
  nrf_drv_gpiote_in_config_t in_config = { .is_watcher = false,  \
                                           .hi_accuracy = false, \
                                           .pull = pull,         \
                                           .sense = polarity     \
                                         };

  pin_event_handlers[pin] = handler;
  err_code |= nrf_drv_gpiote_in_init(pin, &in_config, in_pin_handler);
  nrf_drv_gpiote_in_event_enable(pin, true);

  return ruuvi_platform_to_ruuvi_error(&err_code);
}

#endif
#endif