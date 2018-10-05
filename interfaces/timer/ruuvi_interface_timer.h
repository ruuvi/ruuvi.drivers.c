/**
 * Timer abstraction. Allows creating single-shot and repeated timers which call a function at interval.
 * Timer IDs must be defined with PLATFORM_TIMER_ID_DEF(timer_id)
 * The timer will schedule the functions to run, so there might be a lot of jitter in timeout.
 */

#ifndef RUUVI_INTERFACE_TIMER_H
#define RUUVI_INTERFACE_TIMER_H

#include "ruuvi_driver_error.h"
#include "ruuvi_platform_timer.h"
#include <stdbool.h>

typedef enum {
  RUUVI_INTERFACE_TIMER_MODE_SINGLE_SHOT,
  RUUVI_INTERFACE_TIMER_MODE_REPEATED
}ruuvi_interface_timer_mode_t;

/**
 * Function to be called when timer event occurs.
 */
typedef void(*ruuvi_timer_timeout_handler_t)(void* p_context);

// Calls whatever initialization is required by application timers
ruuvi_driver_status_t ruuvi_platform_timers_init(void);

//return true if timers have been successfully initialized.
bool platform_timers_is_init(void);

/* Function for creating a timer instance
 *
 * @param p_timer_id pointer to timer id, outputs ID which can be used to control the timer
 * @param mode mode of the timer, single shot or repeated
 * @param timeout_handler function which gets called
 */
ruuvi_driver_status_t ruuvi_platform_timer_create(ruuvi_platform_timer_id_t const *p_timer_id, ruuvi_interface_timer_mode_t mode, ruuvi_timer_timeout_handler_t timeout_handler);

/**
 * Start given timer at a mode defined in ruuvi_platform_timer_create. This operation is ignored if timer is already running.
 *
 * @param timer_id id of timer to control
 * @param timeout (or interval) of timer in milliseconds
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on start.
 */
ruuvi_driver_status_t ruuvi_platform_timer_start (ruuvi_platform_timer_id_t timer_id, uint32_t ms);

/**
 * Stop a running timer.
 *
 * @param timer_id id of timer to stop
 * returns RUUVI_DRIVER_SUCCESS on success, error code from stack on error
 */
ruuvi_driver_status_t ruuvi_platform_timer_stop (ruuvi_platform_timer_id_t timer_id);

#endif