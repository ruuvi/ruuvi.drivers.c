/**
 * Ruuvi radio interface.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_COMMUNICATION_BLE4_STACK_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_communication_ble4_advertising.h"
#include "ruuvi_interface_communication_radio.h"

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdm.h"
#include "ble_advdata.h"
#include "ble_radio_notification.h"
#include "sdk_errors.h"

// Handle of module which has "reserved" the radio
static ruuvi_interface_communication_radio_user_t handle =
  RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT;

// Application callback for radio events
static ruuvi_interface_communication_radio_activity_interrupt_fp_t
on_radio_activity_callback = NULL;

/**
 * Task to run on radio activity - call event handlers of radio modules and a common radio event handler.
 * This function is in interrupt context, avoid long processing or using peripherals.
 * Schedule any long tasks in application callbacks.
 *
 * parameter active: True if radio is going to be active after event, false if radio was turned off (after tx/rx)
 */
static void on_radio_evt(bool active)
{
  ruuvi_interface_communication_radio_activity_evt_t evt = active ?
      RUUVI_INTERFACE_COMMUNICATION_RADIO_BEFORE : RUUVI_INTERFACE_COMMUNICATION_RADIO_AFTER;

  // Call advertising event handler
  if(RUUVI_INTERFACE_COMMUNICATION_RADIO_ADVERTISEMENT == handle)
  {
    ruuvi_interface_communication_ble4_advertising_activity_handler(evt);
  }

  // Call common event handler if set
  if(NULL != on_radio_activity_callback) { on_radio_activity_callback(evt); }
}

ruuvi_driver_status_t ruuvi_interface_communication_radio_init(
  const ruuvi_interface_communication_radio_user_t _handle)
{
  if(RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT != handle) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ret_code_t err_code = NRF_SUCCESS;
  handle = _handle;
  err_code = nrf_sdh_enable_request();
  RUUVI_DRIVER_ERROR_CHECK(err_code, NRF_SUCCESS);
  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;

  err_code |= nrf_sdh_ble_default_cfg_set(RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG, &ram_start);
  RUUVI_DRIVER_ERROR_CHECK(err_code, NRF_SUCCESS);
  // Define large enough GATT queue
  ble_cfg_t conn_cfg = { 0 };
  conn_cfg.conn_cfg.conn_cfg_tag = RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG;
  conn_cfg.conn_cfg.params.gatts_conn_cfg.hvn_tx_queue_size    = 20;
  err_code |= sd_ble_cfg_set(BLE_CONN_CFG_GATTS, &conn_cfg, ram_start);

  // Enable BLE stack.
  err_code |= nrf_sdh_ble_enable(&ram_start);
  
  // Enable connection event extension for faster data rate
  static ble_opt_t  opt = {0};
  opt.common_opt.conn_evt_ext.enable = true;
  err_code |= sd_ble_opt_set(BLE_COMMON_OPT_CONN_EVT_EXT, &opt);

  RUUVI_DRIVER_ERROR_CHECK(err_code, NRF_SUCCESS);
  // Initialize radio interrupts
  err_code |= ble_radio_notification_init(RUUVI_NRF5_SDK15_RADIO_IRQ_PRIORITY,
                                          NRF_RADIO_NOTIFICATION_DISTANCE_800US,
                                          on_radio_evt);
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

ruuvi_driver_status_t ruuvi_interface_communication_radio_uninit(
  const ruuvi_interface_communication_radio_user_t _handle)
{
  if(RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT == handle) { return RUUVI_DRIVER_SUCCESS; }

  if(_handle != handle) { return RUUVI_DRIVER_ERROR_FORBIDDEN; }

  sd_softdevice_disable();
  on_radio_activity_callback = NULL;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_communication_radio_address_get(
  uint64_t* const address)
{
  uint64_t mac = 0;
  mac |= (uint64_t)(((NRF_FICR->DEVICEADDR[1] >> 8) & 0xFF) | 0xC0) <<
         40; //2 MSB must be 11;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[1] >> 0) & 0xFF)  << 32;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[0] >> 24) & 0xFF) << 24;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[0] >> 16) & 0xFF) << 16;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[0] >> 8) & 0xFF)  << 8;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[0] >> 0) & 0xFF)  << 0;
  *address = mac;
  return RUUVI_DRIVER_SUCCESS;
}

void ruuvi_interface_communication_radio_activity_callback_set(
  const ruuvi_interface_communication_radio_activity_interrupt_fp_t handler)
{
  // Warn user if CB is not NULL and non-null pointer is set, do not overwrite previous pointer.
  if(NULL != handler && NULL != on_radio_activity_callback) { RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INVALID_STATE, ~RUUVI_DRIVER_ERROR_FATAL); }
  else { on_radio_activity_callback = handler; }
}

bool ruuvi_interface_communication_radio_is_init()
{
  return RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT != handle;
}

#endif