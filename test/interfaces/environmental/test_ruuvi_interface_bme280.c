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
rd_sensor_t environmental_sensor =
{
    .init              = ri_bme280_init,
    .uninit            = ri_bme280_uninit,
    .samplerate_set    = ri_bme280_samplerate_set,
    .samplerate_get    = ri_bme280_samplerate_get,
    .resolution_set    = ri_bme280_resolution_set,
    .resolution_get    = ri_bme280_resolution_get,
    .scale_set         = ri_bme280_scale_set,
    .scale_get         = ri_bme280_scale_get,
    .dsp_set           = ri_bme280_dsp_set,
    .dsp_get           = ri_bme280_dsp_get,
    .mode_set          = ri_bme280_mode_set,
    .mode_get          = ri_bme280_mode_get,
    .data_get          = ri_bme280_data_get,
    .configuration_set = rd_sensor_configuration_set,
    .configuration_get = rd_sensor_configuration_get,
    .provides.datas.temperature_c = 1,
    .provides.datas.humidity_rh = 1,
    .provides.datas.pressure_pa = 1
};

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

void test_ri2bme_rate_1000ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = 1U;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_1000_MS);
}

void test_ri2bme_rate_500ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = 2U;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_500_MS);
}

void test_ri2bme_rate_125ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = 8U;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_125_MS);
}

void test_ri2bme_rate_62_5ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = 16U;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_62_5_MS);
}

void test_ri2bme_rate_20ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = 50U;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_20_MS);
}

void test_ri2bme_rate_10ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = 100U;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_10_MS);
}

void test_ri2bme_rate_0_5ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = 200U;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (test_dev.settings.standby_time == BME280_STANDBY_TIME_0_5_MS);
}

void test_ri2bme_rate_incorrect (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct bme280_dev test_dev = {0};
    uint8_t cfg = 210U;
    err_code = ri2bme_rate (&test_dev, &cfg);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
    TEST_ASSERT (cfg == RD_SENSOR_ERR_NOT_SUPPORTED);
}

void test_ri_bme280_init_dev_write (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct rd_sensor_t * p_environmental_sensor = &environmental_sensor;
    rd_bus_t bus = RD_BUS_SPI;
    dev.write = &ri_spi_bme280_write;
    uint8_t handle = 0;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (true);
    err_code = ri_bme280_init (p_environmental_sensor, bus, handle);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_ri_bme280_init_en_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct rd_sensor_t * p_environmental_sensor = NULL;
    rd_bus_t bus = RD_BUS_SPI;
    uint8_t handle = 0;
    err_code = ri_bme280_init (p_environmental_sensor, bus, handle);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_init_invalid_bus (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct rd_sensor_t * p_environmental_sensor = &environmental_sensor;
    rd_bus_t bus = RD_BUS_UART;
    uint8_t handle = 0;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (p_environmental_sensor);
    err_code = ri_bme280_init (p_environmental_sensor, bus, handle);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
}


void test_ri_bme280_init_spi_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct rd_sensor_t n_environmental_sensor;
    rd_bus_t bus = RD_BUS_SPI;
    uint8_t handle = 1;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (&n_environmental_sensor);
    bme280_init_IgnoreAndReturn (RD_SUCCESS);
    bme280_soft_reset_IgnoreAndReturn (BME280_OK);
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_init (&n_environmental_sensor, bus, handle);
    TEST_ASSERT (1 == n_environmental_sensor.provides.datas.pressure_pa);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_init_i2c_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct rd_sensor_t n_environmental_sensor;
    rd_bus_t bus = RD_BUS_I2C;
    uint8_t handle = 1;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (&n_environmental_sensor);
    bme280_init_IgnoreAndReturn (RD_SUCCESS);
    bme280_soft_reset_IgnoreAndReturn (BME280_OK);
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_init (&n_environmental_sensor, bus, handle);
    TEST_ASSERT (1 == n_environmental_sensor.provides.datas.pressure_pa);
    TEST_ASSERT (RD_SUCCESS == err_code);
}


void test_ri_bme280_uninit_sensor_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct rd_sensor_t * p_environmental_sensor = NULL;
    rd_bus_t bus = RD_BUS_SPI;
    uint8_t handle = 1;
    err_code = ri_bme280_uninit (p_environmental_sensor, bus, handle);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_uninit_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    struct rd_sensor_t * p_environmental_sensor = &environmental_sensor;
    rd_bus_t bus = RD_BUS_SPI;
    uint8_t handle = 1;
    struct bme280_dev expect_dev;
    memset (&expect_dev, 0, sizeof (expect_dev));
    bme280_soft_reset_ExpectWithArrayAndReturn (&expect_dev, 1, BME280_OK);
    rd_sensor_uninitialize_Expect (p_environmental_sensor);
    err_code = ri_bme280_uninit (p_environmental_sensor, bus, handle);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_samplerate = NULL;
    err_code = ri_bme280_samplerate_set (p_samplerate);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_samplerate_set_sleep_invalid (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate = 200U;
    uint8_t bme_mode = BME280_FORCED_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_samplerate_set (&samplerate);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_ri_bme280_samplerate_set_sleep (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate = 200U;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_set (&samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_samplerate = NULL;
    err_code = ri_bme280_samplerate_get (p_samplerate);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_samplerate_get_1000ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate;
    dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_get (&samplerate);
    TEST_ASSERT (1U == samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_get_500ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate;
    dev.settings.standby_time = BME280_STANDBY_TIME_500_MS;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_get (&samplerate);
    TEST_ASSERT (2U == samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_get_125ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate;
    dev.settings.standby_time = BME280_STANDBY_TIME_125_MS;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_get (&samplerate);
    TEST_ASSERT (8U == samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_get_62_5ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate;
    dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_get (&samplerate);
    TEST_ASSERT (16U == samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_get_20ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate;
    dev.settings.standby_time = BME280_STANDBY_TIME_20_MS;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_get (&samplerate);
    TEST_ASSERT (50U == samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_get_10ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate;
    dev.settings.standby_time = BME280_STANDBY_TIME_10_MS;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_get (&samplerate);
    TEST_ASSERT (100U == samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_get_0_5ms (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate;
    dev.settings.standby_time = BME280_STANDBY_TIME_0_5_MS;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_get (&samplerate);
    TEST_ASSERT (200U == samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_samplerate_get_invalid (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t samplerate;
    dev.settings.standby_time = 0x10;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (BME280_OK);
    err_code = ri_bme280_samplerate_get (&samplerate);
    TEST_ASSERT (RD_SENSOR_ERR_NOT_SUPPORTED == samplerate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_resolution_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_resolution = NULL;
    err_code = ri_bme280_resolution_set (p_resolution);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_resolution_set_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t resolution = RD_SENSOR_CFG_NO_CHANGE;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_resolution_set (&resolution);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_resolution_set_invalid (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t resolution = RD_SENSOR_CFG_CUSTOM_2;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_resolution_set (&resolution);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
}

void test_ri_bme280_resolution_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_resolution = NULL;
    err_code = ri_bme280_resolution_get (p_resolution);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_resolution_get_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t resolution = RD_SENSOR_CFG_CUSTOM_2;
    err_code = ri_bme280_resolution_get (&resolution);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_scale_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_scale = NULL;
    err_code = ri_bme280_scale_set (p_scale);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_scale_set_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t scale = RD_SENSOR_CFG_NO_CHANGE;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_resolution_set (&scale);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == scale);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_scale_set_invalid (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t scale = RD_SENSOR_CFG_CUSTOM_2;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_resolution_set (&scale);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == scale);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
}

void test_ri_bme280_scale_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_scale = NULL;
    err_code = ri_bme280_resolution_get (p_scale);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_scale_get_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t scale = RD_SENSOR_CFG_CUSTOM_2;
    err_code = ri_bme280_resolution_get (&scale);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == scale);
    TEST_ASSERT (RD_SUCCESS == err_code);
}


void test_ri_bme280_mode_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_mode = NULL;
    err_code = ri_bme280_mode_get (p_mode);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_mode_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_mode = NULL;
    err_code = ri_bme280_mode_set (p_mode);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_mode_set_mode_sleep_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    struct bme280_dev expect_dev;
    memset (&expect_dev, 0, sizeof (expect_dev));
    bme280_set_sensor_mode_ExpectWithArrayAndReturn (BME280_SLEEP_MODE, &expect_dev, 1,
            BME280_OK);
    err_code = ri_bme280_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_mode_get_mode_sleep_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    struct bme280_dev expect_dev;
    memset (&expect_dev, 0, sizeof (expect_dev));
    bme280_get_sensor_mode_ExpectWithArrayAndReturn (&bme_mode, 1, &expect_dev, 1, BME280_OK);
    err_code = ri_bme280_mode_get (&mode);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_mode_set_mode_single_sleep (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    struct bme280_dev expect_dev;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    memset (&expect_dev, 0, sizeof (expect_dev));
    bme280_set_sensor_mode_ExpectWithArrayAndReturn (BME280_FORCED_MODE, &expect_dev, 1,
            BME280_OK);
    ri_delay_ms_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_sensor_timestamp_get_IgnoreAndReturn (RD_SUCCESS);
    err_code = ri_bme280_mode_set (&mode);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_mode_set_mode_single_cont (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    struct bme280_dev expect_dev;
    uint8_t bme_mode = BME280_NORMAL_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_mode_set (&mode);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_ri_bme280_mode_get_mode_forced_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode;
    uint8_t bme_mode = BME280_FORCED_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_mode_get (&mode);
    TEST_ASSERT (RD_SENSOR_CFG_SINGLE == mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_mode_set_mode_normal_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    struct bme280_dev expect_dev;
    memset (&expect_dev, 0, sizeof (expect_dev));
    bme280_set_sensor_mode_ExpectWithArrayAndReturn (BME280_NORMAL_MODE, &expect_dev, 1,
            BME280_OK);
    err_code = ri_bme280_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_mode_set_invalid (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = 0x02;
    err_code = ri_bme280_mode_set (&mode);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
}

void test_ri_bme280_mode_get_mode_normal_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode;
    uint8_t bme_mode = BME280_NORMAL_MODE;
    test_ri_bme280_mode_set_mode_normal_ok();
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_mode_get (&mode);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_mode_get_mode_default_invalid (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode;
    uint8_t bme_mode = 0x02;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_mode_get (&mode);
    TEST_ASSERT (RD_SENSOR_ERR_INVALID == mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_set_null_dsp (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_dsp = NULL;
    uint8_t parameter;
    err_code = ri_bme280_dsp_set (p_dsp, &parameter);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_dsp_set_null_param (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t * p_parameter = NULL;
    err_code = ri_bme280_dsp_set (&dsp, p_parameter);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_dsp_set_not_supported (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter = RD_SENSOR_CFG_CUSTOM_2;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
}


void test_ri_bme280_dsp_set_not_supported_2 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 3;
    uint8_t parameter = 4U;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
}

void test_ri_bme280_dsp_set_ok_1 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0x0a;
    uint8_t parameter = 1U;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (parameter == 1U);
    TEST_ASSERT (dev.settings.filter == 0x00);
    TEST_ASSERT (dev.settings.osr_h == 0x01);
    TEST_ASSERT (dev.settings.osr_p == 0x01);
    TEST_ASSERT (dev.settings.osr_t == 0x01);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_set_ok_2 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0x0a;
    uint8_t parameter = RD_SENSOR_CFG_MIN;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (parameter == 1U);
    TEST_ASSERT (dev.settings.filter == 0x00);
    TEST_ASSERT (dev.settings.osr_h == 0x01);
    TEST_ASSERT (dev.settings.osr_p == 0x01);
    TEST_ASSERT (dev.settings.osr_t == 0x01);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_set_ok_3 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0x0a;
    uint8_t parameter = RD_SENSOR_CFG_DEFAULT;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (parameter == 1U);
    TEST_ASSERT (dev.settings.filter == 0x00);
    TEST_ASSERT (dev.settings.osr_h == 0x01);
    TEST_ASSERT (dev.settings.osr_p == 0x01);
    TEST_ASSERT (dev.settings.osr_t == 0x01);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_set_ok_4 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0x0a;
    uint8_t parameter = 2U;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (parameter == 2U);
    TEST_ASSERT (dev.settings.filter == 0x01);
    TEST_ASSERT (dev.settings.osr_h == 0x02);
    TEST_ASSERT (dev.settings.osr_p == 0x02);
    TEST_ASSERT (dev.settings.osr_t == 0x02);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_set_ok_5 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0x0a;
    uint8_t parameter = 4U;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (parameter == 4U);
    TEST_ASSERT (dev.settings.filter == 0x02);
    TEST_ASSERT (dev.settings.osr_h == 0x03);
    TEST_ASSERT (dev.settings.osr_p == 0x03);
    TEST_ASSERT (dev.settings.osr_t == 0x03);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_set_ok_6 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0x0a;
    uint8_t parameter = 8U;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (parameter == 8U);
    TEST_ASSERT (dev.settings.filter == 0x03);
    TEST_ASSERT (dev.settings.osr_h == 0x04);
    TEST_ASSERT (dev.settings.osr_p == 0x04);
    TEST_ASSERT (dev.settings.osr_t == 0x04);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_set_ok_7 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0x0a;
    uint8_t parameter = 16U;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (parameter == 16U);
    TEST_ASSERT (dev.settings.filter == 0x04);
    TEST_ASSERT (dev.settings.osr_h == 0x05);
    TEST_ASSERT (dev.settings.osr_p == 0x05);
    TEST_ASSERT (dev.settings.osr_t == 0x05);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_set_ok_8 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0x0a;
    uint8_t parameter = RD_SENSOR_CFG_MAX;
    uint8_t bme_mode = BME280_SLEEP_MODE;
    bme280_get_sensor_mode_ExpectAnyArgsAndReturn (BME280_OK);
    bme280_get_sensor_mode_ReturnThruPtr_sensor_mode (&bme_mode);
    bme280_set_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = ri_bme280_dsp_set (&dsp, &parameter);
    TEST_ASSERT (parameter == 16U);
    TEST_ASSERT (dev.settings.filter == 0x04);
    TEST_ASSERT (dev.settings.osr_h == 0x05);
    TEST_ASSERT (dev.settings.osr_p == 0x05);
    TEST_ASSERT (dev.settings.osr_t == 0x05);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_null_param (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t * p_parameter = NULL;
    err_code = ri_bme280_dsp_get (&dsp, p_parameter);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_dsp_get_null_dsp (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t * p_dsp = NULL;
    uint8_t parameter;
    err_code = ri_bme280_dsp_get (p_dsp, &parameter);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_bme280_dsp_get_ok_1 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 1;
    dev.settings.osr_h = 0;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 2U);
    TEST_ASSERT (dsp == 0x02);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_ok_2 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 2;
    dev.settings.osr_h = 0;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 4U);
    TEST_ASSERT (dsp == 0x02);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_ok_3 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 3;
    dev.settings.osr_h = 0;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 8U);
    TEST_ASSERT (dsp == 0x02);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_ok_4 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 4;
    dev.settings.osr_h = 0;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 16U);
    TEST_ASSERT (dsp == 0x02);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_ok_5 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 5;
    dev.settings.osr_h = 0;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 2U);
    TEST_ASSERT (dsp == 0x02);
    TEST_ASSERT (RD_SUCCESS == err_code);
}


void test_ri_bme280_dsp_get_ok_6 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 0;
    dev.settings.osr_h = 2;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 2U);
    TEST_ASSERT (dsp == 0x08);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_ok_7 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 0;
    dev.settings.osr_h = 3;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 4U);
    TEST_ASSERT (dsp == 0x08);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_ok_8 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 0;
    dev.settings.osr_h = 4;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 8U);
    TEST_ASSERT (dsp == 0x08);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_ok_9 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 0;
    dev.settings.osr_h = 5;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 16U);
    TEST_ASSERT (dsp == 0x08);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_dsp_get_ok_10 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp;
    uint8_t parameter;
    bme280_get_sensor_settings_ExpectAnyArgsAndReturn (RD_SUCCESS);
    dev.settings.filter = 1;
    dev.settings.osr_h = 6;
    err_code = ri_bme280_dsp_get (&dsp, &parameter);
    TEST_ASSERT (parameter == 2U);
    TEST_ASSERT (dsp == 0x0a);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_bme280_data_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t * p_data = NULL;
    err_code = ri_bme280_data_get (p_data);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
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
