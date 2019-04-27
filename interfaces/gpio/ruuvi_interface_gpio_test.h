#ifndef RUUVI_INTERFACE_GPIO_TEST_H
#define RUUVI_INTERFACE_GPIO_TEST_H
#include "ruuvi_driver_error.h"
#include <stdbool.h>
/**
 * @addtogroup GPIO
 * @{
 */
/**
* @file ruuvi_interface_gpio_test.h
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2019-04-27
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
*
* Test functionality defined in @ref ruuvi_interface_gpio.h
*
*/

/**
 * @brief Test GPIO module initialization
 *
 * - Interface must return RUUVI_DRIVER_SUCCESS after first call.
 * - Interface must return RUUVI_DRIVER_ERROR_INVALID_STATE when called while already initialized.
 * - Interface must return RUUVI_DRIVER_SUCCESS when called after uninitialization.
 * @return @c true if test passes, @c false on error.
 */
bool ruuvi_interface_gpio_test_init(void);

/**
 * @brief Test configuring a pin of a port into a mode.
 *
 * - When both pins are in High-Z mode, input is undefined (not tested)
 * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
 * - When Input is in High-Z mode, and output mode is INPUT_PULLDOWN, input must read as LOW
 * - When Input is in INPUT_PULLUP mode, and output is in OUTPUT_LOW mode, input must read as LOW
 * - When Input is in INPUT_PULLDOWN mode, and output is in OUTPUT_HIGH mode, input must read as HIGH
 *
 * @param input[in]  Pin used to check the state of output pin
 * @param output[in] Pin being configured into various modes.
 *
 * @return @c true if test passes, @c false on error.
 */
bool ruuvi_interface_gpio_test_configure(const ruuvi_interface_gpio_id_t input, const ruuvi_interface_gpio_id_t output);

/**
 * @brief Test toggling the state of a pin of a port.
 *
 * Input is in High-Z mode. Value read by it must toggle after output pin is toggled.
 *
 * @param input[in]  Pin used to check the state of output pin.
 * @param output[in] Pin being toggled.
 *
 * @return @c true if test passes, @c false on error.
 */
bool ruuvi_interface_gpio_test_toggle(const ruuvi_interface_gpio_id_t input, const ruuvi_interface_gpio_id_t output);

/*@}*/
#endif