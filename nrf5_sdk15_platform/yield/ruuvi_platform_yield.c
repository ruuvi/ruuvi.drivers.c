/**
 * Yield and delay function implementations on Nordic SDK15.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_platform_external_includes.h"
#ifdef NRF5_SDK15_YIELD_ENABLED
#include "ruuvi_interface_yield.h"
#include "ruuvi_driver_error.h"
#include "nrf_delay.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_error.h"

ruuvi_driver_status_t ruuvi_platform_yield_init(void)
{
  ret_code_t err_code = nrf_pwr_mgmt_init();
  return ruuvi_platform_to_ruuvi_error(&err_code);
}

ruuvi_driver_status_t ruuvi_platform_yield(void)
{
  nrf_pwr_mgmt_run();
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_platform_delay_ms(uint32_t time)
{
  nrf_delay_ms(time);
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_platform_delay_us(uint32_t time)
{
  nrf_delay_us(time);
  return RUUVI_DRIVER_SUCCESS;
}

#endif