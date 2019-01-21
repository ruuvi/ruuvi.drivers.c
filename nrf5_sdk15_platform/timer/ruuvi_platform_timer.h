#include "ruuvi_platform_nrf5_sdk15_config.h"
#if NRF5_SDK15_TIMER_ENABLED

#ifndef PLATFORM_TIMER_H
#define PLATFORM_TIMER_H
#include "app_timer.h"

#define RUUVI_PLATFORM_TIMER_ID_DEF(timer_id) APP_TIMER_DEF (timer_id)
#define ruuvi_platform_timer_id_t app_timer_id_t

#endif

#endif