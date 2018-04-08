#ifndef TIMER_H
#define TIMER_H

#include "ruuvi_error.h"
#include <stdbool.h>

// Calls whatever initialization is required by application timers
ruuvi_status_t platform_timers_init(void);

//return true if timers have been successfully initialized.
bool platform_timers_is_init(void);

#endif