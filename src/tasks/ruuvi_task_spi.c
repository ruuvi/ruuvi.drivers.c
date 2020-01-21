#include "ruuvi_driver_enabled_modules.h"
#if RT_SPI_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_spi.h"
#include "ruuvi_task_spi.h"

// TODO @ojousima Refactor board dependency out
rd_status_t rt_spi_init (void)
{
    ri_spi_init_config_t config;
    ri_gpio_id_t ss_pins[] = RB_SPI_SS_LIST;
    config.mosi = RB_SPI_MOSI_PIN;
    config.miso = RB_SPI_MISO_PIN;
    config.sclk = RB_SPI_SCLK_PIN;
    config.ss_pins = ss_pins;
    config.ss_pins_number = sizeof (ss_pins) / sizeof (ri_gpio_id_t);
    // Assume mode 0 always.
    config.mode = RI_SPI_MODE_0;

    switch (RB_SPI_FREQ)
    {
        case RB_SPI_FREQUENCY_1M:
            config.frequency = RI_SPI_FREQUENCY_1M;
            break;

        case RB_SPI_FREQUENCY_2M:
            config.frequency = RI_SPI_FREQUENCY_2M;
            break;

        case RB_SPI_FREQUENCY_4M:
            config.frequency = RI_SPI_FREQUENCY_4M;
            break;

        case RB_SPI_FREQUENCY_8M:
            config.frequency = RI_SPI_FREQUENCY_8M;
            break;

        default:
            config.frequency = RI_SPI_FREQUENCY_1M;
            ri_log (RI_LOG_LEVEL_WARNING,
                                 "Unknown SPI frequency, defaulting to 1M\r\n");
    }

    return ri_spi_init (&config);
}
#endif
