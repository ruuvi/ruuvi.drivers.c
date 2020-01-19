/**
 * Ruuvi Firmware 3.x Timer tasks.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/

#ifndef  TASK_TIMER_H
#define  TASK_TIMER_H

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_timer.h"

/**
 * Initializes the timer driver.
 *
 * Returns RUUVI_DRIVER_SUCCESS on success, error code on error
 */
rd_status_t rt_timer_init (void);


#endif