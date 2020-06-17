#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_scheduler.h"
#if RUUVI_NRF5_SDK15_SCHEDULER_ENABLED

#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"

#include "sdk_errors.h"
#include "app_scheduler.h"

static bool m_is_init = false;

rd_status_t ri_scheduler_init ()
{
    rd_status_t err_code = RD_SUCCESS;

    if (m_is_init)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        m_is_init = true;
        APP_SCHED_INIT (RI_SCHEDULER_SIZE, RI_SCHEDULER_LENGTH);
    }

    return err_code;
}

rd_status_t ri_scheduler_execute (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (m_is_init)
    {
        app_sched_execute();
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t ri_scheduler_event_put (void const * p_event_data,
                                    uint16_t event_size, ruuvi_scheduler_event_handler_t handler)
{
    ret_code_t err_code = NRF_SUCCESS;

    if (NULL == handler)
    {
        err_code |= NRF_ERROR_NULL;
    }
    else if (m_is_init)
    {
        err_code = app_sched_event_put (p_event_data, event_size,
                                        (app_sched_event_handler_t) handler);
    }
    else
    {
        err_code |= NRF_ERROR_INVALID_STATE;
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

// No implementation needed.
rd_status_t ri_scheduler_uninit (void)
{
    m_is_init = false;
    return RD_SUCCESS;
}

bool ri_scheduler_is_init (void)
{
    return m_is_init;
}
#endif
