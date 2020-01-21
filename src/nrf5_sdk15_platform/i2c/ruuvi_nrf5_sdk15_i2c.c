/**
 * Copyright (c) 2015 - 2017, Nordic Semiconductor ASA
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
#if RUUVI_NRF5_SDK15_I2C_ENABLED
#include <stdint.h>
#include <string.h> //memcpy

#include "ruuvi_boards.h"
#include "nrf_drv_twi.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_nrf5_sdk15_gpio.h"
#include "ruuvi_nrf5_sdk15_error.h"



static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE (I2C_INSTANCE);
static bool m_i2c_is_init        = false;
static volatile bool m_tx_in_progress     = false;
static volatile ret_code_t xfer_status    = NRF_SUCCESS;
static uint16_t timeout_us_per_byte       = 400;

static nrf_drv_twi_frequency_t ruuvi_to_nrf_frequency (const
        ruuvi_interface_i2c_frequency_t freq)
{
    switch (freq)
    {
        case RUUVI_INTERFACE_I2C_FREQUENCY_100k:
            return NRF_DRV_TWI_FREQ_100K;

        case RUUVI_INTERFACE_I2C_FREQUENCY_250k:
            return NRF_DRV_TWI_FREQ_250K;

        case RUUVI_INTERFACE_I2C_FREQUENCY_400k:
        default:
            return NRF_DRV_TWI_FREQ_400K;
    }
}

static void byte_timeout_set (const
                              ruuvi_interface_i2c_frequency_t freq)
{
    switch (freq)
    {
        case RUUVI_INTERFACE_I2C_FREQUENCY_100k:
            timeout_us_per_byte = 1000;

        case RUUVI_INTERFACE_I2C_FREQUENCY_250k:
            timeout_us_per_byte = 400;

        case RUUVI_INTERFACE_I2C_FREQUENCY_400k:
        default:
            timeout_us_per_byte = 200;
    }
}

static void on_complete (nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    m_tx_in_progress = false;

    if (p_event->type == NRF_DRV_TWI_EVT_ADDRESS_NACK) { xfer_status |= NRF_ERROR_NOT_FOUND; }
    else if (p_event->type == NRF_DRV_TWI_EVT_DATA_NACK) { xfer_status |= NRF_ERROR_DRV_TWI_ERR_DNACK; }
    else if (p_event->type != NRF_DRV_TWI_EVT_DONE) { xfer_status |= NRF_ERROR_INTERNAL; }
}

ruuvi_driver_status_t ruuvi_interface_i2c_init (const ruuvi_interface_i2c_init_config_t *
        config)
{
    ret_code_t err_code;
    nrf_drv_twi_frequency_t frequency = ruuvi_to_nrf_frequency (config->frequency);
    byte_timeout_set (config->frequency);
    const nrf_drv_twi_config_t twi_config =
    {
        .scl                = ruuvi_to_nrf_pin_map (config->scl),
        .sda                = ruuvi_to_nrf_pin_map (config->sda),
        .frequency          = frequency,
        .interrupt_priority = I2C_IRQ_PRIORITY,
        .clear_bus_init     = true
    };
    // Verify that lines can be pulled up
    ruuvi_interface_gpio_configure (config->scl, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP);
    ruuvi_interface_gpio_configure (config->sda, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP);
    ruuvi_interface_gpio_state_t state_scl, state_sda;
    ruuvi_interface_delay_us (10);
    ruuvi_interface_gpio_read (config->sda, &state_sda);
    ruuvi_interface_gpio_read (config->scl, &state_scl);

    if (RUUVI_INTERFACE_GPIO_HIGH != state_sda ||
            RUUVI_INTERFACE_GPIO_HIGH != state_scl)
    {
        return RUUVI_DRIVER_ERROR_INTERNAL;
    }

    err_code = nrf_drv_twi_init (&m_twi, &twi_config, on_complete, NULL);
    nrf_drv_twi_enable (&m_twi);
    m_i2c_is_init = true;
    m_tx_in_progress = false;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

bool ruuvi_interface_i2c_is_init()
{
    return m_i2c_is_init;
}

/**
 * @brief uninitialize I2C driver with default settings
 * @return RUUVI_DRIVER_SUCCESS on success, ruuvi error code on error
 */
ruuvi_driver_status_t i2c_uninit (void)
{
    return RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED;
}


/**
 * @breif I2C Write function
 *
 * @param[in] address I2C address of slave, without R/W bit
 * @param[in] tx_len length of data to be sent
 * @param[in] data pointer to data to be sent.
 * @param[in] stop Should a stop condition be clocked out. False to keep communicating
 *
 **/
ruuvi_driver_status_t ruuvi_interface_i2c_write_blocking (const uint8_t address,
        uint8_t * const p_tx, const size_t tx_len, const bool stop)
{
    if (!m_i2c_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

    if (NULL == p_tx) { return RUUVI_DRIVER_ERROR_NULL; }

    if (m_tx_in_progress) { return RUUVI_DRIVER_ERROR_BUSY; }

    int32_t err_code = NRF_SUCCESS;
    m_tx_in_progress = true;
    err_code |= nrf_drv_twi_tx (&m_twi, address, p_tx, tx_len, !stop);
    volatile uint32_t timeout = 0;

    while (m_tx_in_progress && timeout < (timeout_us_per_byte * tx_len))
    {
        timeout++;
        ruuvi_interface_delay_us (1);
    }

    if (timeout >= (timeout_us_per_byte * tx_len)) { err_code |= NRF_ERROR_TIMEOUT; }

    err_code |= xfer_status;
    xfer_status = NRF_SUCCESS;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

/**
 * I2C Read function
 *
 * parameter address: I2C address of slave, without or without R/W bit set
 * parameter rx_len: length of data to be received
 * parameter data: pointer to data to be received.
 *
 **/
ruuvi_driver_status_t ruuvi_interface_i2c_read_blocking (const uint8_t address,
        uint8_t * const p_rx, const size_t rx_len)
{
    if (!m_i2c_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

    if (NULL == p_rx) { return RUUVI_DRIVER_ERROR_NULL; }

    int32_t err_code = NRF_SUCCESS;
    m_tx_in_progress = true;
    err_code |= nrf_drv_twi_rx (&m_twi, address, p_rx, rx_len);
    volatile uint32_t timeout = 0;

    while (m_tx_in_progress && timeout < (timeout_us_per_byte * rx_len))
    {
        timeout++;
        ruuvi_interface_delay_us (1);
    }

    if (timeout >= (timeout_us_per_byte * rx_len)) { err_code |= NRF_ERROR_TIMEOUT; }

    err_code |= xfer_status;
    xfer_status = NRF_SUCCESS;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

#endif