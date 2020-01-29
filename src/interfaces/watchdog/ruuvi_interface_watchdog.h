#ifndef RUUVI_INTERFACE_WATCHDOG_H
#define RUUVI_INTERFACE_WATCHDOG_H

/**
 * @defgroup Watchdog Watchdog timer functions
 */
/*@{*/
/**
 * @file ruuvi_interface_watchdog.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-01-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for Watchdog basic usage. Typical use:
 *
 * @code{.c}
 *  rd_status_t err_code = RD_SUCCESS;
 *  err_code = ri_watchdog_init(WATCHDOG_INTERVAL_MS);
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 *  while(1)
 *  {
 *    err_code = do_stuff();
 *    if(RD_SUCCESS == err_code)
 *    {
 *       (void)ri_watchdog_feed();
 *    }
 *  }
 * @endcode
 */

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
 * Return RD_SUCCESS on success, error code on failure.
 */
rd_status_t ri_watchdog_init (uint32_t interval);

/**
 * "Feed" the watchdog, resets the watchdog timer.
 * This must be called after watchdog initialization or the program will reset.
 */
rd_status_t ri_watchdog_feed (void);

#endif