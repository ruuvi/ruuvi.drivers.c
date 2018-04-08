#include "sdk_application_config.h"

#if NRF5_SDK15_GPIO
#include "gpio.h"
#include "ruuvi_error.h"
#include <stdbool.h>

#include "nrf_gpio.h"

// No impl required.
ruuvi_status_t platform_gpio_init(void)
{
  return RUUVI_SUCCESS;
}

ruuvi_status_t platform_gpio_configure(uint8_t pin, ruuvi_gpio_mode_t mode)
{


  switch (mode)
  {
  case HIGH_Z:
    nrf_gpio_cfg_default(pin);
    break;

  case INPUT_NOPULL:
    nrf_gpio_cfg_input (pin, NRF_GPIO_PIN_NOPULL);
    break;

  case INPUT_PULLUP:
    nrf_gpio_cfg_input (pin, NRF_GPIO_PIN_PULLUP);
    break;

  case INPUT_PULLDOWN:
    nrf_gpio_cfg_input (pin, NRF_GPIO_PIN_PULLDOWN);
    break;

  case OUTPUT_STANDARD:
    nrf_gpio_cfg_output (pin);
    break;

  case OUTPUT_HIGHDRIVE:
    nrf_gpio_cfg (pin,
                  NRF_GPIO_PIN_DIR_OUTPUT,
                  NRF_GPIO_PIN_INPUT_DISCONNECT,
                  NRF_GPIO_PIN_NOPULL,
                  NRF_GPIO_PIN_H0H1,
                  NRF_GPIO_PIN_NOSENSE);
    break;

  default:
    return RUUVI_ERROR_INVALID_PARAM;
  }
  return RUUVI_SUCCESS;
}

ruuvi_status_t platform_gpio_set(uint8_t pin)
{
    nrf_gpio_pin_set(pin);
    return RUUVI_SUCCESS;
}

ruuvi_status_t platform_gpio_clear(uint8_t pin)
{
    nrf_gpio_pin_clear(pin);
    return RUUVI_SUCCESS;
}

ruuvi_status_t platform_gpio_toggle(uint8_t pin)
{
    nrf_gpio_pin_toggle(pin);
    return RUUVI_SUCCESS;
}

ruuvi_status_t platform_gpio_read(uint8_t pin, bool* high)
{
    *high = (bool)nrf_gpio_pin_read (pin);
    return RUUVI_SUCCESS;
}

#endif