#include "sdk_application_config.h"
#if NRF5_SDK15_TIMER

#include "ruuvi_error.h"
#include <stdbool.h>

#include "nrf_error.h"
#include "sdk_errors.h"
#include "app_timer.h"

static bool m_is_init = false;

// Calls whatever initialization is required by application timers
ruuvi_status_t platform_timers_init(void)
{
  if (m_is_init) { return RUUVI_SUCCESS; }
  ret_code_t err_code = NRF_SUCCESS;

  err_code |= app_timer_init();
  if (NRF_SUCCESS == err_code) { m_is_init = true; }
  return platform_to_ruuvi_error(&err_code);
}

//return true if timers have been successfully initialized.
bool platform_timers_is_init(void)
{
  return m_is_init;
}

#endif