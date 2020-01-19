#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_TIMER_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_timer.h"

#include "nrf_error.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "app_timer.h"

#include <stdbool.h>

#if APPLICATION_TIMER_MAX_INSTANCES > 10
  #error "Allocating over 10 timers is not supported"
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 9
  APP_TIMER_DEF(timer_9);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 8
  APP_TIMER_DEF(timer_8);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 7
  APP_TIMER_DEF(timer_7);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 6
  APP_TIMER_DEF(timer_6);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 5
  APP_TIMER_DEF(timer_5);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 4
  APP_TIMER_DEF(timer_4);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 3
  APP_TIMER_DEF(timer_3);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 2
  APP_TIMER_DEF(timer_2);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 1
  APP_TIMER_DEF(timer_1);
#endif
#if APPLICATION_TIMER_MAX_INSTANCES > 0
  APP_TIMER_DEF(timer_0);
#endif
#if 0 >= APPLICATION_TIMER_MAX_INSTANCES
  #error "No instances enabled for application timer"
#endif

static uint8_t timer_idx = 0;  ///< Counter to next timer to allocate.
static bool m_is_init = false; ///< Flag keeping track on if module is initialized.

/**
 * @brief return free timer ID
 */
static app_timer_id_t get_timer_id(void)
{
  switch(timer_idx++)
  {
      #if APPLICATION_TIMER_MAX_INSTANCES > 0

    case 0:
      return timer_0;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 1

    case 1:
      return timer_1;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 2

    case 2:
      return timer_2;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 3

    case 3:
      return timer_3;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 4

    case 4:
      return timer_4;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 5

    case 5:
      return timer_5;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 6

    case 6:
      return timer_6;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 7

    case 7:
      return timer_7;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 8

    case 8:
      return timer_8;
      #endif
      #if APPLICATION_TIMER_MAX_INSTANCES > 9

    case 9:
      return timer_9;
      #endif

    default:
      return NULL;
  }
}

ruuvi_driver_status_t ruuvi_interface_timer_init(void)
{
  if(m_is_init) { return RUUVI_DRIVER_SUCCESS; }

  ret_code_t err_code = NRF_SUCCESS;

  // Initialize clock if not already initialized
  if(false == nrf_drv_clock_init_check()) { err_code |= nrf_drv_clock_init(); }

  nrf_drv_clock_lfclk_request(NULL);
  err_code |= app_timer_init();

  if(NRF_SUCCESS == err_code) { m_is_init = true; }

  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

//return true if timers have been successfully initialized.
bool ruuvi_interface_timer_is_init(void)
{
  return m_is_init;
}

ruuvi_driver_status_t ruuvi_interface_timer_create(ruuvi_interface_timer_id_t*
    p_timer_id, const ruuvi_interface_timer_mode_t mode,
    const ruuvi_timer_timeout_handler_t timeout_handler)
{
  app_timer_mode_t nrf_mode = APP_TIMER_MODE_SINGLE_SHOT;

  if(RUUVI_INTERFACE_TIMER_MODE_REPEATED == mode) { nrf_mode = APP_TIMER_MODE_REPEATED; }

  app_timer_id_t tid = get_timer_id();
  ret_code_t err_code = app_timer_create(&tid,
                                         nrf_mode,
                                         (app_timer_timeout_handler_t)timeout_handler);

  if(NRF_SUCCESS == err_code) {*p_timer_id = (void*)tid;}

  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

ruuvi_driver_status_t ruuvi_interface_timer_start(const ruuvi_interface_timer_id_t
    timer_id, const uint32_t ms)
{
  // Counters are 24 bits
  // nrf5 sdk_config.h has prescaler setting for timer, resolution can be traded for run time
  if(APP_TIMER_TICKS(ms) >= (1 << 24))
  {
    ruuvi_interface_log(RUUVI_INTERFACE_LOG_ERROR, "Timer overflow, timer not started\r\n");
    return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  ret_code_t err_code = app_timer_start((app_timer_id_t)timer_id, APP_TIMER_TICKS(ms),
                                        NULL);
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

ruuvi_driver_status_t ruuvi_interface_timer_stop(ruuvi_interface_timer_id_t timer_id)
{
  ret_code_t err_code = app_timer_stop((app_timer_id_t)timer_id);
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

#endif