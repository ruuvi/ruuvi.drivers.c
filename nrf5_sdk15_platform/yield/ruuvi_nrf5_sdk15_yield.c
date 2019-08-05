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
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_timer.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_driver_error.h"
#include "nrf_delay.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_error.h"

static bool m_lp = false;                          //!< low-power mode enabled flag
static volatile bool m_wakeup = false;             //!< wakeup flag
static ruuvi_interface_timer_id_t wakeup_timer;    //!< timer ID for wakeup
static ruuvi_interface_yield_state_ind_fp_t m_ind; //!< State indication function

#ifdef FLOAT_ABI_HARD
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
static void fpu_init(void)
{
  NVIC_SetPriority(FPU_IRQn, APP_IRQ_PRIORITY_LOW);
  NVIC_ClearPendingIRQ(FPU_IRQn);
  NVIC_EnableIRQ(FPU_IRQn);
}
#else
static void fpu_init(void)
{}
#endif

/*
 * Set a flag to wake up
 */
static void wakeup_handler(void* p_context)
{
  m_wakeup = true;
}

/**
 *
 */
ruuvi_driver_status_t ruuvi_interface_yield_init(void)
{
  fpu_init();
  ret_code_t err_code = nrf_pwr_mgmt_init();
  m_lp = false;
  m_wakeup = false;
  m_ind = NULL;
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

ruuvi_driver_status_t ruuvi_interface_yield_low_power_enable(const bool enable)
{
  // Timer can be allocated after timer has initialized
  ruuvi_driver_status_t timer_status = RUUVI_DRIVER_SUCCESS;
  if(NULL == wakeup_timer)
  {
    timer_status = ruuvi_interface_timer_create(&wakeup_timer, RUUVI_INTERFACE_TIMER_MODE_SINGLE_SHOT, wakeup_handler);
  }
  
  if(timer_status == RUUVI_DRIVER_SUCCESS)
  {
    m_lp = enable;
  }
  else 
  {
    m_lp = false;
  }
  return timer_status;
}



ruuvi_driver_status_t ruuvi_interface_yield(void)
{
  if(NULL != m_ind) { m_ind(false); }
  nrf_pwr_mgmt_run();
  if(NULL != m_ind) { m_ind(true); }
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_delay_ms(uint32_t time)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  if(m_lp)
  {
    m_wakeup = false;
    err_code |= ruuvi_interface_timer_start(wakeup_timer, time);
    while(RUUVI_DRIVER_SUCCESS == err_code && !m_wakeup )
    {
      err_code |= ruuvi_interface_yield();
    }
  } 
  else 
  {
    nrf_delay_ms(time);
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_delay_us(uint32_t time)
{
  nrf_delay_us(time);
  return RUUVI_DRIVER_SUCCESS;
}

void ruuvi_interface_yield_indication_set(const ruuvi_interface_yield_state_ind_fp_t indication)
{
  m_ind = indication;
}

#endif