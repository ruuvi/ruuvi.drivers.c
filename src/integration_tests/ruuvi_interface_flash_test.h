#ifndef RUUVI_INTERFACE_FLASH_TEST_H
#define RUUVI_INTERFACE_FLASH_TEST_H
/**
 * @addtogroup Flash
 * @{
 */
/**
 * @file ruuvi_interface_flash_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-10
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Integration test flash module implementation.
 */
#include "ruuvi_driver_test.h"
#include <stdbool.h>

/**
 * @brief Run all flash tests.
 *
 * @param[in] printfp Function pointer to which test result is printed.
 * @return false if test had no errors, true otherwise.
 **/
bool ri_flash_run_integration_test (const rd_test_print_fp printfp);

/**
 * @brief Test flash initialization.
 *
 * Flash must initialize successfully on first try.
 * Flash must return RD_ERROR_INVALID_STATE on second try.
 *
 * @param[in] printfp Function pointer to which test result is printed.
 * @return false if test had no errors, true otherwise.
 */
bool ri_flash_init_test (const rd_test_print_fp printfp);

/**
 * @brief test flash uninitialization.
 *
 * Uninitialization must always be successful.
 * Initialization must be successful after uninitialization.
 *
 * store, load, free, gc must return RD_ERROR_NOT_INITIALIZED after uninitialization.
 * busy must return false after uninitialization.
 *
 * @param[in] printfp Function pointer to which test result is printed
 * @return false if test had no errors, true otherwise.
 */
bool ri_flash_uninit_test (const rd_test_print_fp printfp);

/* @} */
#endif
