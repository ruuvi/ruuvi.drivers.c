#include "ruuvi_driver_enabled_modules.h"
#if RT_RTC_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_task_rtc.h"

#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>

uint64_t task_rtc_millis()
{
    return ruuvi_interface_rtc_millis();
}

ruuvi_driver_status_t task_rtc_init (void)
{
    // Use task_rtc function to apply offset configured by user to sensor values.
    ruuvi_driver_sensor_timestamp_function_set (task_rtc_millis);
    return ruuvi_interface_rtc_init();
}
#endif