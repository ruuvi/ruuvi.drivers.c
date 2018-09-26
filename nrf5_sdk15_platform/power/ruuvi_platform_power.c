/**
 *  Power handling functions, such as enabling internal regulators.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/
#include "ruuvi_platform_external_includes.h"
#if NRF5_SDK15_POWER_ENABLED
#include <stdint.h>
#include "nrfx_power.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_power.h"
#include "sdk_errors.h"

ruuvi_driver_status_t ruuvi_interface_power_regulators_enable(const ruuvi_interface_power_regulators_t regulators)
{

  ret_code_t err_code = NRF_SUCCESS;
  nrfx_power_config_t config = {0};
  if(RUUVI_INTERFACE_POWER_REGULATORS_DCDC_INTERNAL & regulators)
  {
    config.dcdcen = true;
  }
  if(RUUVI_INTERFACE_POWER_REGULATORS_DCDC_HV & regulators)
  {
    #if NRF_POWER_HAS_VDDH
    config.dcdcenhv = true;
    #endif
  }
  nrfx_power_uninit();
  err_code |= nrfx_power_init (&config);
  return ruuvi_platform_to_ruuvi_error(&err_code);
}

#endif