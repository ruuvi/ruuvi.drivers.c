#ifndef RUUVI_INTERFACE_SPI_BME280_H
#define RUUVI_INTERFACE_SPI_BME280_H
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/**
 * @addtogroup SPI
 * @{
 */
/**
 * @file ruuvi_interface_spi_bme280.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief SPI read/write functions for Bosch BME280.
 * @date 2020-04-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * You'll need to get the official Bosch BME280 driver, available on
 * <a href="https://github.com/BoschSensortec/BME280_driver">GitHub</a>.
 * The wrappers will use Ruuvi Interface internally, so you don't have to port these to
 * use BME280 on a new platform. You're required to port @ref Yield, @ref GPIO and @ref SPI.
 *
 * General usage is:
 * @code{c}
 * static struct bme280_dev dev = {0};
 * dev.dev_id = bme280_cs_pin;
 * dev.intf = BME280_SPI_INTF;
 * dev.read = ri_spi_bme280_read;
 * dev.write = ri_spi_bme280_write;
 * dev.delay_ms = bosch_delay_ms;
 * bme280_init(&dev);
 * @endcode
 */

/**
 * @brief SPI write function for BME280
 *
 * Binds Ruuvi Interface SPI functions into official Bosch BME280 driver.
 * Handles GPIO chip select, there is no forced delay to let th CS settle.
 *
 * @param[in] dev_id @ref SPI interface handle, i.e. pin number of the chip select pin of BME280.
 * @param[in] reg_addr BME280 register address to write.
 * @param[in] p_reg_data pointer to data to be written.
 * @param[in] len length of data to be written.
 **/
int8_t ri_spi_bme280_write (uint8_t dev_id, uint8_t reg_addr,
                            uint8_t * p_reg_data, uint16_t len);

/**
 * @brief SPI Read function for BME280
 *
 * Binds Ruuvi Interface SPI functions into official Bosch BME280 driver.
 * Handles GPIO chip select, there is no forced delay to let th CS settle.
 *
 * @param[in] dev_id @ref SPI interface handle, i.e. pin number of the chip select pin of BME280.
 * @param[in] reg_addr BME280 register address to read.
 * @param[in] p_reg_data pointer to data to be received.
 * @param[in] len length of data to be received.
 **/
int8_t ri_spi_bme280_read (uint8_t dev_id, uint8_t reg_addr,
                           uint8_t * p_reg_data, uint16_t len);
/** @} */
#endif