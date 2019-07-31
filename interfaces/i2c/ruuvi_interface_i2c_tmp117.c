/**
 * @addtogroup I2C
 * @{
 */
/**
 * @file ruuvi_interface_i2c_tmp117.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief I2C read/write functions for TI TMP117.
 * @date 2019-07-29
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ENVIRONMENTAL_SHTCX_ENABLED || DOXYGEN
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_i2c_tmp117.h"

ruuvi_driver_status_t ruuvi_interface_i2c_tmp117_write(uint8_t dev_id, uint8_t reg_addr,
                                                       uint8_t* p_reg_data, uint16_t len)
{
  if(NULL == p_reg_data) { return RUUVI_DRIVER_ERROR_NULL; }
  if(2 != len) { return RUUVI_DRIVER_ERROR_INVALID_LENGTH; }
  uint8_t command[2];
  command[0] = reg_addr;
  command[1] = p_reg_data[0];
  return ruuvi_interface_i2c_write_blocking(dev_id, command, sizeof(command), true);
}

ruuvi_driver_status_t ruuvi_interface_i2c_tmp117_read(uint8_t dev_id, uint8_t reg_addr,
                                                      uint8_t* p_reg_data, uint16_t len)
{
  if(NULL == p_reg_data) { return RUUVI_DRIVER_ERROR_NULL; }
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  err_code |= ruuvi_interface_i2c_write_blocking(dev_id, &reg_addr, 1, false);
  err_code |= ruuvi_interface_i2c_read_blocking(dev_id, p_reg_data, len);
  return err_code;
}
#endif
/*@}*/
