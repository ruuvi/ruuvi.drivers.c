#ifndef TIMER_H
#define TIMER_H

/** 
 * Timer abstraction. Allows creating single-shot and repeated timers which call a function at interval.
 * Timer IDs must be defined with PLATFORM_TIMER_ID_DEF(timer_id)
 * The timer will schedule the functions to run, so there might be a lot of jitter in timeout.
 */

#include "ruuvi_error.h"
#include "platform_timer.h"
#include <stdbool.h>

typedef enum { 
  RUUVI_TIMER_MODE_SINGLE_SHOT, 
  RUUVI_TIMER_MODE_REPEATED 
}ruuvi_timer_mode_t;

typedef void(*ruuvi_timer_timeout_handler_t)(void* p_context);

// Calls whatever initialization is required by application timers
ruuvi_status_t platform_timers_init(void);

//return true if timers have been successfully initialized.
bool platform_timers_is_init(void);

/* Function for creating a timer instance 
 *
 * @param p_timer_id pointer to timer id, outputs ID which can be used to control the timer
 * @param mode mode of the timer, single shot or repeated
 * @param timeout_handler function which gets called 
 */ 
ruuvi_status_t platform_timer_create (platform_timer_id_t const *p_timer_id, ruuvi_timer_mode_t mode, ruuvi_timer_timeout_handler_t timeout_handler);

// Function for starting a timer.
// @param timer_id id of timer to control
// @param timeout (or interval) of timer in milliseconds
// @param p_context generic pointer which will be passed to timeout handler
ruuvi_status_t platform_timer_start (platform_timer_id_t timer_id, uint32_t ms, void *p_context);

// Function for stopping the specified timer.
// @param timer_id id of timer to stop
ruuvi_status_t platform_timer_stop (platform_timer_id_t timer_id);

// Function for stopping all running timers. 
// ret_code_t  app_timer_stop_all (void)
 
// uint8_t   app_timer_op_queue_utilization_get (void)
//   Function for getting the maximum observed operation queue utilization. More...
 
// void  app_timer_pause (void)
//   Function for pausing RTC activity which drives app_timer. More...
 
// void  app_timer_resume (void)
//   Function for resuming RTC activity which drives app_timer. More...

#endif