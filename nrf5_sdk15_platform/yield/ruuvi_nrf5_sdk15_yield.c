/**
 * @file ruuvi_nrf5_sdk15_yield.c
 * @author Otso Jousimaa
 * @date 2019-01-30
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause
 * @brief Implementation for yield and delay
 *
 * Implementation for yielding execution or delaying for a given time.
 * Yield enters a low-power system on state, delay blocks and keeps CPU active.
 *
 */

#include "ruuvi_platform_external_includes.h"
#ifdef NRF5_SDK15_YIELD_ENABLED
#include "ruuvi_interface_yield.h"
#include "ruuvi_driver_error.h"
#include "nrf_delay.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_error.h"

ruuvi_driver_status_t ruuvi_interface_yield_init(void)
{
  ret_code_t err_code = nrf_pwr_mgmt_init();
  return ruuvi_platform_to_ruuvi_error(&err_code);
}

ruuvi_driver_status_t ruuvi_interface_yield(void)
{
  nrf_pwr_mgmt_run();
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interdace_delay_ms(uint32_t time)
{
  nrf_delay_ms(time);
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_delay_us(uint32_t time)
{
  nrf_delay_us(time);
  return RUUVI_DRIVER_SUCCESS;
}

#endif