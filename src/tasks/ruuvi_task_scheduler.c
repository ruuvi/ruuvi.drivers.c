#include "ruuvi_driver_enabled_modules.h"
#if RT_SCHEDULER_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_scheduler.h"
#include "task_scheduler.h"

ruuvi_driver_status_t task_scheduler_init (void)
{
    // These must be known at compile time for static allocation
    return ruuvi_interface_scheduler_init (RT_SCHEDULER_DATA_MAX_SIZE,
                                           RT_SCHEDULER_TASK_QUEUE_MAX_LENGTH);
}
#endif