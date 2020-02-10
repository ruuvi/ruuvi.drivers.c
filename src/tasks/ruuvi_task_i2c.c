/**
 * Ruuvi Firmware 3.x I2C tasks.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/
#include "ruuvi_driver_enabled_modules.h"
#if RT_I2C_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_task_i2c.h"

rd_status_t rt_i2c_init (void)
{
    ri_i2c_init_config_t config;
    ri_gpio_id_t scl = RB_I2C_SCL_PIN;
    ri_gpio_id_t sda = RB_I2C_SDA_PIN;
    config.sda = sda;
    config.scl = scl;

    switch (RB_I2C_FREQ)
    {
        case RB_I2C_FREQUENCY_100k:
            config.frequency = RB_I2C_FREQUENCY_100k;
            break;

        case RB_I2C_FREQUENCY_250k:
            config.frequency = RI_I2C_FREQUENCY_250k;
            break;

        case RB_I2C_FREQUENCY_400k:
            config.frequency = RI_I2C_FREQUENCY_400k;
            break;

        default:
            config.frequency = RI_I2C_FREQUENCY_100k;
            ri_log (RI_LOG_LEVEL_WARNING,
                                 "Unknown I2C frequency, defaulting to 100k\r\n");
    }

    return ri_i2c_init (&config);
}
#endif