#ifndef RUUVI_INTERFACE_GPIO_H
#define RUUVI_INTERFACE_GPIO_H
#include "ruuvi_driver_error.h"
#include <stdbool.h>
/**
 * @addtogroup GPIO
 * @{
 */
/**
* @file ruuvi_interface_gpio_test.h
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2019-02-20
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
*
* Test functionality defined in @ref ruuvi_interface_gpio.h
*
*/

/**
 * @brief Test GPIO module
 *
 * - Interface must return RUUVI_DRIVER_SUCCESS after first call.
 * - Interface must return RUUVI_DRIVER_ERROR_INVALID_STATE when called while already initialized.
 * - Interface must return RUUVI_DRIVER_SUCCESS when called after uninitialization.
 *
 *
 *
 * @return @c true if test passes, @c false on error.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_init_test(void);

/**
 * @brief Configure a pin of a port into a mode.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param pin[in] Pin number. On multi-port ICs port 0 or A is first, and port 1 or B is second etc. Pin number is <tt> port_index * pins_in_port + pin_index. </tt>
 * @param mode[in] Mode to set the pin to. See @ref ruuvi_interface_gpio_mode_t for possible values.
 *
 * @return @ref RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @return @ref RUUVI_DRIVER_ERROR_NOT_SUPPORTED if underlying platform does not support given mode.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_configure(const uint8_t pin,
    const ruuvi_interface_gpio_mode_t mode);

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
ruuvi_driver_status_t ruuvi_interface_gpio_write(const uint8_t pin,
    const ruuvi_interface_gpio_state_t state);

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
ruuvi_driver_status_t ruuvi_interface_gpio_read(const uint8_t pin,
    ruuvi_interface_gpio_state_t* const p_state);
/*@}*/
#endif