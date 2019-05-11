/**
 * GPIO interrupt implementations on Nordic SDK15.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_GPIO_INTERRUPT_ENABLED
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"

#include <stdbool.h>
#include "nrf.h"
#include "nrf_drv_gpiote.h"

//Pointer to look-up table for event handlers
static ruuvi_interface_gpio_interrupt_fp_t* pin_event_handlers;
static uint8_t max_interrupts = 0;

static inline ruuvi_interface_gpio_id_t nrf_to_ruuvi_pin(nrf_drv_gpiote_pin_t pin)
{
  ruuvi_interface_gpio_id_t rpin = {.pin = ((pin >> 5) << 8) + (pin & 0x1F)};
  return rpin;
}

/**
 * @brief convert @ref ruuvi_interface_gpio_id_t to nRF GPIO.
 */
static inline uint8_t ruuvi_to_nrf_pin(const ruuvi_interface_gpio_id_t pin)
{
  return (pin.port_pin.port << 5) + pin.port_pin.pin;
}

ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_init(
  ruuvi_interface_gpio_interrupt_fp_t* const interrupt_table,
  const uint8_t interrupt_table_size)
{
  if(NULL == interrupt_table) { return RUUVI_DRIVER_ERROR_NULL; }

  if(!ruuvi_interface_gpio_is_init()) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  // Check module initialization status by max interrupts
  if(0 != max_interrupts) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  /* Driver initialization
     The GPIOTE driver is a shared resource that can be used by multiple modules in an application.
     Therefore, it can be initialized only once. If a module is using the driver,
     it must check if it has already been initialized by calling the function nrf_drv_gpiote_is_init.
     If this function returns false, the module must initialize the driver by calling the function nrf_drv_gpiote_init.

     The following code example shows how to initialize the driver:
  */
  ret_code_t err_code = NRF_SUCCESS;

  if(!nrf_drv_gpiote_is_init())
  {
    err_code = nrf_drv_gpiote_init();
  }

  pin_event_handlers = interrupt_table;
  max_interrupts = interrupt_table_size;
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_uninit(void)
{
  if(0 == max_interrupts) { return RUUVI_DRIVER_SUCCESS; }

  pin_event_handlers = NULL;
  max_interrupts = 0;
  return RUUVI_DRIVER_SUCCESS;
}

bool ruuvi_interface_gpio_interrupt_is_init()
{
  return (0 != max_interrupts);
}

static void in_pin_handler(const nrf_drv_gpiote_pin_t pin,
                           const nrf_gpiote_polarity_t action)
{
  if(max_interrupts <= pin) { return; }

  //Call event handler.
  ruuvi_interface_gpio_evt_t event;

  if(NULL != pin_event_handlers[pin])
  {
    switch(action)
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

    event.pin = nrf_to_ruuvi_pin(pin);
    (pin_event_handlers[pin])(event);
  }
}

ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_enable(const
    ruuvi_interface_gpio_id_t pin,
    const ruuvi_interface_gpio_slope_t slope,
    const ruuvi_interface_gpio_mode_t mode,
    const ruuvi_interface_gpio_interrupt_fp_t handler)
{
  if(!ruuvi_interface_gpio_interrupt_is_init()) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  // nRF5 devices have 32 pins per port. Pack the port-pin representation into 8 bits for interrupt table.
  uint8_t nrf_pin = ruuvi_to_nrf_pin(pin);

  if(nrf_pin >= max_interrupts) { return RUUVI_DRIVER_ERROR_INVALID_PARAM; }

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
  pin_event_handlers[nrf_pin] = handler;
  err_code |= nrf_drv_gpiote_in_init(nrf_pin, &in_config, in_pin_handler);
  nrf_drv_gpiote_in_event_enable(nrf_pin, true);
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_disable(const
    ruuvi_interface_gpio_id_t pin)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t nrf_pin = ruuvi_to_nrf_pin(pin);

  if(NULL != pin_event_handlers && NULL != pin_event_handlers[nrf_pin])
  {
    nrf_drv_gpiote_in_event_disable(nrf_pin);
    nrf_drv_gpiote_in_uninit(nrf_pin);
    pin_event_handlers[nrf_pin] = NULL;
  }

  return err_code;
}

#endif