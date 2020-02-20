/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_watchdog.h"
#if RUUVI_NRF5_SDK15_WATCHDOG_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_power.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_wdt.h"
#include <stdbool.h>

nrf_drv_wdt_channel_id m_channel_id;
wdt_evt_handler_t      m_on_trigger;

/**
 * @brief WDT events handler.
 */
void wdt_event_handler (void)
{
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
    if (NULL != m_on_trigger)
    {
        m_on_trigger();
    }

    ri_log (RI_LOG_LEVEL_ERROR, "WDT Triggered, reset\r\n");

    while (1);
}

/**
 * @param Initializes watchdog module.
 * After initialization watchdog must be fed at given interval or the program will reset.
 * There is no way to uninitialize the watchdog.
 * Consider bootloader watchdog interval on setup.
 *
 * @param[in] interval How often the watchdog should be fed.
 * @param[in] handler Function called on watchdog trigger. Must complete within 2 1/2^15 ms cycles.
 *
 * @retval RD_SUCCESS on success.
 * @retval RI_ERROR_INVALID_STATE if Watchdog is already initialized.
 */
rd_status_t ri_watchdog_init (const uint32_t interval_ms, const wdt_evt_handler_t handler)
{
    uint32_t err_code = NRF_SUCCESS;
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    config.reload_value = interval_ms;
    err_code = nrf_drv_wdt_init (&config, wdt_event_handler);

    if (NRF_SUCCESS == err_code)
    {
        err_code = nrf_drv_wdt_channel_alloc (&m_channel_id);

        if (NRF_SUCCESS == err_code)
        {
            nrf_drv_wdt_enable();

            // Halting the LFCLK while WDT is running hangs the program.
            // Request LFCLK to make sure it's never stopped after this point.
            // https://devzone.nordicsemi.com/f/nordic-q-a/45584/softdevice-hangs-forever-during-disable-while-stopping-lfclk
            // Initialize clock if not already initialized
            if (false == nrf_drv_clock_init_check())
            {
                err_code |= nrf_drv_clock_init();
            }

            nrf_drv_clock_lfclk_request (NULL);
        }
    }

    m_on_trigger = handler;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

/**
 * "Feed" the watchdog, resets the watchdog timer.
 * This must be called after watchdog initialization or the program will reset.
 */
rd_status_t ri_watchdog_feed (void)
{
    nrf_drv_wdt_channel_feed (m_channel_id);
    return RD_SUCCESS;
}

#endif