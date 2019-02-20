/**
 * Interface to the scheduler
 *
 * Execute scheduled tasks in round-robin order.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_SCHEDULER_H
#define RUUVI_INTERFACE_SCHEDULER_H

#include "ruuvi_driver_error.h"
#include <stddef.h>

/**
 * Initialize scheduler.
 *
 * Allocates memory for scheduler task queue.
 *
 * parameter max_event_size: maximum size for event data
 * parameter queue_size: maximum number of scheduled tasks
 *
 * Returns RUUVI_DRIVER_SUCCESS on success, error code from stack on error
 */
ruuvi_driver_status_t ruuvi_interface_scheduler_init(size_t event_size, size_t queue_size);

/**
 * Type definition for scheduler event handler.
 *
 * parameter p_event_data: Data for the event handler
 * parameter event_size: Size of the event data. Must be smaller than or equal to max_event_size
 *
 */
typedef void(*ruuvi_scheduler_event_handler_t)(void *p_event_data, uint16_t event_size);

/**
 *  Executes all scheduled tasks. If task schedules itself to be run immediately this will be run in a never-ending loop, without sleeping.
 *
 *  Returns RUUVI_DRIVER_SUCCESS if queue was executed successfully.
 *  Returns error code from the stack if error occurs.
 */
ruuvi_driver_status_t ruuvi_interface_scheduler_execute(void);

/**
 * Schedule given task to be executed on next call to ruuvi_platform_scheduler_execute
 */
ruuvi_driver_status_t ruuvi_interface_scheduler_event_put (const void const *p_event_data, const uint16_t event_size, const ruuvi_scheduler_event_handler_t handler);



#endif