/**
 * @defgroup RTC RTC functions
 * @brief Functions for using Real-time clock onboard device.
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_rtc.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-07-07
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for basic RTC functionality.
 *
 */

#ifndef RUUVI_INTERFACE_RTC_H
#define RUUVI_INTERFACE_RTC_H
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"

#if RI_RTC_ENABLED
#  define RUUVI_NRF5_SDK15_RTC_ENABLED  RUUVI_NRF5_SDK15_ENABLED
#endif

/**
 * @brief Initializes RTC at 0 ms.
 *
 * @return RUUVI_SUCCESS if no error occured, error code otherwise.
 **/
rd_status_t ri_rtc_init (void);

/**
  * @brief Stop RTC if applicable.
  *
  * @return RUUVI_SUCCESS if no error occured, error code otherwise.
  **/
rd_status_t ri_rtc_uninit (void);

/**
 * @brief Get milliseconds since init.
 *
 * @return number of milliseconds since RTC init.
 * @return @c RUUVI_DRIVER_UINT64_INVALID if RTC is not running
  **/
uint64_t ri_rtc_millis (void);

/*@}*/

#endif