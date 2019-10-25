/**
 *  Functions for printing out log via preferred method, i.e. RTT, UART or BLE.
 *
 *  Logging should implement well-defined overflow behaviour, i.e. block or drop new / drop old
 *  if log buffer gets filled.
 *
 *  Logging requires init and platform_log_level -functions.
 *  log must support levels ERROR, INFO, DEBUG.
 *
 *  Any prefixes, linenumbers etc are implemented at interface level and backend should print out raw data.
 *
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_LOG_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_yield.h"
#include <stdarg.h>

#define NRF_LOG_MODULE_NAME ruuvi_log
#define NRF_LOG_LEVEL 3
#include "nrf_log.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_ctrl.h"
NRF_LOG_MODULE_REGISTER();

static ruuvi_interface_log_severity_t log_level;
ruuvi_driver_status_t ruuvi_interface_log_init(const ruuvi_interface_log_severity_t
    min_severity)
{
  log_level = min_severity;
  NRF_LOG_INIT(NULL);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_log_flush(void)
{
  NRF_LOG_FLUSH();
  // Use microsecond delay to avoid getting stuck in timer loop if
  // logs are flushed in interrupt context.
  ruuvi_interface_delay_us(5000);
  return RUUVI_DRIVER_SUCCESS;
}

void ruuvi_interface_log(const ruuvi_interface_log_severity_t severity,
                         const char* const message)
{
  if(NULL == message)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_NULL, RUUVI_DRIVER_ERROR_NULL);
    return;
  }

  if(log_level >= severity)
  {
    NRF_LOG_INTERNAL_RAW_INFO("%s", message);
  }
}
#endif