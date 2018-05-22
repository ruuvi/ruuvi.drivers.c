#ifndef INTERFACE_SCHEDULER_H
#define INTERFACE_SCHEDULER_H

/*
 * Execute scheduled tasks in round-robin order. 
 * platform_scheduler.h must implement PLATFORM_SCHEDULER_INIT(max_event_size, queue_size) which statically allocates buffers required by scheduler.
 */

#include "platform_scheduler.h"

typedef void(*ruuvi_scheduler_event_handler_t)(void *p_event_data, uint16_t event_size);
 
ruuvi_status_t platfrom_scheduler_execute (void);

ruuvi_status_t platfrom_scheduler_event_put (void const *p_event_data, uint16_t event_size, ruuvi_scheduler_event_handler_t handler);
 
// uint16_t  app_sched_queue_utilization_get (void)
//   Function for getting the maximum observed queue utilization. More...
 
// uint16_t  app_sched_queue_space_get (void)
//   Function for getting the current amount of free space in the queue. More...
 
// void  app_sched_pause (void)
//   A function to pause the scheduler. More...
 
// void  app_sched_resume (void)
//   A function to resume a scheduler. More...


#endif