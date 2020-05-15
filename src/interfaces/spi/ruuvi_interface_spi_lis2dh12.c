/**
 * Ruuvi spi interface for lis2dh12
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/
#include "ruuvi_driver_enabled_modules.h"
#if RI_LIS2DH12_ENABLED
#include <stdint.h>
#include <string.h>

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_spi.h"


int32_t ri_spi_lis2dh12_write (void * dev_ptr, uint8_t reg_addr,
                               uint8_t * reg_data, uint16_t len)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dev_id = * ( (uint8_t *) dev_ptr);
    // bit 0: READ bit. The value is 0.
    reg_addr &= 0x7F;

    // bit 1: MS bit. When 0, does not increment the address; when 1, increments the address in
    // multiple read / writes.
    if (len > 1) { reg_addr |= 0x40; }

    ri_gpio_id_t ss;
    ss = RD_HANDLE_TO_GPIO (dev_id);
    err_code |= ri_gpio_write (ss, RI_GPIO_LOW);
    err_code |= ri_spi_xfer_blocking (&reg_addr, 1, NULL, 0);
    err_code |= ri_spi_xfer_blocking (reg_data, len, NULL, 0);
    err_code |= ri_gpio_write (ss, RI_GPIO_HIGH);
    return err_code;
}

int32_t ri_spi_lis2dh12_read (void * dev_ptr, uint8_t reg_addr,
                              uint8_t * reg_data, uint16_t len)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dev_id = * ( (uint8_t *) dev_ptr);
    // bit 0: READ bit. The value is 1.
    reg_addr |= 0x80;

    // bit 1: MS bit. When 0, does not increment the address; when 1, increments the address in
    // multiple read / writes.
    if (len > 1) { reg_addr |= 0x40; }

    ri_gpio_id_t ss;
    ss = RD_HANDLE_TO_GPIO (dev_id);
    err_code |= ri_gpio_write (ss, RI_GPIO_LOW);
    err_code |= ri_spi_xfer_blocking (&reg_addr, 1, NULL, 0);
    err_code |= ri_spi_xfer_blocking (NULL, 0, reg_data, len);
    err_code |= ri_gpio_write (ss, RI_GPIO_HIGH);
    return err_code;
}
#endif