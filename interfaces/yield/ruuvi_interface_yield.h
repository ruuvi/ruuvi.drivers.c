/**
 * Yield and delay function definitions.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_YIELD_H
#define RUUVI_INTERFACE_YIELD_H
#include "ruuvi_driver_error.h"

/** 
 * Initializes yield, for example inits CPU usage timers.
 *
 * Returns RUUVI_SUCCESS if no error occured, error code otherwise.
 **/
ruuvi_driver_status_t ruuvi_platform_yield_init(void);

/** 
  * Function which will release execution / go to sleep until next event occurs.
  *
  * Returns RUUVI_SUCCESS if no error occured, error code otherwise.
  **/
ruuvi_driver_status_t ruuvi_platform_yield(void);

/** 
  * Delay a given number of milliseconds.
  *
  * Return RUUVI_SUCCESS on success, error code otherwise.
  **/
ruuvi_driver_status_t platform_delay_ms(uint32_t time);

/** 
  * Delay a given number of microseconds.
  *
  * Return RUUVI_SUCCESS on success, error code otherwise.
  **/
ruuvi_driver_status_t platform_delay_us(uint32_t time);

#endif