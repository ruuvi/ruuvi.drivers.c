#ifndef RUUVI_INTERFACE_I2C_TMP117_H
#define RUUVI_INTERFACE_I2C_TMP117_H
/**
 * @addtogroup I2C
 * @{
 */
/**
 * @file ruuvi_interface_i2c_tmp117.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief I2C read/write functions for TI TMP117.
 * @date 2019-07-29
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */

#include "ruuvi_driver_error.h"

/**
 * @brief I2C write function for TMP117
 *
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C addess of TMP117
 * @param[in] reg_addr TMP117 register address to write.
 * @param[in] p_reg_data pointer to data to be written.
 * @param[in] len length of data to be written.
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_NULL if p_data is null
 * @return RUUVI_DRIVER_ERROR_TIMEOUT if device does not respond on bus
 **/
int8_t ruuvi_interface_i2c_bme280_write(uint8_t dev_id, uint8_t reg_addr,
                                        uint8_t* p_reg_data, uint16_t len);

/**
 * @brief I2C Read function for TMP117
 *
 * Binds Ruuvi Interface I2C functions for TMP117
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C addess of TMP117.
 * @param[in] reg_addr TMP117 register address to read.
 * @param[in] p_reg_data pointer to data to be received.
 * @param[in] len length of data to be received.
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_NULL if p_data is null
 * @return RUUVI_DRIVER_ERROR_TIMEOUT if device does not respond on bus
 **/
int8_t ruuvi_interface_i2c_bme280_read(uint8_t dev_id, uint8_t reg_addr,
                                       uint8_t* reg_data, uint16_t len);

/*@}*/
#endif