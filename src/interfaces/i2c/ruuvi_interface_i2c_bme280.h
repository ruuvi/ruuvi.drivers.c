#ifndef RUUVI_INTERFACE_I2C_BME280_H
#define RUUVI_INTERFACE_I2C_BME280_H
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/**
 * @addtogroup I2C
 * @{
 */
/**
 * @file ruuvi_interface_i2c_bme280.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief I2C read/write functions for Bosch BME280.
 * @date 2020-04-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * You'll need to get the official Bosch BME280 driver, available on
 * <a href="https://github.com/BoschSensortec/BME280_driver">GitHub</a>.
 * The wrappers will use Ruuvi Interface internally, so you don't have to port these to
 * use BME280 on a new platform. You're required to port @ref Yield, @ref GPIO and @ref I2C.
 *
 * General usage is:
 * @code{c}
 * static struct bme280_dev dev = {0};
 * dev.dev_id = bme280_cs_pin;
 * dev.intf = BME280_I2C_INTF;
 * dev.read = ri_i2c_bme280_read;
 * dev.write = ri_i2c_bme280_write;
 * dev.delay_ms = bosch_delay_ms;
 * bme280_init(&dev);
 * @endcode
 */

/**
 * @brief I2C write function for BME280
 *
 * Binds Ruuvi Interface I2C functions into official Bosch BME280 driver.
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C addess of BME280.
 * @param[in] reg_addr BME280 register address to write.
 * @param[in] p_reg_data pointer to data to be written.
 * @param[in] len length of data to be written.
 * @warning I2C address depends on BME280 SDO logic level
 * @return negative number on error
 * @return 0 on success
 * @return positive number on warning
 **/
int8_t ri_i2c_bme280_write (uint8_t dev_id, uint8_t reg_addr,
                            uint8_t * p_reg_data, uint16_t len);

/**
 * @brief I2C Read function for BME280
 *
 * Binds Ruuvi Interface I2C functions into official Bosch BME280 driver.
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C addess of BME280.
 * @param[in] reg_addr BME280 register address to read.
 * @param[in] p_reg_data pointer to data to be received.
 * @param[in] len length of data to be received.
 * @warning I2C address depends on BME280 SDO logic level
 * @return negative number on error
 * @return 0 on success
 * @return positive number on warning
 **/
int8_t ri_i2c_bme280_read (uint8_t dev_id, uint8_t reg_addr,
                           uint8_t * p_reg_data, uint16_t len);
/** @} */
#endif