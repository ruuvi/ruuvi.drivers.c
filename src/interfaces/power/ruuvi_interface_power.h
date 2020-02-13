#ifndef RUUVI_INTERFACE_POWER_H
#define RUUVI_INTERFACE_POWER_H
/**
 * @defgroup Power CPU power
 */
/*@{*/
/**
 * @file ruuvi_interface_power.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-01
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for controlling CPU-integrated regulators and system power modes.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
/** @brief Enable implementation selected by application */
#if RI_POWER_ENABLED
#  define RUUVI_NRF5_SDK15_POWER_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif
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
} ri_power_regulators_t;

/**
 * @brief Enable given regulators.
 *
 * The implementation must work regardless of software radio state, i.e.
 * on S132 on nRF52 the function must check if softdevice is running and call softdevice wrapper to
 * DC/DC if it is and write registers directly if SD is not running.
 *
 * @param[in] regulators binary flags of regulators to enable.
 * @return RD_SUCCESS on success, error code from stack in case of a error.
 */
rd_status_t ri_power_regulators_enable (const ri_power_regulators_t regulators);

/**
 * @brief Reset IC.
 *
 * This function attempts to reset and restart the program as closely as possible to a power cycle.
 *
 * @warning This functions affects only the CPU, any peripheral sensors must be reset separately.
 * @warning Some registers might retain their values across soft resets.
 */
void ri_power_reset (void);

/**
 * @brief Enter bootloader
 *
 * This function attempts to enter bootloader. It's main purpose is to provide a wireless recovery
 * mechanism on fatal error.
 *
 * @warning Behaviour is undefined if bootloader is not onboard.
 */
void ri_power_enter_bootloader (void);

/*@}*/
#endif