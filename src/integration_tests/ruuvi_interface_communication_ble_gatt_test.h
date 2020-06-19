#ifndef RUUVI_INTERFACE_COMMUNICATION_BLE_GATT_TEST_H
#define RUUVI_INTERFACE_COMMUNICATION_BLE_GATT_TEST_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_interface_communication.h"
#include <stdbool.h>

/**
 * @addtogroup BLE
 * @{
 */
/**
 * @file ruuvi_interface_communication_ble_gatt_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-06-18
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Test implementation of BLE interface.
 */

#define TEST_GATT_PACKET_LEN (20U) //!< Bytes of GATT test packet
#define TEST_GATT_PACKET_NUM (10000U) //!< Number of packets to send in throughput test.

/*
 * @brief Run BLE integration tests.
 *
 * Writing and reading requires external BLE device which connects to DUT
 * and register to NUS TX notifications on all tested PHYs.
 *
 * @param[in] printfp Function pointer to which test JSON is sent.
 * @retval true if error occured in test.
 * @retval false if no errors occured.
 * @warning This test requires external BLE Central device to connect.
 */
bool ri_communication_ble_gatt_run_integration_test (const rd_test_print_fp printfp,
        const ri_radio_modulation_t modulation);
/** @} */
#endif
