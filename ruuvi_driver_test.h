/**
 * @file ruuvi_driver_test.h
 * @author Otso Jousimaa
 * @date 2019-04-27
 * @brief Functions for testing drivers
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 * Run tests for drivers.
 */
#ifndef RUUVI_DRIVER_TEST_H
#define RUUVI_DRIVER_TEST_H
#include "ruuvi_interface_gpio.h"
#include <stdbool.h>
#include <stdint.h>

/** @brief structure to configure GPIO test with input and output. These GPIOs must be physically connected on board. */
typedef struct
{
  ruuvi_interface_gpio_id_t input;  //!< Input pin used in test. Must be interrupt-capable.
  ruuvi_interface_gpio_id_t output; //!< Output pin used in test. Must be PWM-capable.
} ruuvi_driver_test_gpio_cfg_t;

/** @defgroup test_driver Driver tets
 *  Functions to test drivers.
 *  @{
 */

/** @brief function pointer to print test information */
typedef void(*ruuvi_driver_test_print_fp)(const char* const msg);

/**
 * @brief Runs the tests.
 *
 * @param printfp[in] Pointer to function which will print out test results. Takes
 *                    a null terminated string as a parameter and returns void.
 * @return True if all tests passed, false otherwise
 */
bool ruuvi_driver_test_all_run(const ruuvi_driver_test_print_fp printfp);

/**
 * @brief configure GPIO tests
 *
 * The GPIO test uses 2 GPIO pins connected to gether to verify the implementation of
 * GPIO interface on target board. The pin input is used to check the state of the output pin.
 *
 * @param cfg[in] Structure which defines pin to use as output and pin to use as input for all GPIO tests.
 */
void ruuvi_driver_test_gpio_cfg(const ruuvi_driver_test_gpio_cfg_t cfg);

/**
 * @brief Check if given value is "near enough" to what user expects.
 *
 * Check if two floats are close to another, down to precision.
 * Example
 * @code
 * float expect = 0.12f;
 * float check  = 0.127f;
 * int8_t precision = 2;
 * ruuvi_library_expect_close(expect, precision, check); // true
 * expect = 120f;
 * check  = 127f;
 * precision = -1;
 * ruuvi_library_expect_close(expect, precision, check); // true
 *
 * @param expect[in] Expected value.
 * @param precision[in] Number of decimals which must match.
 * @param check[in] Value to check
 *
 */
bool ruuvi_driver_expect_close(const float expect, const int8_t precision,
                               const float check);

/** @} */ // End of group Library tests
#endif