#include "ruuvi_driver_enabled_modules.h"
#if RI_BME280_ENABLED && RI_BME280_I2C_ENABLED
/**
 * @addtogroup I2C
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_i2c_bme280.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-04-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Wrapper for BME280 I2C calls
 *
 */
#include <stdint.h>
#include <string.h>

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_yield.h"

#define I2C_BME280_BUFF_SIZE_MIN    1   //!< Minimum size of buffer.
#define I2C_BME280_BUFF_SIZE_MAX    2   //!< Maximum size of buffer.

/*
 * Data on the bus should be like
 * |------------+---------------------|
 * | I2C action | Data                |
 * |------------+---------------------|
 * | Start      | -                   |
 * | Write      | (reg_addr)          |
 * | Write      | (reg_data[0])       |
 * | Write      | (....)              |
 * | Write      | (reg_data[len - 1]) |
 * | Stop       | -                   |
 * |------------+---------------------|
 */
int8_t ri_i2c_bme280_write (uint8_t dev_id, uint8_t reg_addr,
                            uint8_t * reg_data, uint16_t len)
{
    rd_status_t err_code = RD_SUCCESS;
    int8_t result = 0;

    // Support only 1-byte writes
    if ( (I2C_BME280_BUFF_SIZE_MIN > len) || (I2C_BME280_BUFF_SIZE_MAX < len))
    {
        result = -1;
    }
    else
    {
        uint8_t wbuf[I2C_BME280_BUFF_SIZE_MAX] = {0};
        wbuf[0] = reg_addr;
        wbuf[1] = reg_data[0];
        err_code |= ri_i2c_write_blocking (dev_id, wbuf, I2C_BME280_BUFF_SIZE_MAX, true);

        if (RD_SUCCESS != err_code)
        {
            result = -1;
        }
    }

    return result;
}

/*
 * Data on the bus should be like
 * |------------+---------------------|
 * | I2C action | Data                |
 * |------------+---------------------|
 * | Start      | -                   |
 * | Write      | (reg_addr)          |
 * | Stop       | -                   |
 * | Start      | -                   |
 * | Read       | (reg_data[0])       |
 * | Read       | (....)              |
 * | Read       | (reg_data[len - 1]) |
 * | Stop       | -                   |
 * |------------+---------------------|
 */

int8_t ri_i2c_bme280_read (uint8_t dev_id, uint8_t reg_addr,
                           uint8_t * reg_data, uint16_t len)
{
    rd_status_t err_code = RD_SUCCESS;
    int8_t result = 0;
    err_code |= ri_i2c_write_blocking (dev_id, &reg_addr, 1, true);
    err_code |= ri_i2c_read_blocking (dev_id, reg_data, len);

    if (RD_SUCCESS != err_code)
    {
        result = -1;
    }

    return result;
}
#endif
