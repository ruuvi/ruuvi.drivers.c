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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/** @defgroup test_driver Driver tets
 *  Functions to test drivers.
 *  @{
 */

/** @brief function pointer to print test information */
typedef void (*rd_test_print_fp) (const char * const msg);

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
 * @param[in] expect Expected value.
 * @param[in] precision Number of decimals which must match. Negative values are allowed.
 * @param[in] check Value to check.
 *
 */
bool rd_expect_close (const float expect, const int8_t precision,
                      const float check);

/** @} */ // End of group Driver tests
#endif
