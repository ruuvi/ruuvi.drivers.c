/**
 * Timer abstraction. Allows creating single-shot and repeated timers which call a function at interval.
 * Timer IDs must be defined with PLATFORM_TIMER_ID_DEF(timer_id)
 * The timer will schedule the functions to run, so there might be a lot of jitter in timeout.
 */

#ifndef RUUVI_INTERFACE_TIMER_H
#define RUUVI_INTERFACE_TIMER_H

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_enabled_modules.h"
#include <stdbool.h>

/** @brief Enable implementation selected by application */
#if RI_TIMER_ENABLED
#define RUUVI_NRF5_SDK15_TIMER_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

typedef enum
{
    RI_TIMER_MODE_SINGLE_SHOT,
    RI_TIMER_MODE_REPEATED
} ri_timer_mode_t;

typedef void * ri_timer_id_t; ///< Pointer to timer data

/**
 * Function to be called when timer event occurs.
 */
typedef void (*ruuvi_timer_timeout_handler_t) (void * p_context);

// Calls whatever initialization is required by application timers
rd_status_t ri_timer_init (void);

// Calls whatever uninitialization is required by application timers
rd_status_t ri_timer_uninit (void);

//return true if timers have been successfully initialized.
bool ri_timer_is_init (void);

/* Function for creating a timer instance
 *
 * @param[out] p_timer_id pointer to timer id, outputs ID which can be used to control the timer
 * @param[in] mode mode of the timer, single shot or repeated
 * @param[in] timeout_handler function which gets called
 * @return RD_SUCCESS if timer was created
 * @return RD_ERROR_RESOURCES if no more timers can be allocated
 * @return RD_ERROR_INVALID_STATE if timers have not been initialized
 * @return error code from stack on other error
 */
rd_status_t ri_timer_create (ri_timer_id_t *
                             p_timer_id, ri_timer_mode_t mode,
                             ruuvi_timer_timeout_handler_t timeout_handler);

/**
 * Start given timer at a mode defined in ruuvi_platform_timer_create. This operation is ignored if timer is already running.
 *
 * @param timer_id id of timer to control
 * @param timeout (or interval) of timer in milliseconds
 *
 * Return RD_SUCCESS on success, error code on start.
 */
rd_status_t ri_timer_start (ri_timer_id_t timer_id,
                            uint32_t ms);

/**
 * Stop a running timer.
 *
 * @param timer_id id of timer to stop
 * returns RD_SUCCESS on success, error code from stack on error
 */
rd_status_t ri_timer_stop (ri_timer_id_t timer_id);

#endif