#ifndef RUUVI_INTERFACE_COMMUNICATION_NFC_TEST_H
#define RUUVI_INTERFACE_COMMUNICATION_NFC_TEST_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication_nfc.h"
#include "ruuvi_interface_communication.h"
#include <stdbool.h>

/**
 * @addtogroup NFC
 * @{
 */
/**
 * @file ruuvi_interface_communication_nfc_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Test implementation of NFC interface.
 */
/*
 * Initializes NFC hardware.
 *
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_INVALID_STATE if NFC is already initialized
 */
bool ri_communication_nfc_run_integration_test (const rd_test_print_fp printfp);
#endif