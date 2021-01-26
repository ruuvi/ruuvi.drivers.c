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
 * @date 2021-01-25
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
 * @param[in] reg_val 16-bit value to be written
 * @return RD_SUCCESS on success
 * @return RD_ERROR_TIMEOUT if device does not respond on bus
 **/
rd_status_t ri_i2c_tmp117_write (const uint8_t dev_id,
                                 const uint8_t reg_addr,
                                 const uint16_t reg_val);

/**
 * @brief I2C Read function for TMP117
 *
 * Binds Ruuvi Interface I2C functions for TMP117
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C addess of TMP117.
 * @param[in] reg_addr TMP117 register address to read.
 * @param[in] reg_val pointer to 16-bit data to be received.
 * @return RD_SUCCESS on success
 * @return RD_ERROR_TIMEOUT if device does not respond on bus
 **/
rd_status_t ri_i2c_tmp117_read (const uint8_t dev_id,
                                const uint8_t reg_addr,
                                uint16_t * const reg_val);

/** @} */
#endif
