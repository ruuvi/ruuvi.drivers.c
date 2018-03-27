#ifndef YIELD_H
#define YIELD_H
#include "ruuvi_error.h"

typedef ruuvi_status_t (*yield_fptr_t)(void);

/** Initializes yield, for example inits CPU usage timers **/
ruuvi_status_t platform_yield_init(void);

/** Call function which will release execution / go to sleep **/
ruuvi_status_t platform_yield(void);

/** Setup yield function **/
void yield_set(yield_fptr_t yield_ptr);

/** delay given number of milliseconds **/
void platform_delay_ms(uint32_t time);

/** delay given number of microseconds **/
void platform_delay_us(uint32_t time);

#endif