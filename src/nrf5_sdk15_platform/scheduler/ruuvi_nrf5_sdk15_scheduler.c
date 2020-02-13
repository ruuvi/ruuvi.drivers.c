#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_scheduler.h"
#if RUUVI_NRF5_SDK15_SCHEDULER_ENABLED

#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"

#include "sdk_errors.h"
#include "app_scheduler.h"

// Ignore given parameters to call the macro with #defined constants
rd_status_t ri_scheduler_init (size_t event_size,
                               size_t queue_length)
{
    // Event size and queue length must be fixed at compile time. Warn user if other values are going to be used.
    if ( (event_size !=  RI_SCHEDULER_SIZE) || (queue_length != RI_SCHEDULER_SIZE))
    {
        RD_ERROR_CHECK (RD_ERROR_INVALID_PARAM, ~RD_ERROR_FATAL);
    }

    APP_SCHED_INIT (RI_SCHEDULER_SIZE,
                    RI_SCHEDULER_LENGTH);
    return RD_SUCCESS;
}

rd_status_t ri_scheduler_execute (void)
{
    app_sched_execute();
    return RD_SUCCESS;
}

rd_status_t ri_scheduler_event_put (void const * p_event_data,
                                    uint16_t event_size, ruuvi_scheduler_event_handler_t handler)
{
    ret_code_t err_code = app_sched_event_put (p_event_data, event_size,
                          (app_sched_event_handler_t) handler);
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

#endif
