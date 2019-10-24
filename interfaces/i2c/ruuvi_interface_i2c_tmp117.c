/**
 * @addtogroup I2C
 * @{
 */
/**
 * @file ruuvi_interface_i2c_tmp117.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief I2C read/write functions for TI TMP117.
 * @date 2019-10-23
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ENVIRONMENTAL_SHTCX_ENABLED || DOXYGEN
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_i2c_tmp117.h"

ruuvi_driver_status_t ruuvi_interface_i2c_tmp117_write(const uint8_t dev_id, const uint8_t reg_addr, const uint16_t reg_val)
{
  uint8_t command[3];
  command[0] = reg_addr;
  command[1] = reg_val >> 8;
  command[2] = reg_val & 0xFF;
  return ruuvi_interface_i2c_write_blocking(dev_id, command, sizeof(command), true);
}

ruuvi_driver_status_t ruuvi_interface_i2c_tmp117_read(const uint8_t dev_id, const uint8_t reg_addr,
                                                      uint16_t* const reg_val)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  uint8_t command[3] = {0};
  command[0] = reg_addr;

  err_code |= ruuvi_interface_i2c_write_blocking(dev_id, command, 1, false);
  err_code |= ruuvi_interface_i2c_read_blocking(dev_id, &(command[1]), sizeof(command));
  *reg_val = (command[1] << 8) + command[2];
  return err_code;
}
#endif
/*@}*/
