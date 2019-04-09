/**
 * Ruuvi radio interface.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_COMMUNICATION_RADIO_H
#define RUUVI_INTERFACE_COMMUNICATION_RADIO_H
#include "ruuvi_driver_error.h"
#include <stdint.h>

typedef enum
{
  RUUVI_INTERFACE_COMMUNICATION_RADIO_BEFORE,
  RUUVI_INTERFACE_COMMUNICATION_RADIO_AFTER
} ruuvi_interface_communication_radio_activity_evt_t;

typedef enum
{
  RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT = 0,
  RUUVI_INTERFACE_COMMUNICATION_RADIO_ADVERTISEMENT,
  RUUVI_INTERFACE_COMMUNICATION_RADIO_GATT,
  RUUVI_INTERFACE_COMMUNICATION_RADIO_MESH
} ruuvi_interface_communication_radio_user_t;

/**
 *  Type of radio activity interrupt. This is common to all radio modules, i,e, the callback gets called for every radio action.
 */
typedef void(*ruuvi_interface_communication_radio_activity_interrupt_fp_t)(
  const ruuvi_interface_communication_radio_activity_evt_t evt);

// Enable / disable radio stacks
ruuvi_driver_status_t ruuvi_interface_communication_radio_init(
  const ruuvi_interface_communication_radio_user_t handle);
ruuvi_driver_status_t ruuvi_interface_communication_radio_uninit(
  const ruuvi_interface_communication_radio_user_t handle);

/**
 * Writes maximum 64-bit unique address of the device to the pointer. This address
 * may be changed during runtime. The address is identifier of the device on radio network,
 * such as BLE MAC address.
 *
 * parameter address: Output, value of address.
 *
 * return RUUVI_DRIVER_SUCCESS on success
 * return RUUVI_DRIVER_ERROR_NOT_SUPPORTED if address cannot be returned on given platform
 */
ruuvi_driver_status_t ruuvi_interface_communication_radio_address_get(
  uint64_t* const address);

/**
 * Setup radio activity interrupt
 *
 * parameter handler: Function to call on radio event NULL to disable radio-level callback, however module-level callbacks (advertising, GATT etc) will be called.
 */
void ruuvi_interface_communication_radio_activity_callback_set(
  const ruuvi_interface_communication_radio_activity_interrupt_fp_t handler);

#endif