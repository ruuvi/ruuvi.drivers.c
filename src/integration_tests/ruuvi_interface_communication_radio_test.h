#ifndef RUUVI_INTERFACE_COMMUNICATION_RADIO_TEST_H
#define RUUVI_INTERFACE_COMMUNICATION_RADIO_TEST_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_interface_communication.h"
#include <stdbool.h>

/**
 * @addtogroup Radio
 * @{
 */
/**
 * @file ruuvi_interface_communication_radio_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-03-03
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Test implementation of Radio interface.
 */

/*
 * @brief Runs radio integration tests.
 *
 * - Verify radio initialization.
 * - Verify radio cannot be re-initialized without uninitialization.
 * - Radio callbacks are not verified as radio module does not transmit by itself.
 *
 * @param[in] printfpf Function pointer to which test JSON is sent.
 * @retval True if error occured.
 * @retval False if no errors occured.
 */
bool ri_communication_radio_run_integration_test (const rd_test_print_fp printfp);

/**
 * @brief Print used modulation to printfp.
 *
 * Example:
 *
 * @param[in] printfp Function which accepts a char* message string to print.
 * @param[in] modulation Modulation to print.
 */
void print_modulation (const rd_test_print_fp printfp,
                       const ri_radio_modulation_t modulation);

/** @} */
#endif
