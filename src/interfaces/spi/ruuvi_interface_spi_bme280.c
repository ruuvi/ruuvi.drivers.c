#include "ruuvi_driver_enabled_modules.h"
#if (RI_BME280_ENABLED && RI_BME280_SPI_ENABLED) || DOXYGEN
/**
 * @addtogroup SPI SPI functions
 * @brief Functions for using SPI bus
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_spi_bme280.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-04-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Implementation for SPI operations
 *
 */
#include <stdint.h>
#include <string.h>

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_spi.h"
#include "ruuvi_interface_yield.h"


int8_t ri_spi_bme280_write (uint8_t dev_id, uint8_t reg_addr,
                            uint8_t * reg_data, uint16_t len)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_id_t ss;
    ss = RD_HANDLE_TO_GPIO (dev_id);
    err_code |= ri_gpio_write (ss, RI_GPIO_LOW);
    err_code |= ri_spi_xfer_blocking (&reg_addr, 1, NULL, 0);
    err_code |= ri_spi_xfer_blocking (reg_data, len, NULL, 0);
    err_code |= ri_gpio_write (ss, RI_GPIO_HIGH);
    return (RD_SUCCESS == err_code) ? 0 : -1;
}

int8_t ri_spi_bme280_read (uint8_t dev_id, uint8_t reg_addr,
                           uint8_t * reg_data, uint16_t len)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_id_t ss;
    ss = RD_HANDLE_TO_GPIO (dev_id);
    err_code |= ri_gpio_write (ss, RI_GPIO_LOW);
    err_code |= ri_spi_xfer_blocking (&reg_addr, 1, NULL, 0);
    err_code |= ri_spi_xfer_blocking (NULL, 0, reg_data, len);
    err_code |= ri_gpio_write (ss, RI_GPIO_HIGH);
    return (RD_SUCCESS == err_code) ? 0 : -1;
}
/*@}*/
#endif