/**
 * Copyright (c) 2016 - 2018, Nordic Semiconductor ASA
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
/**
 * RTC sensor implementation on Nordic SDK15 / nRF52832. Based on Nordic Semiconductor RTC peripheral example.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_NRF52832_RTC_ENABLED

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_rtc.h"
#include "nrf.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include <stdint.h>
#include <stdbool.h>

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(NRF5_SDK15_RTC_INSTANCE); /**< RTC0 is reserved by the softdevice, use something else. */
static uint64_t ticks = 0;
static bool m_is_init = false;

/** @brief: Function for handling the RTC0 interrupts.
 * Triggered on overflow
 */
static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
  if (int_type == NRF_DRV_RTC_INT_OVERFLOW)
  {
    // nRF RTC is 24 bits wide.
    ticks += (1<<24);
  }
}

/**
 * Initializes RTC at 0 ms.
 *
 * Returns RUUVI_SUCCESS if no error occured, error code otherwise.
 **/
ruuvi_driver_status_t ruuvi_interface_rtc_init(void)
{
  if(true == m_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ret_code_t err_code = NRF_SUCCESS;

  // Initialize clock if not already initialized
  if(false == nrf_drv_clock_init_check()){ err_code |= nrf_drv_clock_init(); }
  // Request LFCLK for RTC
  nrf_drv_clock_lfclk_request(NULL);
  nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
  config.prescaler = 0;
  ticks = 0;
  err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);

  //Power on RTC instance before clearing the counter and enabling overflow
  nrf_drv_rtc_enable(&rtc);
  nrf_drv_rtc_counter_clear(&rtc);
  nrf_drv_rtc_overflow_enable(&rtc, true);
  if(NRF_SUCCESS == err_code) { m_is_init = true; }
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

/**
  * Stop RTC if applicable.
  *
  * Returns RUUVI_SUCCESS if no error occured, error code otherwise.
  **/
ruuvi_driver_status_t ruuvi_interface_rtc_uninit(void)
{
  if(true == m_is_init)
  {
    m_is_init = false;
    nrf_drv_rtc_uninit(&rtc);
    nrf_drv_clock_lfclk_release();
  }
  return RUUVI_DRIVER_SUCCESS;
}

/**
  * Return number of milliseconds since RTC init, RUUVI_DRIVER_UINT64_INVALID if RTC is not running
  **/
uint64_t ruuvi_interface_rtc_millis(void)
{
  if(false == m_is_init) { return RUUVI_DRIVER_UINT64_INVALID; }
  uint64_t ms = nrf_drv_rtc_counter_get(&rtc) + ticks;
  return (ms*1000)/32768;
}

#endif