#ifndef YIELD_H
#define YIELD_H
#include "nrf_error.h"

typedef ret_code_t (*yield_fptr_t)(void);

/** Call function which will release execution / go to sleep **/
ret_code_t platform_yield(void);

/** Setup yield function **/
void yield_set(yield_fptr_t yield_ptr);

/** delay given number of milliseconds **/
void platform_delay_ms(uint32_t time);

/** delay given number of microseconds **/
void platform_delay_us(uint32_t time);

#endif