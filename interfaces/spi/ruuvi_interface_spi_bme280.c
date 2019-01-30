/**
 * Ruuvi spi interface for bme280
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ENVIRONMENTAL_BME280_ENABLED
#include <stdint.h>
#include <string.h> //memcpy

#include "ruuvi_boards.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_spi.h"
#include "ruuvi_interface_yield.h"


int8_t ruuvi_interface_spi_bme280_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= ruuvi_platform_gpio_write(dev_id, RUUVI_INTERFACE_GPIO_LOW);
  err_code |= ruuvi_platform_spi_xfer_blocking(&reg_addr, 1, NULL, 0);
  err_code |= ruuvi_platform_spi_xfer_blocking(reg_data, len, NULL, 0);
  err_code |= ruuvi_platform_gpio_write(dev_id, RUUVI_INTERFACE_GPIO_HIGH);
  return (RUUVI_DRIVER_SUCCESS == err_code) ? 0 : -1;
}

int8_t ruuvi_interface_spi_bme280_read (uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= ruuvi_platform_gpio_write(dev_id, RUUVI_INTERFACE_GPIO_LOW);
  err_code |= ruuvi_platform_spi_xfer_blocking(&reg_addr, 1, NULL, 0);
  err_code |= ruuvi_platform_spi_xfer_blocking(NULL, 0, reg_data, len);
  err_code |= ruuvi_platform_gpio_write(dev_id, RUUVI_INTERFACE_GPIO_HIGH);
  return (RUUVI_DRIVER_SUCCESS == err_code) ? 0 : -1;
}
#endif