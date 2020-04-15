#ifndef RUUVI_INTERFACE_COMMUNICATION_BLE_ADVERTISING_TEST_H
#define RUUVI_INTERFACE_COMMUNICATION_BLE_ADVERTISING_TEST_H
#include "ruuvi_interface_communication_radio.h"

#define RI_TEST_ADV_FAST          (100U)   //!< Fastest allowed interval for connectable advertising.
#define RI_TEST_ADV_TOO_FAST      (99U)    //!< Interval shorter than allowed for non-connectable advertising.
#define RI_TEST_ADV_TOO_SLOW      (10001U) //!< Interval longer than allowed for non-connectable advertising.
#define RI_ADV_RND_DELAY          (10U)    //!< Maximum random delay in adv interval.
#define RI_TEST_ADV_SCAN_INTERVAL (200U)   //!< Scan interval for test
#define RI_TEST_ADV_SCAN_WINDOW   (100U)   //!< Scan window for test
#define RI_TEST_ADV_SCAN_CH_NUM   (3U)     //!< Number of channels to scan.

bool ri_communication_ble_advertising_run_integration_test (const rd_test_print_fp
        printfp,
        const ri_radio_modulation_t modulation);

#endif
