#include "sdk_application_config.h"
#if NRF5_SDK15_SCHEDULER

#include "ruuvi_error.h"
#include "interface_scheduler.h"
#include "sdk_errors.h"
#include "app_scheduler.h"

ruuvi_status_t platfrom_scheduler_execute (void)
{
  app_sched_execute();
  return RUUVI_SUCCESS;
}

ruuvi_status_t platfrom_scheduler_event_put (void const *p_event_data, uint16_t event_size, ruuvi_scheduler_event_handler_t handler)
{
  ret_code_t err_code = app_sched_event_put(p_event_data, event_size, (app_sched_event_handler_t) handler);
  return platform_to_ruuvi_error(&err_code);
}

#endif