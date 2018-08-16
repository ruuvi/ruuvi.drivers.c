/**
 * Ruuvi spi interface for bme280
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/

#ifndef RUUVI_INTERFACE_SPI_BME280_H
#define BME280
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Wrappers for BME280
int8_t ruuvi_platform_spi_bme280_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t ruuvi_platform_spi_bme280_read (uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

#endif