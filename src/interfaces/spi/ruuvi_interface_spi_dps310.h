#ifndef RUUVI_INTERFACE_SPI_DPS310_H
#define RUUVI_INTERFACE_SPI_DPS310_H
#include <stdint.h>
/**
 * @addtogroup SPI
 * @{
 */
/**
 * @file ruuvi_interface_spi_dps310.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief SPI read/write functions for Infineon DPS310.
 * @date 2020-12-31
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * You'll need to get the Ruuvi's DPS310, available at
 * <a href="https://github.com/BoschSensortec/BME280_driver">GitHub</a>.
 * The wrappers will use Ruuvi Interface internally, so you don't have to port these to
 * use DPS310 on a new platform. You're required to port @ref Yield, @ref GPIO and @ref SPI.
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
 * @brief SPI write function for DPS310
 *
 * Binds Ruuvi Interface SPI functions into Ruuvi's DPS310 driver.
 * Handles GPIO chip select, there is no forced delay to let the CS settle.
 *
 * @param[in] dev_id @ref SPI interface handle, i.e. pin number of the chip select pin of DPS310.
 * @param[in] reg_addr DPS310 register address to write.
 * @param[in] p_reg_data pointer to data to be written.
 * @param[in] len length of data to be written.
 **/
uint32_t ri_spi_dps310_write (uint8_t dev_id, uint8_t reg_addr,
                              uint8_t * p_reg_data, uint16_t len);

/**
 * @brief SPI Read function for DPS310
 *
 * Binds Ruuvi Interface SPI functions into Ruuvi DPS310 driver.
 * Handles GPIO chip select, there is no forced delay to let the CS settle.
 *
 * @param[in] dev_id @ref SPI interface handle, i.e. pin number of the chip select pin of DPS310.
 * @param[in] reg_addr DPS310 register address to read.
 * @param[in] p_reg_data pointer to data to be received.
 * @param[in] len length of data to be received.
 **/
uint32_t ri_spi_dps310_read (uint8_t dev_id, uint8_t reg_addr,
                             uint8_t * reg_data, uint16_t len);
/** @} */

#endif // RUUVI_INTERFACE_SPI_DPS310_H
