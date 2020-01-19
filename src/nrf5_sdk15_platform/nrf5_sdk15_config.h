#ifndef NRF5_SDK5_CONFIG_H
#define NRF5_SDK5_CONFIG_H

#include "ruuvi_driver_enabled_modules.h"

#    if (!NRF5_SDK15_CONFIGURED)
#        warning "NRF5 SDK15 is not configured, using defaults. Consider #include nrf5_sdk15_app_config.h before this file."
#    endif

#ifndef NRF_BLE_SCAN_BUFFER
/** @brief maximum data in a BLE packet */
#define NRF_BLE_SCAN_BUFFER 31
#endif

#ifndef RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG
#define RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG 1
#endif

/** @brief priorities 0,1, 4,5 are reserved by SD */
#ifndef RUUVI_NRF5_SDK15_RADIO_IRQ_PRIORITY
#define RUUVI_NRF5_SDK15_RADIO_IRQ_PRIORITY 2
#endif

#endif