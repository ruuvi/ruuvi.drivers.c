#include "sdk_application_config.h"
#if NRF5_SDK15_SCHEDULER

#ifndef PLATFORM_SCHEDULER_H
#define PLATFORM_SCHEDULER_H

#define PLATFORM_SCHEDULER_INIT(EVENT_SIZE, QUEUE_SIZE) APP_SCHED_INIT(max_event_size, queue_size)

#endif

#endif