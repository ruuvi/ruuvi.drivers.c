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
 *  err_code = ri_watchdog_init(WATCHDOG_INTERVAL_MS, on_wdt);
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
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include <stdbool.h>

/** @brief Enable implementation selected by application */
#if RI_WATCHDOG_ENABLED
#define RUUVI_NRF5_SDK15_WATCHDOG_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/** @brief Watchdog event handler function.
 *
 * Set up at initialization, gets called on watchdog triggered.
 */

typedef void (*wdt_evt_handler_t) (void);

/**
 * Initializes watchdog module.
 * After initialization watchdog must be fed at given interval or the program will reset.
 * There is not way to uninitialize the watchdog.
 * Consider bootloader watchdog interval on setup.
 *
 * @param interval_ms Watchdog will reset program unless fed faster than this.
 * @param handler Handler for watchdog event.
 *
 * @retval RD_SUCCESS on success, error code on failure.
 */
rd_status_t ri_watchdog_init (const uint32_t interval_ms,
                              const wdt_evt_handler_t handler);

/**
 * "Feed" the watchdog, resets the watchdog timer.
 * This must be called after watchdog initialization or the program will reset.
 */
rd_status_t ri_watchdog_feed (void);

#endif
