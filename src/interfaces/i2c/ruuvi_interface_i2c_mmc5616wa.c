#include "ruuvi_driver_enabled_modules.h"
#if (RI_MMC5616WA_ENABLED || DOXYGEN)

#include "mmc5616wa.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_i2c_mmc5616wa.h"

#define RI_I2C_MMC5616WA_WRITE_MAX_LEN (16U)

int32_t ri_i2c_mmc5616wa_write (void * handle, uint8_t reg_addr,
                                const uint8_t * reg_data, uint16_t len)
{
    if ( (NULL == handle) || (NULL == reg_data))
    {
        return MMC5616WA_E_NULL;
    }

    if ( (0U == len) || (RI_I2C_MMC5616WA_WRITE_MAX_LEN < len))
    {
        return MMC5616WA_E_INVALID_PARAM;
    }

    const uint8_t dev_id = * ( (uint8_t *) handle);
    uint8_t tx[RI_I2C_MMC5616WA_WRITE_MAX_LEN + 1U] = {0};
    tx[0] = reg_addr;

    for (uint16_t ii = 0U; ii < len; ii++)
    {
        tx[ii + 1U] = reg_data[ii];
    }

    const rd_status_t err_code = ri_i2c_write_blocking (dev_id, tx,
                                 (size_t) len + 1U, true);
    return (RD_SUCCESS == err_code) ? MMC5616WA_OK : MMC5616WA_E_IO;
}

int32_t ri_i2c_mmc5616wa_read (void * handle, uint8_t reg_addr,
                               uint8_t * reg_data, uint16_t len)
{
    if ( (NULL == handle) || (NULL == reg_data))
    {
        return MMC5616WA_E_NULL;
    }

    if (0U == len)
    {
        return MMC5616WA_E_INVALID_PARAM;
    }

    const uint8_t dev_id = * ( (uint8_t *) handle);
    rd_status_t err_code = ri_i2c_write_blocking (dev_id, &reg_addr, 1U, false);

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_i2c_read_blocking (dev_id, reg_data, len);
    }

    return (RD_SUCCESS == err_code) ? MMC5616WA_OK : MMC5616WA_E_IO;
}

#endif
