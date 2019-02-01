#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_POWER_ENABLED 
/**
 * @file ruuvi_nrf5_sdk15_power.c
 * @author Otso Jousimaa
 * @date 2019-01-31
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause
 * @brief Power handling functions, such as enabling internal regulators.
 *
 */
#include <stdint.h>
#include "nrfx_power.h"
#include "nvic.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_power.h"
#include "sdk_errors.h"


static bool m_is_init = false;

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
  if(m_is_init)
  {
    nrfx_power_uninit();
    m_is_init = false;
  }
  err_code |= nrfx_power_init (&config);
  m_is_init = true;
  return ruuvi_platform_to_ruuvi_error(&err_code);
}

void ruuvi_interface_power_reset(void)
{
  NVIC_SystemReset();
}

#endif