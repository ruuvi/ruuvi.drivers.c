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

/** @} */
#endif
