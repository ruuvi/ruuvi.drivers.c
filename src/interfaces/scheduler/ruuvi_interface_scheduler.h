#ifndef RUUVI_INTERFACE_SCHEDULER_H
#define RUUVI_INTERFACE_SCHEDULER_H
/**
 * @defgroup scheduler Interface for scheduling tasks to be executed later.
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_scheduler.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-13
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Interface functions to scheduler.
 *
 * The scheduler is in drivers rather than in library as many platforms provide
 * their own implementation of the scheduler. 
 */
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include <stddef.h>

/** @brief Enable implementation selected by application */
#if RI_SCHEDULER_ENABLED
#define RUUVI_NRF5_SDK15_SCHEDULER_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/**
 * Initialize scheduler.
 *
 * Allocates memory for scheduler task queue if dynamically allocated,
 * verifies expected size if statically allocated. 
 *
 * @param[in] event_size Maximum size for event data.
 * @param[in] queue_size Maximum number of scheduled tasks.
 *
 * @retval RD_SUCCESS on success.
 * @retval error code from stack on error
 */
rd_status_t ri_scheduler_init (size_t event_size,
                               size_t queue_size);

/**
 * Type definition for scheduler event handler.
 *
 * @param[in] p_event_data Data for the event handler.
 * @param[in] event_size Size of the event data. Must be smaller than or equal to max_event_size.
 *
 */
typedef void (*ruuvi_scheduler_event_handler_t) (void * p_event_data,
        uint16_t event_size);

/**
 *  Executes all scheduled tasks. If task schedules itself to be run immediately this will be run in a never-ending loop, without sleeping.
 *
 *  @retval RD_SUCCESS if queue was executed successfully.
 *  @retval error code from the stack if error occurs.
 */
rd_status_t ri_scheduler_execute (void);

/**
 * @brief Schedule given task to be executed on next call to @ref ri_scheduler_execute
 *
 * @param[in] p_event_data Context for the scheduled event, will be stored to scheduler queue.
 *                         Use NULL if there is no context for the task.
 * @param[in] event_size Size of context in bytes. Use 0 if there is no context for the task.
 * @param[in] handler Function to handle the event. Must not be NULL
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL if handler is NULL
 * @retval RD_ERROR_DATA_SIZE if event data is larger than task queue event size.
 * @retval RD_ERROR_NO_MEM if the task cannot be added to queue due to queue full.
 */
rd_status_t ri_scheduler_event_put (const void * const p_event_data,
                                    const uint16_t event_size, const ruuvi_scheduler_event_handler_t handler);


/* @} */
#endif
