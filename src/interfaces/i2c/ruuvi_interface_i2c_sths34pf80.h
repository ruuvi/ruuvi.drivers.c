/**
 * @addtogroup I2C
 * @{
 */
/**
 * @file ruuvi_interface_i2c_sths34pf80.h
 * @author Ruuvi Innovations Ltd
 * @brief I2C read/write functions for ST STHS34PF80.
 * @date 2025-12-22
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */
#ifndef RUUVI_INTERFACE_I2C_STHS34PF80_H
#define RUUVI_INTERFACE_I2C_STHS34PF80_H

#include <stdint.h>

/**
 * @brief I2C write function for STHS34PF80 ST driver.
 *
 * This function conforms to stmdev_write_ptr signature.
 *
 * @param[in] handle Pointer to device handle (I2C address).
 * @param[in] reg    Register address to write.
 * @param[in] data   Pointer to data buffer to write.
 * @param[in] len    Number of bytes to write.
 * @return 0 on success, non-zero on error.
 */
int32_t ri_i2c_sths34pf80_write (void * handle, uint8_t reg,
                                  const uint8_t * data, uint16_t len);

/**
 * @brief I2C read function for STHS34PF80 ST driver.
 *
 * This function conforms to stmdev_read_ptr signature.
 *
 * @param[in]  handle Pointer to device handle (I2C address).
 * @param[in]  reg    Register address to read.
 * @param[out] data   Pointer to buffer for read data.
 * @param[in]  len    Number of bytes to read.
 * @return 0 on success, non-zero on error.
 */
int32_t ri_i2c_sths34pf80_read (void * handle, uint8_t reg,
                                 uint8_t * data, uint16_t len);

#endif // RUUVI_INTERFACE_I2C_STHS34PF80_H
/** @} */
