#include "sdk_application_config.h"
#if NRF5_SDK15_TIMER
#include "app_timer.h"

#ifndef PLATFORM_TIMER_H
#define PLATFORM_TIMER_H

#define PLATFORM_TIMER_ID_DEF(timer_id) APP_TIMER_DEF (timer_id)
#define platform_timer_id_t app_timer_id_t

#endif

#endif