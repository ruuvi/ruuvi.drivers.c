
#include "ruuvi_platform_nrf5_sdk15_config.h"
#ifdef NRF5_SDK15_YIELD_ENABLED
#include "ruuvi_interface_yield.h"
#include "ruuvi_driver_error.h"
#include "nrf_delay.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_error.h"

#ifndef NULL
#ifdef __cplusplus
#define NULL   0
#else
#define NULL   ((void *) 0)
#endif
#endif

/** Handles softdevice being present and initialized, works around FPU bug **/
static ruuvi_driver_status_t default_yield(void)
{
  nrf_pwr_mgmt_run();
  return RUUVI_DRIVER_SUCCESS;
}

static yield_fptr_t yield = default_yield;

ruuvi_driver_status_t ruuvi_platform_yield_init(void)
{
  ret_code_t err_code = nrf_pwr_mgmt_init();
  return platform_to_ruuvi_error(&err_code);
}

/** Call function which will release execution / go to sleep **/
ruuvi_driver_status_t ruuvi_platform_yield(void)
{
  if(NULL == yield) { return RUUVI_DRIVER_ERROR_NULL; }
  return yield();
}

/** Setup yield function, for example sd_app_evt_wait() with SD **/
ruuvi_driver_status_t ruuvi_platform_yield_set(yield_fptr_t yield_ptr)
{
  yield = yield_ptr;
  return RUUVI_DRIVER_SUCCESS;
}

/** delay given number of milliseconds **/
ruuvi_driver_status_t ruuvi_platform_delay_ms(uint32_t time)
{
  nrf_delay_ms(time);
  return RUUVI_DRIVER_SUCCESS;
}

/** delay given number of microseconds **/
ruuvi_driver_status_t ruuvi_platform_delay_us(uint32_t time)
{
  nrf_delay_us(time);
  return RUUVI_DRIVER_SUCCESS;
}

#endif