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
#include "ruuvi_platform_external_includes.h"
#if NRF5_SDK15_SPI_ENABLED
#include <stdint.h>
#include <string.h> //memcpy

#include "ruuvi_boards.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_spi.h"
#include "ruuvi_interface_yield.h"


int8_t ruuvi_platform_spi_bme280_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= ruuvi_platform_gpio_write(dev_id, RUUVI_INTERFACE_GPIO_LOW);
  err_code |= ruuvi_platform_spi_xfer_blocking(&reg_addr, 1, NULL, 0);
  err_code |= ruuvi_platform_spi_xfer_blocking(reg_data, len, NULL, 0);
  err_code |= ruuvi_platform_gpio_write(dev_id, RUUVI_INTERFACE_GPIO_HIGH);
  return (RUUVI_DRIVER_SUCCESS == err_code) ? 0 : -1;
}

int8_t ruuvi_platform_spi_bme280_read (uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= ruuvi_platform_gpio_write(dev_id, RUUVI_INTERFACE_GPIO_LOW);
  err_code |= ruuvi_platform_spi_xfer_blocking(&reg_addr, 1, NULL, 0);
  err_code |= ruuvi_platform_spi_xfer_blocking(NULL, 0, reg_data, len);
  err_code |= ruuvi_platform_gpio_write(dev_id, RUUVI_INTERFACE_GPIO_HIGH);
  return (RUUVI_DRIVER_SUCCESS == err_code) ? 0 : -1;
}

#endif