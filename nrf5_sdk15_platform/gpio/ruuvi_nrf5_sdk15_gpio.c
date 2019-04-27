#include "ruuvi_driver_enabled_modules.h"

#if (RUUVI_NRF5_SDK15_GPIO_ENABLED || DOXYGEN)

#include "ruuvi_interface_gpio.h"
#include "ruuvi_driver_error.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include <stdbool.h>

/**
 * @addtogroup GPIO
 * @{
 */
/**
* @file ruuvi_nrf5_sdk15_gpio.c
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2019-04-27
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
*
* Implementations for basic GPIO writes and reads on nRF5 SDK15.
*
*/

/** @brief flag to keep track on if GPIO is initialized */
static bool m_gpio_is_init = false;

/**
 * @brief convert @ref ruuvi_interface_gpio_id_t to nRF GPIO.
 */
static inline uint8_t ruuvi_to_nrf_pin_map(const ruuvi_interface_gpio_id_t pin)
{
  return (pin.port_pin.port << 5) + pin.port_pin.pin;
}

/**
 * @brief convert nRF GPIO to @ref ruuvi_interface_gpio_id_t.
 */
static inline ruuvi_interface_gpio_id_t nrf_to_ruuvi_pin(nrf_drv_gpiote_pin_t pin)
{
  ruuvi_interface_gpio_id_t rpin = {.pin = ((pin>>5)<<8) + (pin&0x1F)};
  return rpin;
}

ruuvi_driver_status_t ruuvi_interface_gpio_init(void)
{
  if(m_gpio_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  m_gpio_is_init = true;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_gpio_uninit(void)
{
  ruuvi_driver_status_t status = RUUVI_DRIVER_SUCCESS;
  // Number of pins is defined by nrf_gpio.h
  for(uint8_t iii = 0; iii < NUMBER_OF_PINS; iii++)
  {
      ruuvi_interface_gpio_id_t pin = nrf_to_ruuvi_pin(iii);
      status |= ruuvi_interface_gpio_configure(pin, RUUVI_INTERFACE_GPIO_MODE_HIGH_Z);

  }
  return status;
}

bool  ruuvi_interface_gpio_is_init(void)
{
  return m_gpio_is_init;
}

ruuvi_driver_status_t ruuvi_interface_gpio_configure(const ruuvi_interface_gpio_id_t pin,
    const ruuvi_interface_gpio_mode_t mode)
{
  if(RUUVI_INTERFACE_GPIO_ID_UNUSED == pin.pin) { return RUUVI_DRIVER_SUCCESS; }
  const uint8_t nrf_pin = ruuvi_to_nrf_pin_map(pin);

  switch(mode)
  {
    case RUUVI_INTERFACE_GPIO_MODE_HIGH_Z:
      nrf_gpio_cfg_default(nrf_pin);
      break;

    case RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL:
      nrf_gpio_cfg_input(nrf_pin, NRF_GPIO_PIN_NOPULL);
      break;

    case RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP:
      nrf_gpio_cfg_input(nrf_pin, NRF_GPIO_PIN_PULLUP);
      break;

    case RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN:
      nrf_gpio_cfg_input(nrf_pin, NRF_GPIO_PIN_PULLDOWN);
      break;

    case RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD:
      nrf_gpio_cfg_output(nrf_pin);
      break;

    case RUUVI_INTERFACE_GPIO_MODE_OUTPUT_HIGHDRIVE:
      nrf_gpio_cfg(nrf_pin,
                   NRF_GPIO_PIN_DIR_OUTPUT,
                   NRF_GPIO_PIN_INPUT_DISCONNECT,
                   NRF_GPIO_PIN_NOPULL,
                   NRF_GPIO_PIN_H0H1,
                   NRF_GPIO_PIN_NOSENSE);
      break;

    default:
      return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_gpio_toggle(const ruuvi_interface_gpio_id_t pin)
{
  const uint8_t nrf_pin = ruuvi_to_nrf_pin_map(pin);
  nrf_gpio_pin_toggle(nrf_pin);
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_gpio_write(const ruuvi_interface_gpio_id_t pin,
    const ruuvi_interface_gpio_state_t state)
{
  const uint8_t nrf_pin = ruuvi_to_nrf_pin_map(pin);
  if(RUUVI_INTERFACE_GPIO_HIGH == state) { nrf_gpio_pin_set(nrf_pin);   }

  if(RUUVI_INTERFACE_GPIO_LOW  == state) { nrf_gpio_pin_clear(nrf_pin); }

  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_gpio_read(const ruuvi_interface_gpio_id_t pin,
    ruuvi_interface_gpio_state_t* const state)
{
  if(NULL == state) { return RUUVI_DRIVER_ERROR_NULL; }
  const uint8_t nrf_pin = ruuvi_to_nrf_pin_map(pin);

  bool high = nrf_gpio_pin_read(nrf_pin);

  if(true == high)  { *state = RUUVI_INTERFACE_GPIO_HIGH; }

  if(false == high) { *state = RUUVI_INTERFACE_GPIO_LOW;  }

  return RUUVI_DRIVER_SUCCESS;
}

#endif