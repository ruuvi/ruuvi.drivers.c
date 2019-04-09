#ifndef RUUVI_INTERFACE_POWER_H
#define RUUVI_INTERFACE_POWER_H
/**
 * @defgroup Power CPU power
 */
/*@{*/
/**
 * @file ruuvi_interface_power.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-01-31
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for controlling CPU-integrated regulators and system power modes.
 *
 */
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Bitfield for regulators to enable
 */
typedef struct
{
  unsigned int DCDC_INTERNAL : 1;    //!< DC/DC for internal circuitry, i.e. nRF52832 radio
  unsigned int DCDC_HV : 1;          //!< DC/DC for high voltage, i.e. nRF52840 USB
} ruuvi_interface_power_regulators_t;

/**
 * @brief Enable given regulators.
 *
 * The implementation must work regardless of software radio state, i.e.
 * on S132 on nRF52 the function must check if softdevice is running and call softdevice wrapper to
 * DC/DC if it is and write registers directly if SD is not running.
 *
 * @param[in] regulators binary flags of regulators to enable.
 * @return RUUVI_DRIVER_SUCCESS on success, error code from stack in case of a error.
 */
ruuvi_driver_status_t ruuvi_interface_power_regulators_enable(const
    ruuvi_interface_power_regulators_t regulators);

/**
 * @brief Reset IC.
 *
 * This function attempts to reset and restart the program as closely as possible to a power cycle.
 *
 * @warning This functions affects only the CPU, any peripheral sensors must be reset separately.
 * @warning Some registers might retain their values across softresets.
 */
void ruuvi_interface_power_reset(void);


#endif