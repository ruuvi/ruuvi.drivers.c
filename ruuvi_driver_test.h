/**
 * @file ruuvi_library_test.h
 * @author Otso Jousimaa
 * @date 2019-01-24
 * @brief Function for testing library functions. 
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 * Run tests for library functions. 
 */
#ifndef RUUVI_LIBRARY_TEST_H
#define RUUVI_LIBRARY_TEST_H
#include <stdbool.h>
#include <stdint.h>

/** @defgroup test_library Library tests
 *  Functions to test analysis functions for correctness.
 *  @{
 */

typedef void(*ruuvi_library_test_print_fp)(const char* const msg);

/**
 * @brief Initializes the tests. 
 *
 *
 * @param printfp[in] Pointer to function which will print out test results. Takes
 *                    a null terminated string as a parameter and returns void. 
 * @return True if all tests passed, false otherwise
 */
bool ruuvi_library_test_all_run(const ruuvi_library_test_print_fp printfp);

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
bool ruuvi_library_expect_close(const float expect, const int8_t precision, const float check);

/** @} */ // End of group Library tests
#endif