#ifndef RUUVI_DRIVER_TEST_H
#define RUUVI_DRIVER_TEST_H
/**
 * @file ruuvi_driver_test.h
 * @author Otso Jousimaa
 * @date 2019-07-10
 * @brief Functions for testing drivers
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 * Run tests for drivers.
 */
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_test.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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
 * @code{.c}
 * float expect = 0.12f;
 * float check  = 0.127f;
 * int8_t precision = 2;
 * ruuvi_library_expect_close(expect, precision, check); // true
 * expect = 120f;
 * check  = 127f;
 * precision = -1;
 * ruuvi_library_expect_close(expect, precision, check); // true
 * @endcode
 *
 * @param expect[in] Expected value.
 * @param precision[in] Number of decimals which must match.
 * @param check[in] Value to check
 *
 */
bool ruuvi_driver_expect_close(const float expect, const int8_t precision,
                               const float check);

/**
 * @brief Register a test as being run. 
 * Increments counter of total tests.
 * Read results with ruuvi_driver_test_status
 *
 * @param passed[in]: True if your test was successful.
 *
 * @return RUUVI_DRIVER_SUCCESS
 */
ruuvi_driver_status_t ruuvi_driver_test_register(const bool passed);

/**
 * Get total number of tests run and total number of tests passed.
 *
 * @param[out] total  pointer to value which will be set to the number of tests run
 * @param[out] passed pointer to value which will be set to the number of tests passed
 *
 * return RUUVI_DRIVER_SUCCESS
 */
ruuvi_driver_status_t ruuvi_driver_test_status(size_t* const total, size_t* const passed);

/** @} */ // End of group Driver tests
#endif