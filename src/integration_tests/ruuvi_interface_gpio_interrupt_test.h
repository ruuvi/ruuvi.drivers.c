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

/** @brief Fixed 64 interrupt table size, adjust this if some device has more than 2 ports with 32 gpios each */
#define RI_GPIO_INTERRUPT_TEST_TABLE_SIZE 64

/**
 * @brief Test GPIO interrupt initialization.
 *
 * - Initialization must return @c RD_ERROR_INVALID_STATE if GPIO is uninitialized
 * - Initialization must return @c RD_SUCCESS on first call.
 * - Initialization must return @c RD_ERROR_INVALID_STATE on second call.
 * - Initialization must return @c RD_SUCCESS after uninitializtion.
 *
 * @param[in] cfg configuration of GPIO pins to test.
 *
 * @return RD_SUCCESS on success, error code on failure.
 */
rd_status_t ri_gpio_interrupt_test_init (
    const rd_test_gpio_cfg_t cfg);

/**
 * @brief Test enabling interrupt on a pin.
 *
 * Requires basic gpio functionality to work, run gpio tests first.
 * Behaviour is undefined if GPIO is uninitialized while GPIO interrupts are initialized.
 *
 * - Return RD_ERROR_INVALID_STATE if GPIO or GPIO_INTERRUPT are not initialized
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
 * @param[in] cfg pins to use for testing interrupts
 *
 * @return @ref RD_SUCCESS on success, error code on failure.
 * @warning Simultaneous interrupts may be lost. Check the underlying implementation.
 */
rd_status_t ri_gpio_interrupt_test_enable (const rd_test_gpio_cfg_t cfg);

/**
 * @brief Run all GPIO interrupt integration tests
 *
 * @param[in] printfp Function pointer to which test result strings are sent.
 * @param[in] input  Pin used to check the state of output pin.
 * @param[in] output Pin being toggled.
 *
 * @return false if there are no errors, true otherwise.
 */
bool ri_gpio_interrupt_run_integration_test (const rd_test_print_fp printfp,
        const ri_gpio_id_t input, const ri_gpio_id_t output);

/** @} */
#endif
