#include "sdk_application_config.h"
#if NRF5_SDK15_BLE4_STACK

#include "ble4_stack.h"
#include "ruuvi_error.h"

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "sdk_errors.h"

#define PLATFORM_LOG_MODULE_NAME ble4_stack
#if LIS2DW12_INTERFACE_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       LIS2DW12_INTERFACE_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  LIS2DW12_INTERFACE_INFO_COLOR
#else
#define PLATFORM_LOG_LEVEL       0
#endif
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

static bool ble_stack_is_init = false;

ruuvi_status_t ble4_stack_init(void)
{
    ret_code_t err_code = NRF_SUCCESS;

    err_code = nrf_sdh_enable_request();
    // APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code |= nrf_sdh_ble_default_cfg_set(BLE_CONN_CFG_TAG_DEFAULT, &ram_start);
    // APP_ERROR_CHECK(err_code);
    PLATFORM_LOG_INFO("RAM starts at %d", ram_start);

    // Enable BLE stack.
    err_code |= nrf_sdh_ble_enable(&ram_start);

    // APP_ERROR_CHECK(err_code);
    if (NRF_SUCCESS == err_code) { ble_stack_is_init = true; }
    return platform_to_ruuvi_error(&err_code);
}

ruuvi_status_t ble4_set_name(uint8_t* name, uint8_t name_length)
{
    ble_gap_conn_sec_mode_t security;
    security.sm = 1;
    security.lv = 1;
    ret_code_t err_code = sd_ble_gap_device_name_set (&security, name, name_length);
    return platform_to_ruuvi_error(&err_code);
}

#endif