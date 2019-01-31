#ifndef RUUVI_INTERFACE_GPIO_H
#define RUUVI_INTERFACE_GPIO_H
/**
 * @defgroup Gpio Gpio functions
 * @brief Functions for digitally reading and actuating GPIO pins.
 *
 * The GPIO functions do include interrupts, but they do not include PWM,
 * ADC or DAC functions.
 */
/*@{*/
/**
 * @file ruuvi_interface_gpio.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-01-30
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for basic GPIO writes and reads 
 *
 */
#include "ruuvi_driver_error.h"
#include <stdbool.h>

#define RUUVI_INTERFACE_GPIO_PIN_UNUSED 0xFF //!< Use this value to signal that nothing should be done with this pin, i.e. UART CTS not used.

typedef enum
{
  RUUVI_INTERFACE_GPIO_MODE_HIGH_Z,          //!< High-impedance mode, electrically disconnected. 
  RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL,    //!< Input, can be read. No pull resistors
  RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP,    //!< Input, can be read. Pulled up by internal resistor, value depends on IC.
  RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN,  //!< Input, can be read. Pulled dpwn by internal resistor, value depends on IC.
  RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD, //!< Push-pull output, can be written.
  RUUVI_INTERFACE_GPIO_MODE_OUTPUT_HIGHDRIVE //!< Push-pull output, can be written. Higher current drive than standard. 
}ruuvi_interface_gpio_mode_t;

typedef enum
{
  RUUVI_INTERFACE_GPIO_LOW = false,
  RUUVI_INTERFACE_GPIO_HIGH = true
}ruuvi_interface_gpio_state_t;

/**
 * @brief Initializes GPIO module. Call this before other GPIO functions.
 *
 * @return RUUVI_DRIVER_SUCCESS on success, error code from stack on error.
 */
ruuvi_driver_status_t ruuvi_interace_gpio_init(void);

/**
 * @brief Configure a pin of a port into a mode.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param pin[in] Pin number. On multi-port ICs port 0 or A is first, and port 1 or B is second etc. Pin number is <tt> port_index * pins_in_port + pin_index. </tt>
 * @param mode[in] Mode to set the pin to. See @ref ruuvi_interface_gpio_mode_t for possible values.
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_configure(const uint8_t pin, const ruuvi_interface_gpio_mode_t mode);

/**
 * @brief Toggle the state of a pin of a port.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
* @param pin[in] Pin number. On multi-port ICs port 0 or A is first, and port 1 or B is second etc. Pin number is <tt> port_index * pins_in_port + pin_index. </tt>
 *
 * @return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an output (optional).
 */
ruuvi_driver_status_t ruuvi_interface_gpio_toggle(const uint8_t pin);

/**
 * @brief Write a pin of a port into given state
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param pin[in]   Pin number. On multi-port ICs port 0 or A is first, and port 1 or B is second etc. Pin number is <tt> port_index * pins_in_port + pin_index. </tt>
 * @param state[in] State to which the pin should be set to. See @ref ruuvi_interface_gpio_state_t for possible values
 *
 * @return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an output (optional).
 */
ruuvi_driver_status_t ruuvi_interface_gpio_write(const uint8_t pin, const ruuvi_interface_gpio_state_t state);

/**
 * @brief Read state of a pin of a port into bool high
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param pin[in]   Pin number. On multi-port ICs port 0 or A is first, and port 1 or B is second etc. Pin number is <tt> port_index * pins_in_port + pin_index. </tt>
 * @param p_state[out] Pointer to a ruuvi_interface_gpio_state_t which will be set to the state of the pin.
 *
 * @return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @return RUUVI_DRIVER_ERROR_NULL if *state is a null pointer.
 * @return RUUVI_DRIVER_ERROR_INVALID_ADDRESS if pointer is invalid for any reason (optional).
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an input (optional).
 */
ruuvi_driver_status_t ruuvi_interface_gpio_read(const uint8_t pin, ruuvi_interface_gpio_state_t* const p_state);

#endif