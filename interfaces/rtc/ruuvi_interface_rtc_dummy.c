// These dummy functions return a running counter on platforms where RTC is not available
#include "ruuvi_driver_enabled_modules.h"
#if !(APPLICATION_RTC_MCU_ENABLED)
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_rtc.h"

static uint64_t m_dummy;
/**
 * @brief Initializes RTC at 0 ms.
 *
 * @return RUUVI_SUCCESS if no error occured, error code otherwise.
 **/
ruuvi_driver_status_t ruuvi_interface_rtc_init(void)
{
  m_dummy = 0;
  // Return NOT_SUPPORTED to let application know that we're not having an actual RTC
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

/**
  * @brief Stop RTC if applicable.
  *
  * @return RUUVI_SUCCESS if no error occured, error code otherwise.
  **/
ruuvi_driver_status_t ruuvi_interface_rtc_uninit(void)
{
  m_dummy = 0;
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