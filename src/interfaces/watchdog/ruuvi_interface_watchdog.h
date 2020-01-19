/**
 * Watchdog functionality
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_WATCHDOG_H
#define RUUVI_INTERFACE_WATCHDOG_H

#include "ruuvi_driver_error.h"
#include <stdbool.h>

/**
 * Initializes watchdog module.
 * After initialization watchdog must be fed at given interval or the program will reset.
 * There is not way to uninitialize the watchdog.
 * Consider bootloader watchdog interval on setup.
 *
 * parameter interval: how often the watchdog should be fed.
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_interface_watchdog_init(uint32_t interval);

/**
 * "Feed" the watchdog, resets the watchdog timer.
 * This must be called after watchdog initialization or the program will reset.
 */
ruuvi_driver_status_t ruuvi_interface_watchdog_feed(void);

#endif