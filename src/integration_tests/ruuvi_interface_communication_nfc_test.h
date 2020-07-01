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
 * @date 2020-03-03
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Test implementation of NFC interface.
 */
/*
 * @brief Run NFC integration tests.
 *
 * Writing and reading requires external NFC device which clones the content back to
 * the device running integration test. For example smartphone NFC Tools app running
 * clone function.
 *
 * @param[in] printfp Function pointer to which test JSON is sent.
 * @retval true if error occured in test.
 * @retval false if no errors occured.
 * @warning This test requires external NFC reader to copy + write copied data to device.
 */
bool ri_communication_nfc_run_integration_test (const rd_test_print_fp printfp);
/** @} */
#endif
