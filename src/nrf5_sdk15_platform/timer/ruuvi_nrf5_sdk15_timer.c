#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_timer.h"
#if RUUVI_NRF5_SDK15_TIMER_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_log.h"

#include "nrf_error.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "app_timer.h"

#include <stdbool.h>

#if RI_TIMER_MAX_INSTANCES > 10
#error "Allocating over 10 timers is not supported"
#endif
#if RI_TIMER_MAX_INSTANCES > 9
APP_TIMER_DEF (timer_9);
#endif
#if RI_TIMER_MAX_INSTANCES > 8
APP_TIMER_DEF (timer_8);
#endif
#if RI_TIMER_MAX_INSTANCES > 7
APP_TIMER_DEF (timer_7);
#endif
#if RI_TIMER_MAX_INSTANCES > 6
APP_TIMER_DEF (timer_6);
#endif
#if RI_TIMER_MAX_INSTANCES > 5
APP_TIMER_DEF (timer_5);
#endif
#if RI_TIMER_MAX_INSTANCES > 4
APP_TIMER_DEF (timer_4);
#endif
#if RI_TIMER_MAX_INSTANCES > 3
APP_TIMER_DEF (timer_3);
#endif
#if RI_TIMER_MAX_INSTANCES > 2
APP_TIMER_DEF (timer_2);
#endif
#if RI_TIMER_MAX_INSTANCES > 1
APP_TIMER_DEF (timer_1);
#endif
#if RI_TIMER_MAX_INSTANCES > 0
APP_TIMER_DEF (timer_0);
#endif
#if 0 >= RI_TIMER_MAX_INSTANCES
#error "No instances enabled for application timer"
#endif

static uint8_t timer_idx = 0;  ///< Counter to next timer to allocate.
static bool m_is_init = false; ///< Flag keeping track on if module is initialized.

/**
 * @brief return free timer ID
 */
static app_timer_id_t get_timer_id (void)
{
    switch (timer_idx++)
    {
#if RI_TIMER_MAX_INSTANCES > 0

        case 0:
            return timer_0;
#endif
#if RI_TIMER_MAX_INSTANCES > 1

        case 1:
            return timer_1;
#endif
#if RI_TIMER_MAX_INSTANCES > 2

        case 2:
            return timer_2;
#endif
#if RI_TIMER_MAX_INSTANCES > 3

        case 3:
            return timer_3;
#endif
#if RI_TIMER_MAX_INSTANCES > 4

        case 4:
            return timer_4;
#endif
#if RI_TIMER_MAX_INSTANCES > 5

        case 5:
            return timer_5;
#endif
#if RI_TIMER_MAX_INSTANCES > 6

        case 6:
            return timer_6;
#endif
#if RI_TIMER_MAX_INSTANCES > 7

        case 7:
            return timer_7;
#endif
#if RI_TIMER_MAX_INSTANCES > 8

        case 8:
            return timer_8;
#endif
#if RI_TIMER_MAX_INSTANCES > 9

        case 9:
            return timer_9;
#endif

        default:
            return NULL;
    }
}

rd_status_t ri_timer_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ret_code_t nrf_code = NRF_SUCCESS;

    if (m_is_init)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    // Initialize clock if not already initialized
    else if (false == nrf_drv_clock_init_check())
    {
        nrf_code |= nrf_drv_clock_init();
        err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_code);
    }

    if (RD_SUCCESS == err_code)
    {
        nrf_drv_clock_lfclk_request (NULL);
        nrf_code |= app_timer_init();
    }

    if (NRF_SUCCESS == nrf_code)
    {
        m_is_init = true;
    }

    return ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_code) | err_code;
}

//return true if timers have been successfully initialized.
bool ri_timer_is_init (void)
{
    return m_is_init;
}

rd_status_t ri_timer_create (ri_timer_id_t *
                             p_timer_id, const ri_timer_mode_t mode,
                             const ruuvi_timer_timeout_handler_t timeout_handler)
{
    rd_status_t err_code = RD_SUCCESS;
    app_timer_mode_t nrf_mode = APP_TIMER_MODE_SINGLE_SHOT;

    if (m_is_init)
    {
        if (RI_TIMER_MODE_REPEATED == mode)
        {
            nrf_mode = APP_TIMER_MODE_REPEATED;
        }

        app_timer_id_t tid = get_timer_id();

        if (NULL != tid)
        {
            ret_code_t nrf_code = app_timer_create (&tid,
                                                    nrf_mode,
                                                    (app_timer_timeout_handler_t) timeout_handler);

            if (NRF_SUCCESS == nrf_code)
            {
                *p_timer_id = (void *) tid;
            }

            err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
        }
        else
        {
            err_code |= RD_ERROR_RESOURCES;
        }
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t ri_timer_start (const ri_timer_id_t
                            timer_id, const uint32_t ms,
                            void * const context)
{
    // Counters are 24 bits
    // nrf5 sdk_config.h has prescaler setting for timer, resolution can be traded for run time
    if (APP_TIMER_TICKS (ms) >= (1 << 24))
    {
        ri_log (RI_LOG_LEVEL_ERROR, "Timer overflow, timer not started\r\n");
        return RD_ERROR_INVALID_PARAM;
    }

    ret_code_t err_code = app_timer_start ( (app_timer_id_t) timer_id, APP_TIMER_TICKS (ms),
                                            context);
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

rd_status_t ri_timer_stop (ri_timer_id_t timer_id)
{
    ret_code_t err_code = app_timer_stop ( (app_timer_id_t) timer_id);
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

rd_status_t ri_timer_uninit()
{
    app_timer_stop_all();
    nrf_drv_clock_lfclk_release();
    timer_idx = 0;
    m_is_init = false;
    return RD_SUCCESS;
}

#endif
