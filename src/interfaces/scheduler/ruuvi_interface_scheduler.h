#ifndef RUUVI_INTERFACE_SCHEDULER_H
#define RUUVI_INTERFACE_SCHEDULER_H
/**
 * @defgroup scheduler Interface for scheduling tasks to be executed later.
 *
 */
/** @{ */
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
 * @brief Initialize scheduler.
 *
 * Allocates memory for scheduler task queue. Because memory allocation is static,
 * the size has to be defined in macors RI_SCHEDULER_SIZE and RI_SCHEDULER_LENGTH.
 * The RI_SCHEDULER_SIZE defines maximum size of scheduler event and
 * RI_SCHEDULER_LENGTH defines maximum number of events that can be queued.
 *
 *
 * @retval RD_SUCCESS on success.
 * @retval error code from stack on error.
 */
rd_status_t ri_scheduler_init (void);

/**
 * @brief Type definition for scheduler event handler.
 *
 * @param[in] p_event_data Data for the event handler.
 * @param[in] event_size Size of the event data. Must be smaller than or equal to max_event_size.
 *
 */
typedef void (*ruuvi_scheduler_event_handler_t) (void * p_event_data,
        uint16_t event_size);

/**
 *  @brief Executes all scheduled tasks.
 *
 *  If task schedules itself to be run immediately this will be run in a
 *  never-ending loop, without sleeping.
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
 * @retval RD_ERROR_INVALID_LENGTH if event data is larger than task queue event size.
 * @retval RD_ERROR_NO_MEM if the task cannot be added to queue due to queue full.
 */
rd_status_t ri_scheduler_event_put (const void * const p_event_data,
                                    const uint16_t event_size, const ruuvi_scheduler_event_handler_t handler);

/**
 * @brief Uninitialize scheduler.
 *
 * Scheduler has to be re-initialized after uninitialization, and all the previous
 * tasks not yet executed are discarded. As the memory is statically allocated,
 * no memory is freed.
 */
rd_status_t ri_scheduler_uninit (void);

/**
 * @brief Check if scheduler is initialized.
 *
 * @retval true If scheduler is initialized.
 * @retval false If scheduler is not initialized.
 */
bool ri_scheduler_is_init (void);

/** @} */
#endif
