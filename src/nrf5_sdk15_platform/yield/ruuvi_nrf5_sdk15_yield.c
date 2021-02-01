/**
 * @file ruuvi_nrf5_sdk15_yield.c
 * @author Otso Jousimaa
 * @date 2020-01-21
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause
 * @brief Implementation for yield and delay
 *
 * Implementation for yielding execution or delaying for a given time.
 * Yield enters a low-power system on state, delay blocks and keeps CPU active.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_yield.h"
#if RUUVI_NRF5_SDK15_YIELD_ENABLED
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_driver_error.h"
#include "nrf_delay.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_error.h"
#if RUUVI_NRF5_SDK15_TIMER_ENABLED
#include "ruuvi_interface_timer.h"
static ri_timer_id_t wakeup_timer;     //!< timer ID for wakeup
#endif

static bool m_lp = false;              //!< low-power mode enabled flag
static bool m_is_init = false;         //!< Module initialized flag
static volatile bool m_wakeup = false; //!< Wakeup flag, also concurrency barrier.
static ri_yield_state_ind_fp_t m_ind;  //!< State indication function

#ifdef FLOAT_ABI_HARD
#define IOC_MASK (0x01U) //!< Invalid operation
#define DZC_MASK (0x02U) //!< Divide by zero
#define OFC_MASK (0x04U) //!< Overflow
// Function handles and clears exception flags in FPSCR register and at the stack.
// During interrupt, handler execution FPU registers might be copied to the stack
// (see lazy stacking option) and it is necessary to clear data at the stack
// which will be recovered in the return from interrupt handling.
void FPU_IRQHandler (void)
{
    // Prepare pointer to stack address with pushed FPSCR register
    // (0x40 is FPSCR register offset in stacked data)
    uint32_t * fpscr = (uint32_t *) (FPU->FPCAR + 0x40);
    // Execute FPU instruction to activate lazy stacking
    (void) __get_FPSCR();

    // Check exception flags
    // Critical FPU exceptions signaled:
    // - IOC - Invalid Operation cumulative exception bit.
    // - DZC - Division by Zero cumulative exception bit.
    // - OFC - Overflow cumulative exception bit.
    if (*fpscr & IOC_MASK)
    {
        ri_log (RI_LOG_LEVEL_WARNING, "FPU IOC Error");
    }

    if (*fpscr & DZC_MASK)
    {
        ri_log (RI_LOG_LEVEL_WARNING, "FPU DZC Error");
    }

    if (*fpscr & OFC_MASK)
    {
        ri_log (RI_LOG_LEVEL_WARNING, "FPU OFC Error");
    }

    // Clear flags in stacked FPSCR register. To clear IDC, IXC, UFC, OFC, DZC and IOC flags, use 0x0000009F mask.
    *fpscr = *fpscr & ~ (0x0000009F);
}

static void fpu_init (void)
{
    NVIC_SetPriority (FPU_IRQn, 7);
    NVIC_ClearPendingIRQ (FPU_IRQn);
    NVIC_EnableIRQ (FPU_IRQn);
}

#else //!< FLOAT_ABI_HARD
static void fpu_init (void)
{}
#endif

/*
 * Set a flag to wake up
 */
static void wakeup_handler (void * p_context)
{
    m_wakeup = true;
}

bool ri_yield_is_interrupt_context (void)
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}


rd_status_t ri_yield_init (void)
{
    fpu_init();
    ret_code_t err_code = nrf_pwr_mgmt_init();
    m_lp = false;
    m_wakeup = false;
    m_ind = NULL;
    m_is_init = true;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

#if RUUVI_NRF5_SDK15_TIMER_ENABLED
rd_status_t ri_yield_low_power_enable (const bool enable)
{
    // Timer can be allocated after timer has initialized
    rd_status_t timer_status = RD_SUCCESS;

    if (NULL == wakeup_timer)
    {
        timer_status = ri_timer_create (&wakeup_timer,
                                        RI_TIMER_MODE_SINGLE_SHOT, wakeup_handler);
    }

    if (timer_status == RD_SUCCESS)
    {
        m_lp = enable;
        m_wakeup = true;
    }
    else
    {
        m_lp = false;
    }

    return timer_status;
}
#else
// Return error if timers are not enabled.
rd_status_t ri_yield_low_power_enable (const bool enable)
{
    return RD_ERROR_NOT_SUPPORTED;
}
#endif


rd_status_t ri_yield (void)
{
    if (NULL != m_ind) { m_ind (false); }

    nrf_pwr_mgmt_run();

    if (NULL != m_ind) { m_ind (true); }

    return RD_SUCCESS;
}

rd_status_t ri_delay_ms (uint32_t time)
{
    rd_status_t err_code = RD_SUCCESS;
#if RUUVI_NRF5_SDK15_TIMER_ENABLED

    // Check that low-power delay is enabled and sleep timer is not running right now.
    if (m_lp && m_wakeup)
    {
        if (ri_yield_is_interrupt_context())
        {
            ri_delay_us (1000 * time);
        }
        else
        {
            m_wakeup = false;
            err_code |= ri_timer_start (wakeup_timer, time, NULL);

            while (RD_SUCCESS == err_code && !m_wakeup)
            {
                err_code |= ri_yield();
            }
        }
    }

#else

    if (0) {}

#endif
    else
    {
        nrf_delay_ms (time);
    }

    return err_code;
}

rd_status_t ri_delay_us (uint32_t time)
{
    nrf_delay_us (time);
    return RD_SUCCESS;
}

void ri_yield_indication_set (const ri_yield_state_ind_fp_t indication)
{
    m_ind = indication;
}

rd_status_t ri_yield_uninit (void)
{
    m_ind = NULL;
    wakeup_timer = NULL;
    m_wakeup = false;
    m_lp = false;
    m_is_init = false;
    return RD_SUCCESS;
}

#endif