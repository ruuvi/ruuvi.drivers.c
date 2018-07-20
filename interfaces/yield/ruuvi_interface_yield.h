#ifndef RUUVI_INTERFACE_YIELD_H
#define RUUVI_INTERFACE_YIELD_H
#include "ruuvi_driver_error.h"

/**
 * Yield function. 
 *
 * Stops the execution of the thread, execution will continue once other threads have run.
 * If there are no other threads or we're running in a single-threaded environment, go to sleep
 * until an event occurs.
 */
typedef ruuvi_driver_status_t (*yield_fptr_t)(void);

/** 
 * Initializes yield, for example inits CPU usage timers 
 *
 * Returns RUUVI_SUCCESS if no error occured, error code otherwise
 **/
ruuvi_driver_status_t ruuvi_platform_yield_init(void);

/** 
  * Function which will release execution / go to sleep until next event occurs
  *
  * Returns RUUVI_SUCCESS if no error occured, error code otherwise
  **/
ruuvi_driver_status_t ruuvi_platform_yield(void);

/** 
  * Set yield function of type yield_fptr_t.
  *
  * Return RUUVI_SUCCESS on success, error code otherwise.
  **/
ruuvi_driver_status_t ruuvi_platform_yield_set(yield_fptr_t yield_ptr);

/** 
  * Delay given number of milliseconds 
  *
  * Return RUUVI_SUCCESS on success, error code otherwise.
  **/
ruuvi_driver_status_t platform_delay_ms(uint32_t time);

/** 
  * delay given number of microseconds 
  *
  * Return RUUVI_SUCCESS on success, error code otherwise.
  **/
ruuvi_driver_status_t platform_delay_us(uint32_t time);

#endif