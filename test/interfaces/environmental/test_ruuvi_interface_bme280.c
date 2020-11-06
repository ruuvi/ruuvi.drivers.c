#include "unity.h"

#include "ruuvi_interface_bme280.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_i2c_bme280.h"
#include "mock_ruuvi_interface_spi_bme280.h"
#include "mock_ruuvi_interface_yield.h"
#include <string.h>

#include "mock_bme280.h"
#include "bme280_defs.h"
#include "mock_bme280_selftest.h"

extern struct bme280_dev dev;

void setUp (void)
{
    memset (&dev, 0, sizeof (struct bme280_dev));
}

void tearDown (void)
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
    ri_delay_ms_ExpectAndReturn (time_ms, RD_SUCCESS);
    bosch_delay_ms (time_ms);
}


/** @brief @ref rd_sensor_init_fp */
void test_ri_bme280_init_spi (void)
{
    rd_status_t err_code;
    rd_sensor_t environmental_sensor;
    memset (&environmental_sensor, 0, sizeof (rd_sensor_t));
    const uint8_t handle = 1;
    struct bme280_dev expect_dev =
    {
        .dev_id = handle,
        .intf = BME280_SPI_INTF,
        .read = &ri_spi_bme280_read,
        .write = &ri_spi_bme280_write,
        .delay_ms = bosch_delay_ms
    };
    bme280_init_ExpectWithArrayAndReturn (&expect_dev, 1, BME280_OK);
    err_code = bme280_spi_init (&expect_dev, handle);
    TEST_ASSERT (RD_SUCCESS == err_code);
}
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_bme280_uninit (rd_sensor_t *
                              environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_samplerate_get (uint8_t * samplerate);

void test_ri2bme_rate_default (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = RD_SENSOR_CFG_DEFAULT;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_1000_MS);
}


void test_ri2bme_rate_min (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = RD_SENSOR_CFG_MIN;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_1000_MS);
}

void test_ri2bme_rate_max (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = RD_SENSOR_CFG_MAX;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_0_5_MS);
}

void test_ri2bme_rate_no_change (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = RD_SENSOR_CFG_MAX;
    err_code = ri2bme_rate (&test_dev, &cfg);
    cfg = RD_SENSOR_CFG_NO_CHANGE;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_0_5_MS);
}

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