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
#if NRF_SDK15_SPI
#include <stdint.h>
#include <string.h> //memcpy

#include "spi.h"
#include "boards.h"

#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "ruuvi_error.h"
#include "yield.h"

#define PLATFORM_LOG_MODULE_NAME spi_platform
#if SPI_PLATFORM_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       SPI_PLATFORM_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  SPI_PLATFORM_INFO_COLOR
#else // ANT_BPWR_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       0
#endif // ANT_BPWR_LOG_ENABLED
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

#define SPI_INSTANCE  BOARD_SPI_INSTANCE /**< SPI instance index. */
#if (BOARD_SPI_FREQUENCY == RUUVI_SPI_FREQ_0M25)
#define SPI_FREQUENCY NRF_DRV_SPI_FREQ_250K
#elif (BOARD_SPI_FREQUENCY == RUUVI_SPI_FREQ_1M)
#define SPI_FREQUENCY NRF_DRV_SPI_FREQ_1M
#elif (BOARD_SPI_FREQUENCY == RUUVI_SPI_FREQ_8M)
#define SPI_FREQUENCY NRF_DRV_SPI_FREQ_8M
#else
#define SPI_FREQUENCY SPI0_DEFAULT_FREQUENCY
#endif

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done = true;  /**< Flag used to indicate that SPI instance completed the transfer. */
static volatile bool spi_init_done = false;  /**< Flag used to indicate that SPI instance is initialized. */

/**
 * @brief SPI user event handler.
 * @param event
 */
static void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                              void *                    p_context)
{
  spi_xfer_done = true;
  // NRF_LOG_INFO("Transfer completed.");
}

/**
 * @brief initialize SPI driver with default settings
 * @return 0 on success, Ruuvi error code on error
 */
ruuvi_status_t spi_init(void)
{
  //Return error if SPI is already init
  if (spi_init_done) { return NRF_ERROR_INVALID_STATE; }

  //TODO Configure in board settings
  ret_code_t err_code = NRF_SUCCESS;
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin       = NRF_DRV_SPI_PIN_NOT_USED;
  spi_config.miso_pin     = SPIM0_MISO_PIN;
  spi_config.mosi_pin     = SPIM0_MOSI_PIN;
  spi_config.sck_pin      = SPIM0_SCK_PIN;
  spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
  spi_config.orc          = 0xFF;
  spi_config.frequency    = SPI_FREQUENCY;
  spi_config.mode         = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
  err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
  // APP_ERROR_CHECK(err_code);

#if SPIM0_SS_MEMORY_PIN
  /* Init chipselect for memory */
  nrf_gpio_cfg_output(SPIM0_SS_MEMORY_PIN);
  nrf_gpio_pin_set(SPIM0_SS_MEMORY_PIN);
#endif

#if SPIM0_SS_GYROSCOPE_PIN
  /* Init chipselect for Gyroscope */
  PLATFORM_LOG_DEBUG("Enabling gyro CS");
  nrf_gpio_cfg_output(SPIM0_SS_GYROSCOPE_PIN);
  nrf_gpio_pin_set(SPIM0_SS_GYROSCOPE_PIN);
#endif

#if SPIM0_SS_ENVIRONMENTAL_PIN
  /* Init chipselect for Environmental */
  nrf_gpio_cfg_output(SPIM0_SS_ENVIRONMENTAL_PIN);
  nrf_gpio_pin_set(SPIM0_SS_ENVIRONMENTAL_PIN);
#endif

#if SPIM0_SS_ACCELERATION_PIN
  /* Init chipselect for Accelerometer */
  nrf_gpio_cfg_output(SPIM0_SS_ACCELERATION_PIN);
  nrf_gpio_pin_set(SPIM0_SS_ACCELERATION_PIN);
#endif

  if (NRF_SUCCESS == err_code) { spi_init_done = true; }
  // NRF_LOG_INFO("SPI INIT completed.");
  return platform_to_ruuvi_error(&err_code);
}

/**
 * @brief uninitialize SPI driver with default settings
 * @return 0 on success, Ruuvi error code on error
 */
ruuvi_status_t spi_uninit(void)
{
  //Return error if SPI is already init
  if (!spi_init_done) { return NRF_ERROR_INVALID_STATE; }

  ret_code_t err_code = NRF_SUCCESS;
  nrf_drv_spi_uninit (&spi);

  if (NRF_SUCCESS == err_code) { spi_init_done = false; }
  return platform_to_ruuvi_error(&err_code);
}

/**
 * @brief platform SPI write command for Bosch drivers
 */
int8_t spi_bosch_platform_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  //Return error if not init or if busy
  if (!spi_init_done) { return NRF_ERROR_INVALID_STATE; }
  if (!spi_xfer_done) { return NRF_ERROR_BUSY; }
  PLATFORM_LOG_DEBUG("Start Bosch transfer.");

  //Lock driver
  spi_xfer_done = false;
  //Use int32t to clear out any extra bytes after 8 bits
  int8_t err_code = NRF_SUCCESS;

  nrf_gpio_pin_clear(dev_id);
  //TX address
  err_code |= nrf_drv_spi_transfer(&spi, &reg_addr, 1, NULL, 0);
  if (RUUVI_SUCCESS != err_code) { return err_code; }
  while (!spi_xfer_done)
  {
    //err_code |= platform_yield();
  }

  //Write data
  spi_xfer_done = false;
  err_code |= nrf_drv_spi_transfer(&spi, data, len, NULL, 0);
  if (RUUVI_SUCCESS != err_code) { return err_code; }
  while (!spi_xfer_done)
  {
    err_code |= platform_yield();
  }
  nrf_gpio_pin_set(dev_id);

  PLATFORM_LOG_DEBUG("Bosch transfer completed");
  return err_code;
}

/**
 * @brief platform SPI read command for Bosch drivers
 */
int8_t spi_bosch_platform_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  //Return error if not init or if busy
  // NRF_LOG_INFO("Enter Bosch read. %x %x", spi_init_done, spi_xfer_done);
  if (!spi_init_done) { return NRF_ERROR_INVALID_STATE; }
  if (!spi_xfer_done) { return NRF_ERROR_BUSY; }
  PLATFORM_LOG_DEBUG("Start Bosch read.");

  //Lock driver
  spi_xfer_done = false;

  //Use int32t to clear out any extra bytes after 8 bits
  int8_t err_code = NRF_SUCCESS;

  nrf_gpio_pin_clear(dev_id);
  // Use this code if EASY DMA is in use
  // uint8_t p_write[40] = {0};
  // uint8_t p_read[40]  = {0};
  // p_write[0] = reg_addr;
  // //TX address
  // err_code |= nrf_drv_spi_transfer(&spi, p_write, len+1, p_read, len+1);
  // while (!spi_xfer_done)
  // {
  //   err_code |= platform_yield();NRF_LOG_INFO("Yield");
  // }

  // memcpy(data, p_read+1, len);

  // Use this code if EASY DMA is disabled to avoid extra byte being clocked out on 1-register reads
  // http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52832.Rev2.errata/dita/errata/nRF52832/Rev2/latest/anomaly_832_58.html?cp=2_1_1_0_1_8
#if SPI0_USE_EASY_DMA
#error "setup_workaround_for_ftpan_58, see comment above this line."
#endif
  err_code |= nrf_drv_spi_transfer(&spi, &reg_addr, 1, NULL, 0);
  if (RUUVI_SUCCESS != err_code) { return err_code; }
  while (!spi_xfer_done)
  {
  }
  spi_xfer_done = false;
  err_code |= nrf_drv_spi_transfer(&spi, NULL, 0, data, len);
  if (RUUVI_SUCCESS != err_code) { return err_code; }
  while (!spi_xfer_done)
  {
    err_code |= platform_yield();
  }
  nrf_gpio_pin_set(dev_id);
  PLATFORM_LOG_DEBUG("SPI Read err_code %d", err_code);
  return err_code;
}


/**
 * @brief platform SPI read command for STM drivers
 * Bosch driver causes minor delay between address and register writing which
 * breaks lis2dh12. Therefore here's a separate function for stm drivers
 */
int32_t spi_lis2dh12_platform_write(void* dev_id, uint8_t reg_addr, uint8_t *data,
                                    uint16_t len)
{
  //Return error if not init or if busy
  if (!spi_init_done) { return NRF_ERROR_INVALID_STATE; }
  if (!spi_xfer_done) { return NRF_ERROR_BUSY; }

  //Lock driver
  spi_xfer_done = false;

  ret_code_t err_code = NRF_SUCCESS;
  uint8_t ss = *(uint8_t*)dev_id;

  nrf_gpio_pin_clear(ss);

  uint8_t p_write[10] = {0};
  p_write[0] = reg_addr;
  memcpy(p_write + 1, data, len);

  err_code |= nrf_drv_spi_transfer(&spi, p_write, len + 1, NULL, 0);
  while (!spi_xfer_done)
  {
    err_code |= platform_yield();
  }

  nrf_gpio_pin_set(ss);
  PLATFORM_LOG_DEBUG("SPI Write err_code %d", err_code);
  return platform_to_ruuvi_error(&err_code);
}

/**
 * @brief platform SPI read command for LIS2DH12 driver
 */
int32_t spi_lis2dh12_platform_read(void* dev_id, uint8_t reg_addr, uint8_t *data,
                                   uint16_t len)
{
  uint8_t ss = *(uint8_t*)dev_id;
  // bit 0: READ bit. The value is 1.
  // bit 1: MS bit. When 0, does not increment the address; when 1, increments the address in
  // multiple reads.
  uint8_t read_cmd = reg_addr;
  if (len > 1) { read_cmd |= 0x40; }
  return spi_bosch_platform_read(ss, read_cmd, data, len);
}

int32_t spi_lis2dw12_platform_read(void* dev_id, uint8_t reg_addr, uint8_t *data,
                                   uint16_t len)
{
  uint8_t ss = *(uint8_t*)dev_id;
  // bit 0: READ bit. The value is 1.
  uint8_t read_cmd = reg_addr | 0x80;
  return spi_bosch_platform_read(ss, read_cmd, data, len);
}

/**
 * @brief platform SPI read command for LIS2DW12 driver
 */
int32_t spi_lis2dw12_platform_write(void* dev_id, uint8_t reg_addr, uint8_t *data,
                                    uint16_t len)
{
  return spi_lis2dh12_platform_write(dev_id, reg_addr, data, len);
}


#endif