#include "ruuvi_driver_error.h"
#include "ruuvi_interface_timer.h"
#include "ruuvi_task_timer.h"

rd_status_t rt_timer_init (void)
{
    return ri_timer_init();
}