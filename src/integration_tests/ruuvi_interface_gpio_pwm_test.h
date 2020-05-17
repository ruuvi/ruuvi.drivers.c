#ifndef RUUVI_INTERFACE_GPIO_PWM_TEST_H
#define RUUVI_INTERFACE_GPIO_PWM_TEST_H
/**
 * @addtogroup GPIO
 * @{
 */
/**
 * @file ruuvi_interface_gpio_pwm_test.h
 * @author TODO
 * @date 2020-05-18
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Tests for PWM.
 */
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_pwm.h"
#include "ruuvi_interface_gpio_test.h"

#define RI_GPIO_PWM_TEST_FREQ_HZ   (100U) //!< Frequency of test
/** @brief Period of one PWM cycle in ms. */
#define RI_GPIO_PWM_TEST_PERIOD_MS (1000U / RI_GPIO_PWM_TEST_FREQ_HZ)
#define RI_GPIO_PWM_TEST_DC (0.5F) //!< Duty cycle in test. 
#define RI_GPIO_PWM_TEST_TIME_MS (100U) //!< Milliseconds to test. 
#define RI_GPIO_PWM_EXPECT_TRIGS (RI_GPIO_PWM_TEST_TIME_MS / RI_GPIO_PWM_TEST_PERIOD_MS)

/**
 * @brief Test GPIO PWM initialization.
 *
 * - Initialization must return @c RD_ERROR_INVALID_STATE if GPIO is uninitialized
 * - Initialization must return @c RD_SUCCESS on first call.
 * - Initialization must return @c RD_ERROR_INVALID_STATE on second call.
 * - Initialization must return @c RD_SUCCESS after uninitializtion.
 *
 * @param[in] cfg configuration of GPIO pins to test.
 *
 * @return @ref RD_SUCCESS on success, error code on failure.
 */
rd_status_t ri_gpio_interrupt_test_init (
    const rd_test_gpio_cfg_t cfg);

/**
 * @brief Test running PWM on a pin.
 *
 * Requires gpio interrupt functionality to work, run gpio interrupt tests first.
 * Behaviour is undefined if GPIO interrupts are uninitialized.
 *
 *
 *
 * @param cfg[in] pins to use for testing interrupts
 *
 * @return @ref RD_SUCCESS on success, error code on failure.
 * @warning Simultaneous interrupts may be lost. Check the underlying implementation.
 */
rd_status_t ri_gpio_pwm_test (const rd_test_gpio_cfg_t cfg);

/**
 * @brief Run all GPIO interrupt integration tests
 *
 * @param printfp[in] Function pointer to which test result strings are sent.
 * @param input[in]  Pin used to check the state of output pin.
 * @param output[in] Pin being toggled.
 *
 * @return false if there are no errors, true otherwise.
 */
bool ri_gpio_pwm_run_integration_test (const rd_test_print_fp printfp,
                                       const ri_gpio_id_t input, const ri_gpio_id_t output);
#endif
