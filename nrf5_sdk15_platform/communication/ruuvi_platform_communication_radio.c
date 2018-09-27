/**
 * Ruuvi radio interface.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_platform_external_includes.h"
#if NRF5_SDK15_COMMUNICATION_BLE4_STACK_ENABLED

#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_driver_error.h"

#include <stdbool.h>
#include <stdint.h>
#include "nordic_common.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdm.h"
#include "ble_advdata.h"
#include "sdk_errors.h"

// Handle of module which has "reserved" the radio
static ruuvi_interface_communication_radio_user_t handle = RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT;

ruuvi_driver_status_t ruuvi_interface_communication_radio_init  (const ruuvi_interface_communication_radio_user_t _handle)
{
  if(RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT != handle) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ret_code_t err_code = NRF_SUCCESS;
  handle = _handle;
  err_code = nrf_sdh_enable_request();
  RUUVI_DRIVER_ERROR_CHECK(err_code, NRF_SUCCESS);

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  err_code |= nrf_sdh_ble_default_cfg_set(NRF5_SDK15_BLE4_STACK_CONN_TAG, &ram_start);
  RUUVI_DRIVER_ERROR_CHECK(err_code, NRF_SUCCESS);

  // Enable BLE stack.
  err_code |= nrf_sdh_ble_enable(&ram_start);
  RUUVI_DRIVER_ERROR_CHECK(err_code, NRF_SUCCESS);

  return ruuvi_platform_to_ruuvi_error(&err_code);
}

ruuvi_driver_status_t ruuvi_interface_communication_radio_uninit  (const ruuvi_interface_communication_radio_user_t _handle)
{
  if(RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT == handle){ return RUUVI_DRIVER_SUCCESS; }
  if(_handle != handle) { return RUUVI_DRIVER_ERROR_FORBIDDEN; }
  sd_softdevice_disable();
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_communication_radio_address_get(uint64_t* const address)
{
  uint64_t mac = 0;
  mac |= (uint64_t)(((NRF_FICR->DEVICEADDR[1]>>8)&0xFF) | 0xC0) << 40; //2 MSB must be 11;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[1]>>0)&0xFF)  << 32;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[0]>>24)&0xFF) << 24;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[0]>>16)&0xFF) << 16;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[0]>>8)&0xFF)  << 8;
  mac |= (uint64_t)((NRF_FICR->DEVICEADDR[0]>>0)&0xFF)  << 0;
  *address = mac;
  return RUUVI_DRIVER_SUCCESS;
}

#endif