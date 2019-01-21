/**
 * Ruuvi power interface.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/

#ifndef RUUVI_INTERFACE_POWER_H
#define RUUVI_INTERFACE_POWER_H
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RUUVI_INTERFACE_POWER_REGULATORS_DISABLED 0
#define RUUVI_INTERFACE_POWER_REGULATORS_DCDC_INTERNAL (1<<0)  // DC/DC for internal circuitry, i.e. nRF52832 radio
#define RUUVI_INTERFACE_POWER_REGULATORS_DCDC_HV       (1<<1)  // DC/DC for high voltage, i.e. nRF52840 USB

typedef uint32_t ruuvi_interface_power_regulators_t;

/**
 * Enable given regulators. The implementation must work regardless of software radio state, i.e. 
 * on S132 on nRF52 the function must check if softdevice is running and call softdevice wrapper to
 * DC/DC if it is and write registers directly if SD is not running. 
 *
 * parameter regulators: binary flags of regulators to enable. 
 * return: RUUVI_DRIVER_SUCCESS on success, error code from stack in case of a error.
 */
ruuvi_driver_status_t ruuvi_interface_power_regulators_enable(const ruuvi_interface_power_regulators_t regulators);


#endif