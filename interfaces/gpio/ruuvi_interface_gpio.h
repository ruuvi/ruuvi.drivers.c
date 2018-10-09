/**
 * GPIO definitions
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_GPIO_H
#define RUUVI_INTERFACE_GPIO_H

#include "ruuvi_driver_error.h"
#include <stdbool.h>

#define RUUVI_INTERFACE_GPIO_PIN_UNUSED 0xFF; // Use this value to signal that nothing should be done with this pin, i.e. UART CTS not used.

typedef enum
{
  RUUVI_INTERFACE_GPIO_MODE_HIGH_Z,
  RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL,
  RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP,
  RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN,
  RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD,
  RUUVI_INTERFACE_GPIO_MODE_OUTPUT_HIGHDRIVE
}ruuvi_interface_gpio_mode_t;

typedef enum
{
  RUUVI_INTERFACE_GPIO_LOW = false,
  RUUVI_INTERFACE_GPIO_HIGH = true
}ruuvi_interface_gpio_state_t;

/**
 * Initializes GPIO module
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_platform_gpio_init(void);

/**
 * Configure a pin of a port into a mode.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * Parameter pin:  Pin number
 * Parameter mode: mode to set the pin to.
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_platform_gpio_configure(uint8_t pin, ruuvi_interface_gpio_mode_t mode);

/**
 * Toggle the state of a pin of a port.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * Parameter pin:  Pin number
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * May return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an output.
 */
ruuvi_driver_status_t ruuvi_platform_gpio_toggle(uint8_t pin);

/**
 * Write a pin of a port into given state
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * Parameter pin:   Pin number
 * Parameter state: State to which the pin should be set to.
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * May return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an output.
 */
ruuvi_driver_status_t ruuvi_platform_gpio_write(uint8_t pin, ruuvi_interface_gpio_state_t state);

/**
 * Read state of a pin of a port into bool high
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * Parameter pin:  Pin number
 * Parameter state: pointer to a ruuvi_interface_gpio_state_t which will be set to the state of the pin.
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * Must return RUUVI_DRIVER_ERROR_NULL if *state is a null pointer.
 * May return RUUVI_DRIVER_ERROR_INVALID_ADDRESS if pointer is invalid for any reason.
 * May return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an input.
 */
ruuvi_driver_status_t ruuvi_platform_gpio_read(uint8_t pin, ruuvi_interface_gpio_state_t* state);

#endif