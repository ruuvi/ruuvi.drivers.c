#ifndef RUUVI_INTERFACE_YIELD_H
#define RUUVI_INTERFACE_YIELD_H
/**
 * @defgroup Yield Yield and delay functions
 * @brief Functions for ceasing the execution of the program, either until some event occurs
 * or until a desired time has passed.
 */
/*@{*/
/**
 * @file ruuvi_interface_yield.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-07-26
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for yielding execution or delaying for a given time.
 * Underlying implememntation may enter low-power mode or block the execution
 * during delay.
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include <stdbool.h>

/** @brief Enable implementation selected by application */
#if RI_YIELD_ENABLED
#define RUUVI_NRF5_SDK15_YIELD_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/** Function which gets called when entering / exiting sleep, configured by application.
 *
 * @param[in] active true if device is going to be active, false otherwise.
 *
 */
typedef void (*ri_yield_state_ind_fp_t) (const bool active);

/**
 * Configure sleep indication function.
 *
 * @param[in] indication function to call when entering/exiting sleep, NULL to disable
 */
void ri_yield_indication_set (const ri_yield_state_ind_fp_t
                              indication);

/**
 * @brief Initializes yielding functions.
 *
 * Initializes necessary data stuctures and peripherals (if any) for yielding.
 * May for example allocate a timer to wake the device up after delay is over.
 *
 * @return RD_SUCCESS on success, error code from stack on error.
 */
rd_status_t ri_yield_init (void);

/**
 * @brief Uninitializes yielding functions.
 *
 * Clears state from previous initializations. This should be called before
 * uninitializing timers if timer-based low-power delay is in use.
 *
 * @return RD_SUCCESS.
 */
rd_status_t ri_yield_uninit (void);

/**
 * @brief Initializes yielding functions.
 *
 * Enables using timer + RTC to shutdown the device for millisecond-sleeps.
 * @param[in] enable true to enable low-power mode, false to disable.
 *
 * @return RD_SUCCESS on success, error code from stack on error.
 */
rd_status_t ri_yield_low_power_enable (const bool enable);

/**
  * @brief Function which will release execution.
  *
  * The program execution will not continue until some external event
  * continues the program.
  *
  * @return RD_SUCCESS on success, error code from stack on error.
  * @warning This function will never return unless external event occurs.
  **/
rd_status_t ri_yield (void);

/**
  * @brief Delay a given number of milliseconds.
  *
  * This function is meant for rough timing and is not quaranteed to be exact in any manner
  * If you need exact timing use timers or for example PWM peripheral. This
  * function is affected by low-power delay enable, which uses sleep mode and timer to
  * return out of sleep.
  *
  * @param time number of milliseconds to delay.
  * @return RD_SUCCESS on success, error code from stack on error.
  * @warning Underlying implementation may block execution and keep CPU active leading to high power consumption
  * @warning The timing is indicative only and should not be relied for precise timing.
  **/
rd_status_t ri_delay_ms (uint32_t time);

/**
  * @brief Delay a given number of microseconds.
  *
  * This function is meant for rough timing and is not quaranteed to be exact in any manner
  * If you need exact timing use timers or for example PWM peripheral.
  * This function does not use low-power mode to maintain better precision on timing.
  *
  * @param time number of microseconds to delay.
  * @return RD_SUCCESS on success, error code from stack on error.
  * @warning Underlying implementation may block execution and keep CPU active leading to high power consumption
  * @warning The timing is indicative only and should not be relied for precise timing.
  **/
rd_status_t ri_delay_us (uint32_t time);

/**
  * @brief Check if current execution is in interrupt context.
  *
  * This function reads Interrupt Control and State Register (ICSR) to determine the
  * interrupt status. The register is masked with VECTACTIVE mask.
  *
  * @return true if execution is currently in interrupt context, false otherwise.
  **/
bool ri_yield_is_interrupt_context (void);

/*@}*/

#endif
