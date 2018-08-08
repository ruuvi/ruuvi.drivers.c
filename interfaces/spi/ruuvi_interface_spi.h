/**
 *  This file contains interface to SPI platform drivers.
 *. Implementations are decided by compiler flags
 */

#ifndef RUUVI_INTERFACE_SPI_H
#define RUUVI_INTERFACE_SPI_H
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  RUUVI_INTERFACE_SPI_MODE_0,
  RUUVI_INTERFACE_SPI_MODE_1,
  RUUVI_INTERFACE_SPI_MODE_2,
  RUUVI_INTERFACE_SPI_MODE_3
}ruuvi_interface_spi_mode_t;

typedef enum {
  RUUVI_INTERFACE_SPI_FREQUENCY_1M,
  RUUVI_INTERFACE_SPI_FREQUENCY_2M,
  RUUVI_INTERFACE_SPI_FREQUENCY_4M,
  RUUVI_INTERFACE_SPI_FREQUENCY_8M,
}ruuvi_interface_spi_frequency_t;

typedef struct{
  uint8_t mosi;                              // pin number of MOSI
  uint8_t miso;                              // pin number of MISO
  uint8_t sclk;                              // pin number of SCLK
  uint8_t* ss_pins;                          // array of SPI pins, can be freed after function exits
  uint8_t ss_pins_number;                    // sizeof ss_pins
  ruuvi_interface_spi_frequency_t frequency;
  ruuvi_interface_spi_mode_t mode;
}ruuvi_interface_spi_init_config_t;

/**
 * Initialize SPI driver with default settings
 *
 * parameter config: Configuration of SPI peripheral
 * returns error code from the stack, RUUVI_DRIVER_SUCCESS if no error occurred
 */
 ruuvi_driver_status_t ruuvi_platform_spi_init(ruuvi_interface_spi_init_config_t* config);

/**
 */
ruuvi_driver_status_t ruuvi_platform_spi_generic_platform_xfer_blocking(uint8_t* const tx, const size_t tx_len, uint8_t* rx, const size_t rx_len);
#endif