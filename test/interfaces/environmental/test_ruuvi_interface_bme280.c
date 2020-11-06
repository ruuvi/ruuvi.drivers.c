#include "unity.h"

#include "ruuvi_interface_bme280.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_yield.h"
#include <string.h>

void setUp(void)
{
  memset(&dev, 0, sizeof(struct bme280_dev));
}

void tearDown(void)
{
}


/**
 * @brief Implement delay in Bosch signature
 *
 * @param[in] time_ms time to delay
 */
void test_bosch_delay_ms (void)
{
    const uint32_t time_ms = 1000U;
    ri_delay_ms_Expect(time_ms);
    bosch_delay_ms (time_ms);
}


/** @brief @ref rd_sensor_init_fp */
void test_ri_bme280_init_spi (void)
{
    rd_sensor_t environmental_sensor;
    memset(&environmental_sensor, 0, sizeof(rd_sensor_t));
    const uint8_t handle = 1;
    struct bme280_dev expect_dev = {
      .dev_id = handle;
      .intf = BME280_SPI_INTF;
      .read = &ri_spi_bme280_read;
      .write = &ri_spi_bme280_write;
      .delay_ms = bosch_delay_ms;
    }
        
    rd_sensor_initialize_Expect(&environmental_sensor);
    bme280_init_ExpectWithArrayAndReturn(&dev, 1, BME280_OK);

    rd_status_t ri_bme280_init (&environmental_sensor, 
                                RD_BUS_SPI, handle);
}
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_bme280_uninit (rd_sensor_t *
                              environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_bme280_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_bme280_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_mode_set (uint8_t * mode);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_mode_get (uint8_t * mode);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_bme280_data_get (rd_sensor_data_t * const
                                data);