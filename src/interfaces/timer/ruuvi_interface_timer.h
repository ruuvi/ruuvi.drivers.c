#ifndef RUUVI_INTERFACE_TIMER_H
#define RUUVI_INTERFACE_TIMER_H
/**
 * @defgroup timer Interface for timing tasks to be exeuted later.
 *
 */
/** @{ */
/**
 * @file ruuvi_interface_timer.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-07-14
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Interface functions to timer.
 *
 * Timer abstraction. Allows creating single-shot and repeated timers which call a function at interval.
 * The timer will run in interrupt context, if you need to use peripherals or spend a lot of
 * time in timer function you should use ri_scheduler to run the functions without blocking
 * timing-critical tasks.
 *
 * Typical usage:
 * @code
 * static ri_timer_id_t m_timer;
 *
 * static void do_lots_of_things_in_scheduler (void * p_event_data, uint16_t event_size)
 * {
 *     write_to_i2c_device((uint8_t*) event_data, event_size);
 * }
 *
 * static void timer_isr(void * const p_context)
 * {
 *     // Slow I2C write in scheduler
 *     i2c_tx_t* tx = (i2c_tx_t*) p_context;
 *     uint16_t event_size = (p_context[0] << 8) + p_context[1];
 *     uint8_t* p_event_data = &(p_context[2]);
 *     ri_scheduler_event_put (p_event_data, event_size, &do_lots_of_things_in_scheduler);
 *     // Fast GPIO operation can be done here.
 *     blink_led_right_away();
 * }
 *
 * // Write 10 NULLs over I2C in a scheduler 10 seconds from now.
 * rd_status_t start_my_timer(void)
 * {
 *   rd_status_t err_code = RD_SUCCESS;
 *   static i2c_tx_t tx =
 *   {
 *      .write_len = 10;
 *      .data = {0}
 *   }
 *   err_code |= ri_timer_init();
 *   // Note: Address of timer
 *   err_code |= ri_timer_create (&m_timer, RI_TIMER_MODE_SINGLE_SHOT, &timer_isr);
 *   // Note: Value of timer, statically allocated data as context.
 *   err_code |= ri_timer_start (m_timer, (10 * 1000U), &tx);
 *   return err_code
 * }
 * @endcode
 *
 *
 */


#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_enabled_modules.h"
#include <stdbool.h>

/** @brief Enable implementation selected by application */
#if RI_TIMER_ENABLED
#define RUUVI_NRF5_SDK15_TIMER_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/** @brief Single or continuous execution of task. */
typedef enum
{
    RI_TIMER_MODE_SINGLE_SHOT,
    RI_TIMER_MODE_REPEATED
} ri_timer_mode_t;

typedef void * ri_timer_id_t; ///< Pointer to timer data

/**
 * @brief Function to be called when timer times out.
 *
 * @param p_context pointer to context to be passed to handler, can be NULL.
 */
typedef void (*ruuvi_timer_timeout_handler_t) (void * const p_context);

/* @brief Calls initialization as required by application timers.
 *
 * After initialization, timers can be created, started and stopped.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if timers are already initialized.
 */
rd_status_t ri_timer_init (void);

/* @brief Calls uninitialization as required by application timers
 *
 * Deletes state of timers and stops hardware timers if possible.
 *
 **/
rd_status_t ri_timer_uninit (void);

/**
 * @brief Check if timer is initialized.
 *
 * @retval true if timers have been successfully initialized.
 * @retval false if timer is not initialized.
 */
bool ri_timer_is_init (void);

/* @brief Function for creating a timer instance.
 *
 * Timers may be statically or dynamically allocated, if statically allocated this will
 * only return a handle to instance. If dynamically, this will allocate memory for new
 * instance.
 *
 * @param[out] p_timer_id pointer to timer id, outputs ID which can be used to control the timer
 * @param[in] mode mode of the timer, single shot or repeated
 * @param[in] timeout_handler function which gets called
 * @return RD_SUCCESS if timer was created
 * @return RD_ERROR_RESOURCES if no more timers can be allocated
 * @return RD_ERROR_INVALID_STATE if timers have not been initialized
 * @return error code from stack on other error
 */
rd_status_t ri_timer_create (ri_timer_id_t * p_timer_id,
                             ri_timer_mode_t mode,
                             ruuvi_timer_timeout_handler_t timeout_handler);

/**
 * @brief Start given timer at a mode defined in ri_timer_create.
 *
 * This operation is ignored if timer is already running.
 *
 * @param[in] timer_id id of timer to control
 * @param[in] ms timeout (or interval) of timer in milliseconds.
 * @param[in] context Pointer passed to timer handler.
 *
 * Return RD_SUCCESS on success, error code on start.
 */
rd_status_t ri_timer_start (ri_timer_id_t timer_id,
                            uint32_t ms,
                            void * const context);

/**
 * Stop a running timer.
 *
 * @param timer_id id of timer to stop
 * returns RD_SUCCESS on success, error code from stack on error
 */
rd_status_t ri_timer_stop (ri_timer_id_t timer_id);
/** @} */
#endif
