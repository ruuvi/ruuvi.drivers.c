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
 * @date 2019-01-30
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for yielding execution or delaying for a given time.
 * Underlying implememntation may enter low-power mode or block the execution
 * during delay.
 *
 */

#include "ruuvi_driver_error.h"


/**
 * @brief Initializes yielding functions.
 *
 * Initializes necessary data stuctures and peripherals (if any) for yielding.
 * May for example allocate a timer to wake the device up after delay is over.
 *
 * @return RUUVI_DRIVER_SUCCESS on success, error code from stack on error.
 */
ruuvi_driver_status_t ruuvi_interface_yield_init(void);

/**
  * @brief Function which will release execution.
  *
  * The program execution will not continue until some external event
  * continues the program.
  *
  * @return RUUVI_DRIVER_SUCCESS on success, error code from stack on error.
  * @warning This function will never return unless external event occurs.
  **/
ruuvi_driver_status_t ruuvi_interface_yield(void);

/**
  * @brief Delay a given number of milliseconds.
  *
  * This function is meant for rough timing and is not quaranteed to be exact in any manner
  * If you need exact timing use timers or for example PWM peripheral.
  *
  * @param time number of milliseconds to delay.
  * @return RUUVI_DRIVER_SUCCESS on success, error code from stack on error.
  * @warning Underlying implementation may block execution and keep CPU active leading to high power consumption
  * @warning The timing is indicative only and should not be relied for precise timing.
  **/
ruuvi_driver_status_t ruuvi_interface_delay_ms(uint32_t time);

/**
  * @brief Delay a given number of microseconds.
  *
  * This function is meant for rough timing and is not quaranteed to be exact in any manner
  * If you need exact timing use timers or for example PWM peripheral.
  *
  * @param time number of microseconds to delay.
  * @return RUUVI_DRIVER_SUCCESS on success, error code from stack on error.
  * @warning Underlying implementation may block execution and keep CPU active leading to high power consumption
  * @warning The timing is indicative only and should not be relied for precise timing.
  **/
ruuvi_driver_status_t ruuvi_interface_delay_us(uint32_t time);

/*@}*/

#endif