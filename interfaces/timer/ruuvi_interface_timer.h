/**
 * Timer abstraction. Allows creating single-shot and repeated timers which call a function at interval.
 * Timer IDs must be defined with PLATFORM_TIMER_ID_DEF(timer_id)
 * The timer will schedule the functions to run, so there might be a lot of jitter in timeout.
 */

#ifndef RUUVI_INTERFACE_TIMER_H
#define RUUVI_INTERFACE_TIMER_H

#include "ruuvi_driver_error.h"
#include <stdbool.h>

typedef enum
{
  RUUVI_INTERFACE_TIMER_MODE_SINGLE_SHOT,
  RUUVI_INTERFACE_TIMER_MODE_REPEATED
} ruuvi_interface_timer_mode_t;

typedef void* ruuvi_interface_timer_id_t; ///< Pointer to timer data
/**
 * Function to be called when timer event occurs.
 */
typedef void(*ruuvi_timer_timeout_handler_t)(void* p_context);

// Calls whatever initialization is required by application timers
ruuvi_driver_status_t ruuvi_interface_timer_init(void);

// Calls whatever uninitialization is required by application timers
ruuvi_driver_status_t ruuvi_interface_timer_uninit(void);

//return true if timers have been successfully initialized.
bool ruuvi_interface_timer_is_init(void);

/* Function for creating a timer instance
 *
 * @param[out] p_timer_id pointer to timer id, outputs ID which can be used to control the timer
 * @param[in] mode mode of the timer, single shot or repeated
 * @param[in] timeout_handler function which gets called
 * @return RUUVI_DRIVER_SUCCESS if timer was created
 * @return RUUVI_DRIVER_ERROR_RESOURCES if no more timers can be allocated
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if timers have not been initialized
 * @return error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_timer_create(ruuvi_interface_timer_id_t*
    p_timer_id, ruuvi_interface_timer_mode_t mode,
    ruuvi_timer_timeout_handler_t timeout_handler);

/**
 * Start given timer at a mode defined in ruuvi_platform_timer_create. This operation is ignored if timer is already running.
 *
 * @param timer_id id of timer to control
 * @param timeout (or interval) of timer in milliseconds
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on start.
 */
ruuvi_driver_status_t ruuvi_interface_timer_start(ruuvi_interface_timer_id_t timer_id,
    uint32_t ms);

/**
 * Stop a running timer.
 *
 * @param timer_id id of timer to stop
 * returns RUUVI_DRIVER_SUCCESS on success, error code from stack on error
 */
ruuvi_driver_status_t ruuvi_interface_timer_stop(ruuvi_interface_timer_id_t timer_id);

#endif