// These dummy functions return a running counter on platforms where RTC is not available
#include "ruuvi_driver_enabled_modules.h"
#if !(APPLICATION_RTC_MCU_ENABLED)
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_timer.h"

/** @brief Interval at which RTC should fire. Faster consumes more power and gives better resolution. */
#define DUMMY_RTC_INTERVAL 1024
static ruuvi_interface_timer_id_t counter_timer;    //!< timer ID for counter
static volatile uint64_t m_dummy;                   //!< mark volatile in case someone busyloops with RTC
static void counter_handler(void* p_context)
{
  uint64_t mask = ~((uint64_t)DUMMY_RTC_INTERVAL);
  m_dummy &= mask;
  m_dummy += DUMMY_RTC_INTERVAL;
}

/**
 * @brief Initializes RTC at 0 ms.
 *
 * @return RUUVI_SUCCESS if no error occured, error code otherwise.
 **/
ruuvi_driver_status_t ruuvi_interface_rtc_init(void)
{
  m_dummy = 0;
  // Use timer interrupts at 1024 ms to increment RTC.
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  if(!ruuvi_interface_timer_is_init())
  {
    err_code = ruuvi_interface_timer_init();
  }
  if(NULL == counter_timer)
  {
    err_code |= ruuvi_interface_timer_create(&counter_timer, 
                                             RUUVI_INTERFACE_TIMER_MODE_REPEATED, 
                                             counter_handler);
  }
  err_code |= ruuvi_interface_timer_start(counter_timer, DUMMY_RTC_INTERVAL);
  return (RUUVI_DRIVER_SUCCESS == err_code) ? RUUVI_DRIVER_SUCCESS : RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

/**
  * @brief Stop RTC if applicable.
  *
  * @return RUUVI_SUCCESS if no error occured, error code otherwise.
  **/
ruuvi_driver_status_t ruuvi_interface_rtc_uninit(void)
{
  m_dummy = 0;
  if(NULL != counter_timer)  { ruuvi_interface_timer_stop(counter_timer); }
  return RUUVI_DRIVER_SUCCESS;
}

/**
 * @brief Get milliseconds since init. 
 *
 * @return number of milliseconds since RTC init.
 * @return @c RUUVI_DRIVER_UINT64_INVALID if RTC is not running
  **/
uint64_t ruuvi_interface_rtc_millis(void)
{
  return m_dummy++;
}

#endif