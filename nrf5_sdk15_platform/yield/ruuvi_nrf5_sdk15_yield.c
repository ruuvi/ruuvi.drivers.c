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
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_YIELD_ENABLED
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_driver_error.h"
#include "nrf_delay.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_error.h"

// Function handles and clears exception flags in FPSCR register and at the stack.
// During interrupt, handler execution FPU registers might be copied to the stack
// (see lazy stacking option) and it is necessary to clear data at the stack
// which will be recovered in the return from interrupt handling.
void FPU_IRQHandler(void)
{
  // Prepare pointer to stack address with pushed FPSCR register (0x40 is FPSCR register offset in stacked data)
  uint32_t* fpscr = (uint32_t*)(FPU->FPCAR + 0x40);
  // Execute FPU instruction to activate lazy stacking
  (void)__get_FPSCR();

  // Check exception flags
  // Critical FPU exceptions signaled:
  // - IOC - Invalid Operation cumulative exception bit.
  // - DZC - Division by Zero cumulative exception bit.
  // - OFC - Overflow cumulative exception bit.
  if(*fpscr & 0x07)
  {
    ruuvi_interface_log(RUUVI_INTERFACE_LOG_WARNING, "FPU Error \r\n");
  }

  // Clear flags in stacked FPSCR register. To clear IDC, IXC, UFC, OFC, DZC and IOC flags, use 0x0000009F mask.
  *fpscr = *fpscr & ~(0x0000009F);
}

/**
 *
 */
ruuvi_driver_status_t ruuvi_interface_yield_init(void)
{
  NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_LOW);
  NVIC_ClearPendingIRQ(FPU_IRQn);
  NVIC_EnableIRQ(FPU_IRQn);
  ret_code_t err_code = nrf_pwr_mgmt_init();
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

ruuvi_driver_status_t ruuvi_interface_yield(void)
{
  nrf_pwr_mgmt_run();
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_delay_ms(uint32_t time)
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