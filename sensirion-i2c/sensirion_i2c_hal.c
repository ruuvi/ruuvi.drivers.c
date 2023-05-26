/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include "nrfx_twi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "sensirion_i2c_hal.h"
#include "sensirion_common.h"
#include "sensirion_config.h"

#define TWI_INSTANCE_ID     1


/* TWIM instance. */
static const nrfx_twi_t m_twi = NRFX_TWI_INSTANCE(TWI_INSTANCE_ID);


void twim_handler(nrfx_twi_evt_t const * p_event, void * p_context)
{
    // Handler for TWI events.
    switch (p_event->type)
    {
      case NRFX_TWI_EVT_DONE:
        break;
      case NRFX_TWI_EVT_ADDRESS_NACK:
        break;
      case NRFX_TWI_EVT_DATA_NACK:
        break;
    }
}

void twi_init (void)
{
    ret_code_t err_code;

    const nrfx_twi_config_t twi_config = {
       .scl                = RB_I2C_SCL_PIN,
       .sda                = RB_I2C_SDA_PIN,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .hold_bus_uninit     = false
    };

    err_code = nrfx_twi_init(&m_twi, &twi_config, NULL /*&twim_handler*/, NULL);
    APP_ERROR_CHECK(err_code);

    nrfx_twi_enable(&m_twi);
}

/*
 * INSTRUCTIONS
 * ============
 *
 * Implement all functions where they are marked as IMPLEMENT.
 * Follow the function specification in the comments.
 */

/**
 * Select the current i2c bus by index.
 * All following i2c operations will be directed at that bus.
 *
 * THE IMPLEMENTATION IS OPTIONAL ON SINGLE-BUS SETUPS (all sensors on the same
 * bus)
 *
 * @param bus_idx   Bus index to select
 * @returns         0 on success, an error code otherwise
 */
int16_t sensirion_i2c_hal_select_bus(uint8_t bus_idx) {
    return 0;
}

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void sensirion_i2c_hal_init(void) {
#if 0
  twi_init();
  nrf_gpio_cfg_sense_input(RB_I2C_SCL_PIN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_input(RB_I2C_SDA_PIN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_NOSENSE);
#endif
}

/**
 * Release all resources initialized by sensirion_i2c_hal_init().
 */
void sensirion_i2c_hal_free(void) {
    /* TODO:IMPLEMENT or leave empty if no resources need to be freed */
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count) {
#if 1
    nrfx_twi_xfer_desc_t xfer_desc;
    xfer_desc.address = address;
    xfer_desc.type = NRFX_TWI_XFER_RX;
    xfer_desc.primary_length = count;
    xfer_desc.p_primary_buf = data;

    ret_code_t err_code = nrfx_twi_xfer(&m_twi, &xfer_desc, 0);
    if (NRF_SUCCESS != err_code) {
      return err_code;
    }
    while (nrfx_twi_is_busy(&m_twi)) {
    }
#else
    nrfx_err_t err_code = nrfx_twi_rx(&m_twi, address, data, count);
    if (NRF_SUCCESS != err_code) {
        return err_code;
    }
#endif

    return NO_ERROR;
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* data,
                               uint16_t count) {
#if 1
    nrfx_twi_xfer_desc_t xfer_desc;
    xfer_desc.address = address;
    xfer_desc.type = NRFX_TWI_XFER_TX;
    xfer_desc.primary_length = count;
    xfer_desc.p_primary_buf = data;

    ret_code_t err_code = nrfx_twi_xfer(&m_twi, &xfer_desc, 0);
    if (NRF_SUCCESS != err_code) {
      return err_code;
    }
    while (nrfx_twi_is_busy(&m_twi)) {
    }
#else
    nrfx_err_t err_code = nrfx_twi_tx(&m_twi, address, data, count, false);
    if (NRF_SUCCESS != err_code) {
        return err_code;
    }
#endif

    return NO_ERROR;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * Despite the unit, a <10 millisecond precision is sufficient.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_i2c_hal_sleep_usec(uint32_t useconds) {
    nrf_delay_us(useconds);
}
