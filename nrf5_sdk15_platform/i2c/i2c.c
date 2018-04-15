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
#include "sdk_application_config.h"
#if NRF_SDK15_I2C
#include <stdint.h>
#include <string.h> //memcpy

#include "i2c.h"
#include "boards.h"

#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "ruuvi_error.h"
#include "yield.h"

#define PLATFORM_LOG_MODULE_NAME i2c_platform
#if I2C_PLATFORM_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       I2C_PLATFORM_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  I2C_PLATFORM_INFO_COLOR
#else // ANT_BPWR_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       0
#endif // ANT_BPWR_LOG_ENABLED
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

#define I2C_INSTANCE  BOARD_I2C_INSTANCE /**< SPI instance index. */
#if (BOARD_I2C_FREQUENCY == RUUVI_I2C_FREQ_100k)
#define I2C_FREQUENCY NRF_TWI_FREQ_100K
#elif (BOARD_I2C_FREQUENCY == RUUVI_I2C_FREQ_250k)
#define I2C_FREQUENCY NRF_TWI_FREQ_250K
#elif (BOARD_I2C_FREQUENCY == RUUVI_I2C_FREQ_400k)
#define I2C_FREQUENCY NRF_TWI_FREQ_400K
#else
#define I2C_FREQUENCY I2C1_DEFAULT_FREQUENCY
#endif
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(I2C_INSTANCE);
static bool i2c_is_init = false;
/**
 * @brief initialize SPI driver with default settings
 * @return 0 on success, NRF error code on error
 */
ruuvi_status_t i2c_init(void)
{
  ret_code_t err_code;

  const nrf_drv_twi_config_t twi_config = {
    .scl                = BOARD_SCL_PIN,
    .sda                = BOARD_SDA_PIN,
    .frequency          = I2C_FREQUENCY,
    .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
    .clear_bus_init     = false
  };
  //TODO: Implement callbacks
  err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
  APP_ERROR_CHECK(err_code);

  nrf_drv_twi_enable(&m_twi);
  i2c_is_init = true;
  return platform_to_ruuvi_error(&err_code);
}

/**
 * @brief uninitialize SPI driver with default settings
 * @return 0 on success, NRF error code on error
 */
ruuvi_status_t i2c_uninit(void)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief platform I2C write command for STM drivers
 */
int32_t i2c_stm_platform_write(void* dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  if (!i2c_is_init) { return RUUVI_ERROR_INVALID_STATE; }
  if (NULL == dev_id || NULL == data) { return RUUVI_ERROR_NULL; }
  int32_t err_code = NRF_SUCCESS;
  //Write address, do not stop
  err_code |= nrf_drv_twi_tx(&m_twi, *(uint8_t*)dev_id, &reg_addr, 1, true);
  //Write data, stop
  err_code |= nrf_drv_twi_tx(&m_twi, *(uint8_t*)dev_id, data, len, false);
  if (NRF_SUCCESS != err_code) { NRF_LOG_ERROR("I2C error: %X", err_code); }
  return platform_to_ruuvi_error(&err_code);
}

/**
 * @brief platform I2C read command for STM drivers
 */
int32_t i2c_stm_platform_read(void* dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  if (!i2c_is_init) { return RUUVI_ERROR_INVALID_STATE; }
  if (NULL == dev_id || NULL == data) { return RUUVI_ERROR_NULL; }

  int32_t err_code = NRF_SUCCESS;
  //Write address, do not stop
  err_code |= nrf_drv_twi_tx(&m_twi, *(uint8_t*)dev_id, &reg_addr, 1, true);
  // Read data
  err_code |= nrf_drv_twi_rx(&m_twi, *(uint8_t*)dev_id, data, len);
  if (NRF_SUCCESS != err_code) { NRF_LOG_ERROR("I2C error: %X", err_code); }
  return platform_to_ruuvi_error(&err_code);
}

#endif