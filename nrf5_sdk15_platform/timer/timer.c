#include "sdk_application_config.h"
#if NRF5_SDK15_TIMER

#include "timer.h"
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

// Function for creating a timer instance
ruuvi_status_t platform_timer_create (platform_timer_id_t const *p_timer_id, ruuvi_timer_mode_t mode, ruuvi_timer_timeout_handler_t timeout_handler)
{
  app_timer_mode_t nrf_mode = APP_TIMER_MODE_SINGLE_SHOT;
  if (RUUVI_TIMER_MODE_REPEATED == mode) { nrf_mode = APP_TIMER_MODE_REPEATED; }

  ret_code_t err_code = app_timer_create (p_timer_id,
                        nrf_mode,
                        (app_timer_timeout_handler_t)timeout_handler);
  return platform_to_ruuvi_error(&err_code);
}

//   Function for starting a timer.
ruuvi_status_t platform_timer_start (platform_timer_id_t timer_id, uint32_t ms, void *p_context)
{
  ret_code_t err_code = app_timer_start(timer_id, APP_TIMER_TICKS(ms), p_context);
  return platform_to_ruuvi_error(&err_code);

}

//   Function for stopping the specified timer.
ruuvi_status_t platform_timer_stop (platform_timer_id_t timer_id)
{
  ret_code_t err_code = app_timer_stop(timer_id);
  return platform_to_ruuvi_error(&err_code);
}

#endif