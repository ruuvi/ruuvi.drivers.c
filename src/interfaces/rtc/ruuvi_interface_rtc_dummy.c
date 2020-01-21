// These dummy functions return a running counter on platforms where RTC is not available
#include "ruuvi_driver_enabled_modules.h"
#if !(APPLICATION_RTC_MCU_ENABLED)
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_timer.h"
#include <stdlib.h>

/** @brief Interval at which RTC should fire. Faster consumes more power and gives better resolution. */
#define DUMMY_RTC_INTERVAL 1024
static ri_timer_id_t counter_timer;    //!< timer ID for counter
static volatile uint64_t
m_dummy;                   //!< mark volatile in case someone busyloops with RTC
static void counter_handler (void * p_context)
{
    uint64_t mask = ~ ( (uint64_t) DUMMY_RTC_INTERVAL - 1);
    m_dummy &= mask;
    m_dummy += DUMMY_RTC_INTERVAL;
}

/**
 * @brief Initializes RTC at 0 ms.
 *
 * @return RUUVI_SUCCESS if no error occured, error code otherwise.
 **/
rd_status_t ri_rtc_init (void)
{
    m_dummy = 0;
    // Use timer interrupts at 1024 ms to increment RTC.
    rd_status_t err_code = RD_SUCCESS;

    if (!ri_timer_is_init())
    {
        err_code = ri_timer_init();
    }

    if (NULL == counter_timer)
    {
        err_code |= ri_timer_create (&counter_timer,
                                     RI_TIMER_MODE_REPEATED,
                                     counter_handler);
    }

    err_code |= ri_timer_start (counter_timer, DUMMY_RTC_INTERVAL);
    return (RD_SUCCESS == err_code) ? RD_SUCCESS :
           RD_ERROR_NOT_SUPPORTED;
}

/**
  * @brief Stop RTC if applicable.
  *
  * @return RUUVI_SUCCESS if no error occured, error code otherwise.
  **/
rd_status_t ri_rtc_uninit (void)
{
    m_dummy = 0;

    if (NULL != counter_timer)  { ri_timer_stop (counter_timer); }

    return RD_SUCCESS;
}

/**
 * @brief Get milliseconds since init.
 *
 * @return number of milliseconds since RTC init.
 * @return @c RUUVI_DRIVER_UINT64_INVALID if RTC is not running
  **/
uint64_t ri_rtc_millis (void)
{
    return m_dummy++;
}

#endif