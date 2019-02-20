/**
 * RTC interface
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_RTC_H
#define RUUVI_INTERFACE_RTC_H
#include "ruuvi_driver_error.h"

/**
 * Initializes RTC at 0 ms.
 *
 * Returns RUUVI_SUCCESS if no error occured, error code otherwise.
 **/
ruuvi_driver_status_t ruuvi_interface_rtc_init(void);

/**
  * Stop RTC if applicable.
  *
  * Returns RUUVI_SUCCESS if no error occured, error code otherwise.
  **/
ruuvi_driver_status_t ruuvi_interface_rtc_uninit(void);

/**
  * Return number of milliseconds since RTC init, RUUVI_DRIVER_UINT64_INVALID if RTC is not running
  **/
uint64_t ruuvi_interface_rtc_millis(void);

#endif