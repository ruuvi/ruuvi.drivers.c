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
}ruuvi_interface_communication_radio_activity_evt_t;

typedef enum
{
  RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT = 0,
  RUUVI_INTERFACE_COMMUNICATION_RADIO_ADVERTISEMENT,
  RUUVI_INTERFACE_COMMUNICATION_RADIO_GATT,
  RUUVI_INTERFACE_COMMUNICATION_RADIO_MESH
}ruuvi_interface_communication_radio_user_t;

typedef void(*ruuvi_interface_communication_radio_activity_interrupt_fp_t)(const ruuvi_interface_communication_radio_activity_evt_t*);

// Enable / disable radio stacks
ruuvi_driver_status_t ruuvi_interface_communication_radio_init  (const ruuvi_interface_communication_radio_user_t handle);
ruuvi_driver_status_t ruuvi_interface_communication_radio_uninit(const ruuvi_interface_communication_radio_user_t handle);

/** Set NULL to disable the interrupt **/
//ruuvi_driver_status_t ruuvi_interface_communication_radio_activity_interrupt_set(ruuvi_interface_communication_radio_activity_interrupt_fp_t);


#endif