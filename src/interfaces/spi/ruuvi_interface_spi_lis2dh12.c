/**
 * Ruuvi spi interface for lis2dh12
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ACCELERATION_LIS2DH12_ENABLED
#include <stdint.h>
#include <string.h>

#include "ruuvi_boards.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_spi.h"


int32_t ruuvi_interface_spi_lis2dh12_write (void * dev_ptr, uint8_t reg_addr,
        uint8_t * reg_data, uint16_t len)
{
    ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
    uint8_t dev_id = * ( (uint8_t *) dev_ptr);
    // bit 0: READ bit. The value is 0.
    reg_addr &= 0x7F;

    // bit 1: MS bit. When 0, does not increment the address; when 1, increments the address in
    // multiple read / writes.
    if (len > 1) { reg_addr |= 0x40; }

    ruuvi_interface_gpio_id_t ss;
    ss.pin = RUUVI_DRIVER_HANDLE_TO_GPIO (dev_id);
    err_code |= ruuvi_interface_gpio_write (ss, RUUVI_INTERFACE_GPIO_LOW);
    err_code |= ruuvi_interface_spi_xfer_blocking (&reg_addr, 1, NULL, 0);
    err_code |= ruuvi_interface_spi_xfer_blocking (reg_data, len, NULL, 0);
    err_code |= ruuvi_interface_gpio_write (ss, RUUVI_INTERFACE_GPIO_HIGH);
    return err_code;
}

int32_t ruuvi_interface_spi_lis2dh12_read (void * dev_ptr, uint8_t reg_addr,
        uint8_t * reg_data, uint16_t len)
{
    ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
    uint8_t dev_id = * ( (uint8_t *) dev_ptr);
    // bit 0: READ bit. The value is 1.
    reg_addr |= 0x80;

    // bit 1: MS bit. When 0, does not increment the address; when 1, increments the address in
    // multiple read / writes.
    if (len > 1) { reg_addr |= 0x40; }

    ruuvi_interface_gpio_id_t ss;
    ss.pin = RUUVI_DRIVER_HANDLE_TO_GPIO (dev_id);
    err_code |= ruuvi_interface_gpio_write (ss, RUUVI_INTERFACE_GPIO_LOW);
    err_code |= ruuvi_interface_spi_xfer_blocking (&reg_addr, 1, NULL, 0);
    err_code |= ruuvi_interface_spi_xfer_blocking (NULL, 0, reg_data, len);
    err_code |= ruuvi_interface_gpio_write (ss, RUUVI_INTERFACE_GPIO_HIGH);
    return err_code;
}
#endif