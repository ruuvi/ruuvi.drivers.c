#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_spi_dps310.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_spi.h"
#include <stdint.h>

#if RI_DPS310_SPI_ENABLED

uint32_t ri_spi_dps310_write (const void * const comm_ctx, const uint8_t reg_addr,
                              const uint8_t * const data, const uint8_t data_len)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_id_t ss = * (ri_gpio_id_t *) comm_ctx;
    ss = RD_HANDLE_TO_GPIO (ss);
    err_code |= ri_gpio_write (ss, RI_GPIO_LOW);
    err_code |= ri_spi_xfer_blocking (&reg_addr, 1, NULL, 0);
    err_code |= ri_spi_xfer_blocking (data, data_len, NULL, 0);
    err_code |= ri_gpio_write (ss, RI_GPIO_HIGH);
    return err_code;
}

uint32_t ri_spi_dps310_read (const void * const comm_ctx, const uint8_t reg_addr,
                             uint8_t * const data, const uint8_t data_len)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_id_t ss = * (ri_gpio_id_t *) comm_ctx;
    uint8_t read_cmd = reg_addr | 0x80;
    ss = RD_HANDLE_TO_GPIO (ss);
    err_code |= ri_gpio_write (ss, RI_GPIO_LOW);
    err_code |= ri_spi_xfer_blocking (&read_cmd, 1, NULL, 0);
    err_code |= ri_spi_xfer_blocking (NULL, 0, data, data_len);
    err_code |= ri_gpio_write (ss, RI_GPIO_HIGH);
    return err_code;
}

#endif