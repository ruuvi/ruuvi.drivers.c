/**
 * @addtogroup I2C
 * @{
 */
/**
 * @file ruuvi_interface_i2c_tmp117.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief I2C read/write functions for TI TMP117.
 * @date 2021-01-25
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#if RI_TMP117_ENABLED || DOXYGEN
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_i2c_tmp117.h"

rd_status_t ri_i2c_tmp117_write (const uint8_t dev_id,
                                 const uint8_t reg_addr, const uint16_t reg_val)
{
    uint8_t command[3];
    command[0] = reg_addr;
    command[1] = reg_val >> 8;
    command[2] = reg_val & 0xFF;
    return ri_i2c_write_blocking (dev_id, command, sizeof (command), true);
}

rd_status_t ri_i2c_tmp117_read (const uint8_t dev_id,
                                const uint8_t reg_addr,
                                uint16_t * const reg_val)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t command[3] = {0};
    command[0] = reg_addr;
    err_code |= ri_i2c_write_blocking (dev_id, command, 1, false);
    err_code |= ri_i2c_read_blocking (dev_id, & (command[1]), sizeof (command));
    *reg_val = (command[1] << 8) + command[2];
    return err_code;
}
#endif
/** @} */
