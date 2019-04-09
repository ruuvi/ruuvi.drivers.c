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
#if RUUVI_NRF5_SDK15_SPI_ENABLED

#include <stdint.h>
#include <string.h> //memcpy

#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_spi.h"
#include "ruuvi_interface_yield.h"

#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"


static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(
                                   SPI_INSTANCE);  /**< SPI instance. */
static bool  spi_init_done = false;

static ruuvi_driver_status_t ruuvi_to_nrf_spi_mode(const ruuvi_interface_spi_mode_t
    ruuvi_mode, nrf_drv_spi_mode_t* nrf_mode)
{
  switch(ruuvi_mode)
  {
    case RUUVI_INTERFACE_SPI_MODE_0:
      *nrf_mode = NRF_DRV_SPI_MODE_0;
      return RUUVI_DRIVER_SUCCESS;

    case RUUVI_INTERFACE_SPI_MODE_1:
      *nrf_mode = NRF_DRV_SPI_MODE_1;
      return RUUVI_DRIVER_SUCCESS;

    case RUUVI_INTERFACE_SPI_MODE_2:
      *nrf_mode = NRF_DRV_SPI_MODE_2;
      return RUUVI_DRIVER_SUCCESS;

    case RUUVI_INTERFACE_SPI_MODE_3:
      *nrf_mode = NRF_DRV_SPI_MODE_3;
      return RUUVI_DRIVER_SUCCESS;

    default:
      return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }
}

static ruuvi_driver_status_t ruuvi_to_nrf_spi_freq(const ruuvi_interface_spi_mode_t
    ruuvi_freq, nrf_drv_spi_frequency_t* nrf_freq)
{
  switch(ruuvi_freq)
  {
    case RUUVI_INTERFACE_SPI_FREQUENCY_1M:
      *nrf_freq = NRF_DRV_SPI_FREQ_1M;
      return RUUVI_DRIVER_SUCCESS;

    case RUUVI_INTERFACE_SPI_FREQUENCY_2M:
      *nrf_freq = NRF_DRV_SPI_FREQ_2M;
      return RUUVI_DRIVER_SUCCESS;

    case RUUVI_INTERFACE_SPI_FREQUENCY_4M:
      *nrf_freq = NRF_DRV_SPI_FREQ_4M;
      return RUUVI_DRIVER_SUCCESS;

    case RUUVI_INTERFACE_SPI_FREQUENCY_8M:
      *nrf_freq = NRF_DRV_SPI_FREQ_8M;
      return RUUVI_DRIVER_SUCCESS;

    default:
      return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }
}


ruuvi_driver_status_t ruuvi_interface_spi_init(const ruuvi_interface_spi_init_config_t*
    config)
{
  //Return error if SPI is already init
  if(spi_init_done) { return NRF_ERROR_INVALID_STATE; }

  ruuvi_driver_status_t status = RUUVI_DRIVER_SUCCESS;
  nrf_drv_spi_mode_t mode = RUUVI_INTERFACE_SPI_MODE_0;
  nrf_drv_spi_frequency_t frequency = NRF_DRV_SPI_FREQ_1M;
  status |= ruuvi_to_nrf_spi_mode(config->mode, &mode);
  status |= ruuvi_to_nrf_spi_freq(config->frequency, &frequency);
  RUUVI_DRIVER_ERROR_CHECK(status, RUUVI_DRIVER_SUCCESS);
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin       = NRF_DRV_SPI_PIN_NOT_USED;
  spi_config.miso_pin     = config->miso;
  spi_config.mosi_pin     = config->mosi;
  spi_config.sck_pin      = config->sclk;
  spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
  spi_config.orc          = 0xFF;
  spi_config.frequency    = frequency;
  spi_config.mode         = mode;
  spi_config.bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
  // Use blocking mode by using NULL as event handler
  ret_code_t err_code = NRF_SUCCESS;
  err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
  ;

  for(size_t ii = 0; ii < config->ss_pins_number; ii++)
  {
    nrf_gpio_cfg_output(config->ss_pins[ii]);
    nrf_gpio_pin_set(config->ss_pins[ii]);
  }

  spi_init_done = true;
  return (status | ruuvi_nrf5_sdk15_to_ruuvi_error(err_code));
}

ruuvi_driver_status_t ruuvi_interface_spi_xfer_blocking(const uint8_t* tx,
    const size_t tx_len, uint8_t* rx, const size_t rx_len)
{
  //Return error if not init or if given null pointer
  if(!spi_init_done)            { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  if((NULL == tx && 0 != tx_len) || (NULL == rx && 0 != rx_len)) { return RUUVI_DRIVER_ERROR_NULL; }

  ret_code_t err_code = NRF_SUCCESS;
  err_code |= nrf_drv_spi_transfer(&spi, tx, tx_len, rx, rx_len);
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

#endif