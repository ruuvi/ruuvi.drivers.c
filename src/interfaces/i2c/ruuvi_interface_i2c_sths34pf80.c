/**
 * @addtogroup I2C
 * @{
 */
/**
 * @file ruuvi_interface_i2c_sths34pf80.c
 * @author Ruuvi Innovations Ltd
 * @brief I2C read/write functions for ST STHS34PF80.
 * @date 2025-12-22
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#if RI_STHS34PF80_ENABLED || DOXYGEN
#include <stdint.h>
#include <string.h>

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_i2c_sths34pf80.h"

#define REG_ADDR_SIZE (1U)

int32_t ri_i2c_sths34pf80_write (void * handle, uint8_t reg,
                                  const uint8_t * data, uint16_t len)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dev_addr;
    uint8_t tx_buf[len + REG_ADDR_SIZE];

    if (NULL == handle || NULL == data)
    {
        return (int32_t) RD_ERROR_NULL;
    }

    dev_addr = * ((uint8_t *) handle);

    // Build buffer: [register address][data...]
    tx_buf[0] = reg;
    memcpy (&tx_buf[1], data, len);

    err_code = ri_i2c_write_blocking (dev_addr, tx_buf, len + REG_ADDR_SIZE, true);

    return (int32_t) err_code;
}

int32_t ri_i2c_sths34pf80_read (void * handle, uint8_t reg,
                                 uint8_t * data, uint16_t len)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dev_addr;
    uint8_t reg_addr = reg;

    if (NULL == handle || NULL == data)
    {
        return (int32_t) RD_ERROR_NULL;
    }

    dev_addr = * ((uint8_t *) handle);

    // Write register address (no stop), then read data
    err_code |= ri_i2c_write_blocking (dev_addr, &reg_addr, REG_ADDR_SIZE, false);
    err_code |= ri_i2c_read_blocking (dev_addr, data, len);

    return (int32_t) err_code;
}

#endif // RI_STHS34PF80_ENABLED
/** @} */
