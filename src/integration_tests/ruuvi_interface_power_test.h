#ifndef RUUVI_INTERFACE_POWER_TEST_H
#define RUUVI_INTERFACE_POWER_TEST_H
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_power.h"
#include <stdbool.h>
/**
 * @addtogroup power
 * @{
 */
/**
 * @file ruuvi_interface_power_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-13
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Integration test power implementation, i.e. enabling integrated regulators.
 */

/**
 * @brief Run all Power integration tests
 *
 * @param[in] printfp Function pointer to which test result strings are sent.
 * @param[in] regulators Regulators to test. Will be uninitialized afterwards.
 *
 * @return false if there are no errors, true otherwise.
 */
bool ri_power_run_integration_test (const rd_test_print_fp printfp,
                                    const ri_power_regulators_t regulators);
/** @} */
#endif
