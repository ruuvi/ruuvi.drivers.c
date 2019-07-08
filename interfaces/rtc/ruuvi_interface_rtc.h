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
#include "ruuvi_driver_error.h"

/**
 * @brief Initializes RTC at 0 ms.
 *
 * @return RUUVI_SUCCESS if no error occured, error code otherwise.
 **/
ruuvi_driver_status_t ruuvi_interface_rtc_init(void);

/**
  * @brief Stop RTC if applicable.
  *
  * @return RUUVI_SUCCESS if no error occured, error code otherwise.
  **/
ruuvi_driver_status_t ruuvi_interface_rtc_uninit(void);

/**
 * @brief Get milliseconds since init. 
 *
 * @return number of milliseconds since RTC init.
 * @return @c RUUVI_DRIVER_UINT64_INVALID if RTC is not running
  **/
uint64_t ruuvi_interface_rtc_millis(void);

/*@}*/

#endif