#ifndef RUUVI_INTERFACE_GPIO_INTERRUPT_TEST_H
#define RUUVI_INTERFACE_GPIO_INTERRUPT_TEST_H
/**
 * @addtogroup GPIO
 * @{
 */
/**
 * @file ruuvi_interface_gpio_interrupt_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-04-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for basic GPIO interrupt functions
 */
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include "ruuvi_interface_gpio_test.h"

/**
 * @brief Test GPIO interrupt initialization.
 *
 * - Initialization must return @c RUUVI_DRIVER_ERROR_INVALID_STATE if GPIO is uninitialized
 * - Initialization must return @c RUUVI_DRIVER_SUCCESS on first call.
 * - Initialization must return @c RUUVI_DRIVER_ERROR_INVALID_STATE on second call.
 * - Initialization must return @c RUUVI_DRIVER_SUCCESS after uninitializtion.
 * - Initialization must return @c RUUVI_DRIVER_ERROR_NULL if interrupt handler table is @c NULL.
 *
 * @param[in] cfg configuration of GPIO pins to test. Required to determine interrupt table size.
 *
 * @return @ref RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_test_init(
  const ruuvi_driver_test_gpio_cfg_t cfg);

/**
 * @brief Test enabling interrupt on a pin.
 *
 * Requires basic gpio functionality to work, run gpio tests first.
 * Behaviour is undefined if GPIO is uninitialized while GPIO interrupts are initialized.
 *
 * - Return RUUVI_DRIVER_ERROR_INVALID_STATE if GPIO or GPIO_INTERRUPT are not initialized
 * - Interrupt function shall be called exactly once when input is configured as low-to-high while input is low and
 *   input goes low-to-high, high-to-low.
 * - Interrupt function shall not be called after interrupt has been disabled
 * - Interrupt function shall be called exactly once when input is configured as high-to-low while input is low and
 *   input goes low-to-high, high-to-low.
 * - Interrupt function shall be called exactly twice when input is configured as toggle while input is low and
 *   input goes low-to-high, high-to-low.
 * - Interrupt pin shall be at logic HIGH when interrupt is enabled with a pull-up and the pin is not loaded externally
 * - Interrupt pin shall be at logic LOW when interrupt is enabled with a pull-down and the pin is not loaded externally
 *
 * @param cfg[in] pins to use for testing interrupts
 *
 * @return @ref RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @warning Simultaneous interrupts may be lost. Check the underlying implementation.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_test_enable(
  const ruuvi_driver_test_gpio_cfg_t cfg);

#endif