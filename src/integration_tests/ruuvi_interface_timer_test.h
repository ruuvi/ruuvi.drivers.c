#ifndef RUUVI_INTERFACE_TIMER_TEST_H
#define RUUVI_INTERFACE_TIMER_TEST_H
/**
 * @addtogroup timer
 *
 */
/** @{ */
/**
 * @file ruuvi_interface_scheduler_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-14
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Test interface functions to timer.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Run all timer integration tests.
 *
 * @param[in] printfp Function to which test JSON is passed.
 *
 * @retval false if no errors occured.
 * @retval true if error occured.
 */
bool ri_timer_integration_test_run (const rd_test_print_fp printfp);

/** @} */
#endif
