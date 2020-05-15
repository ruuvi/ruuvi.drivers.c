/**
 * Ruuvi spi interface for lis2dh12
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/

#ifndef RUUVI_INTERFACE_SPI_LIS2DH12_H
#define RUUVI_INTERFACE_SPI_LIS2DH12_H
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Wrappers for LIS2DH12
int32_t ri_spi_lis2dh12_write (void * dev_ptr, uint8_t reg_addr,
                               uint8_t * reg_data, uint16_t len);
int32_t ri_spi_lis2dh12_read (void * dev_ptr, uint8_t reg_addr,
                              uint8_t * reg_data, uint16_t len);

#endif