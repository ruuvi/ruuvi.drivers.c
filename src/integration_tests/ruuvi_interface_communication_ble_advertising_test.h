#ifndef RUUVI_INTERFACE_COMMUNICATION_BLE_ADVERTISING_TEST_H
#define RUUVI_INTERFACE_COMMUNICATION_BLE_ADVERTISING_TEST_H
#include "ruuvi_interface_communication_radio.h"

/**
 * @addtogroup BLE
 */
/** @{ */
/**
 * @file ruuvi_interface_communication_ble_advertising_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-06-18
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Test implementation of BLE interface.
 */

#define RI_TEST_ADV_FAST          (100U)   //!< Fastest allowed interval for connectable advertising.
#define RI_TEST_ADV_TOO_FAST      (99U)    //!< Interval shorter than allowed for non-connectable advertising.
#define RI_TEST_ADV_TOO_SLOW      (10001U) //!< Interval longer than allowed for non-connectable advertising.
#define RI_ADV_RND_DELAY          (10U)    //!< Maximum random delay in adv interval.
#define RI_TEST_ADV_SCAN_INTERVAL (200U)   //!< Scan interval for test
#define RI_TEST_ADV_SCAN_WINDOW   (100U)   //!< Scan window for test
#define RI_TEST_ADV_SCAN_CH_NUM   (3U)     //!< Number of channels to scan.

/**
 * @brief Run advertising test.
 *
 * Tests transmitting data at given modulation.
 * If selected modulation can be used as a primary PHY, it is used.
 * If selected modulation can be used only as a secondary PHY, primary
 * advertisement is sent at 1 MBit / s and secondary extended advertisement is sent
 * on selected PHY.
 *
 * Also checks that data is received while scanning, and that scanning times out
 * after scanning on all selected channels.
 *
 * Currently does not verify the actual PHY data was received / sent on.
 *
 */
bool ri_communication_ble_advertising_run_integration_test (const rd_test_print_fp
        printfp,
        const ri_radio_modulation_t modulation);

/** @} */
#endif
