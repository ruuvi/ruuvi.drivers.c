/**
 *  This file contains interface to I2C platform drivers. 
 *. Implementations are decided by compiler flags
 */

#ifndef I2C_H
#define I2C_H
#include "ruuvi_error.h"
/**
 * @brief initialize SPI driver with default settings
 * @return 0 on success, NRF error code on error
 */
ruuvi_status_t i2c_init(void);

/**
 * @brief uninitialize SPI driver with default settings
 * @return 0 on success, NRF error code on error
 */
ruuvi_status_t i2c_uninit(void);


/**
 * @brief platform I2C write command for STM drivers
 */
int32_t i2c_stm_platform_write(void* dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief platform I2C read command for STM drivers
 */
int32_t i2c_stm_platform_read(void* dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
#endif