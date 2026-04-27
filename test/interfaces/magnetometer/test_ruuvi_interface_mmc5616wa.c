#include "unity.h"

TEST_SOURCE_FILE ("src/ruuvi_driver_sensor.c")

#include "ruuvi_interface_mmc5616wa.h"
#include "mock_mmc5616wa.h"
#include "mock_ruuvi_interface_i2c_mmc5616wa.h"
#include "mock_ruuvi_interface_yield.h"

#include <string.h>

#define MOCK_DEV_ID (0x30U)
#define MOCK_TIME_MS (1234U)

static rd_sensor_t m_sensor;

static uint64_t mock_timestamp_get (void)
{
    return MOCK_TIME_MS;
}


static void default_driver_config_expect (void)
{
    mmc5616wa_default_config_ExpectAnyArgs();
    mmc5616wa_default_fifo_config_ExpectAnyArgs();
}

static void init_ok_expect (void)
{
    bool passed = true;
    default_driver_config_expect();
    mmc5616wa_init_ExpectAndReturn (&dev.ctx, MMC5616WA_OK);
    mmc5616wa_self_test_ExpectAnyArgsAndReturn (MMC5616WA_OK);
    mmc5616wa_self_test_ReturnThruPtr_passed (&passed);
}

void setUp (void)
{
    memset (&m_sensor, 0, sizeof (m_sensor));
    memset (&dev, 0, sizeof (dev));
    (void) rd_sensor_timestamp_function_set (&mock_timestamp_get);
}

void tearDown (void)
{
    (void) rd_sensor_timestamp_function_set (NULL);
}

void test_ri_mmc5616wa_init_ok (void)
{
    init_ok_expect();
    TEST_ASSERT_EQUAL (RD_SUCCESS,
                       ri_mmc5616wa_init (&m_sensor, RD_BUS_I2C, MOCK_DEV_ID));
    TEST_ASSERT_EQUAL_STRING ("MMC5616", m_sensor.name);
    TEST_ASSERT_EQUAL_PTR (&dev.ctx, m_sensor.p_ctx);
    TEST_ASSERT_EQUAL_PTR (&ri_mmc5616wa_data_get, m_sensor.data_get);
    TEST_ASSERT_TRUE (m_sensor.provides.datas.magnetometer_x_g);
    TEST_ASSERT_TRUE (m_sensor.provides.datas.magnetometer_y_g);
    TEST_ASSERT_TRUE (m_sensor.provides.datas.magnetometer_z_g);
    TEST_ASSERT_TRUE (m_sensor.provides.datas.temperature_c);
}

void test_ri_mmc5616wa_init_wrong_bus (void)
{
    TEST_ASSERT_EQUAL (RD_ERROR_NOT_SUPPORTED,
                       ri_mmc5616wa_init (&m_sensor, RD_BUS_SPI, MOCK_DEV_ID));
    TEST_ASSERT_FALSE (rd_sensor_is_init (&m_sensor));
}

void test_ri_mmc5616wa_init_selftest_fail (void)
{
    bool passed = false;
    default_driver_config_expect();
    mmc5616wa_init_ExpectAndReturn (&dev.ctx, MMC5616WA_OK);
    mmc5616wa_self_test_ExpectAnyArgsAndReturn (MMC5616WA_OK);
    mmc5616wa_self_test_ReturnThruPtr_passed (&passed);
    TEST_ASSERT_EQUAL (RD_ERROR_SELFTEST,
                       ri_mmc5616wa_init (&m_sensor, RD_BUS_I2C, MOCK_DEV_ID));
    TEST_ASSERT_FALSE (rd_sensor_is_init (&m_sensor));
}

void test_ri_mmc5616wa_samplerate_set_rounds_and_updates_continuous (void)
{
    uint8_t samplerate = 25U;
    dev.mode = RD_SENSOR_CFG_CONTINUOUS;
    mmc5616wa_config_set_ExpectAnyArgsAndReturn (MMC5616WA_OK);
    TEST_ASSERT_EQUAL (RD_SUCCESS, ri_mmc5616wa_samplerate_set (&samplerate));
    TEST_ASSERT_EQUAL_UINT8 (25U, samplerate);
    TEST_ASSERT_EQUAL_UINT8 (25U, dev.config.odr);
}

void test_ri_mmc5616wa_samplerate_set_rejects_too_high (void)
{
    uint8_t samplerate = RI_MMC5616WA_MAX_SAMPLERATE + 1U;
    TEST_ASSERT_EQUAL (RD_ERROR_NOT_SUPPORTED,
                       ri_mmc5616wa_samplerate_set (&samplerate));
    TEST_ASSERT_EQUAL_UINT8 (RD_SENSOR_ERR_NOT_SUPPORTED, samplerate);
}

void test_ri_mmc5616wa_resolution_and_scale_are_fixed (void)
{
    uint8_t resolution = RD_SENSOR_CFG_DEFAULT;
    uint8_t scale = RD_SENSOR_CFG_MAX;
    dev.mode = RD_SENSOR_CFG_SLEEP;
    TEST_ASSERT_EQUAL (RD_SUCCESS, ri_mmc5616wa_resolution_set (&resolution));
    TEST_ASSERT_EQUAL_UINT8 (RI_MMC5616WA_RESOLUTION_BITS, resolution);
    TEST_ASSERT_EQUAL (RD_SUCCESS, ri_mmc5616wa_scale_set (&scale));
    TEST_ASSERT_EQUAL_UINT8 (RI_MMC5616WA_SCALE_GAUSS, scale);
}

void test_ri_mmc5616wa_mode_single_samples_and_data_get_populates (void)
{
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    mmc5616wa_mag_raw_t raw =
    {
        .x = 524288,
        .y = 540672,
        .z = 507904
    };
    uint8_t raw_temperature = 125U;
    float data_values[4] = {0};
    rd_sensor_data_t data =
    {
        .timestamp_ms = RD_UINT64_INVALID,
        .fields =
        {
            .datas.magnetometer_x_g = 1,
            .datas.magnetometer_y_g = 1,
            .datas.magnetometer_z_g = 1,
            .datas.temperature_c = 1
        },
        .valid.bitfield = 0,
        .data = data_values
    };
    mmc5616wa_config_set_ExpectAnyArgsAndReturn (MMC5616WA_OK);
    mmc5616wa_magnetic_measurement_get_ExpectAnyArgsAndReturn (MMC5616WA_OK);
    mmc5616wa_magnetic_measurement_get_ReturnThruPtr_sample (&raw);
    mmc5616wa_temperature_measurement_get_ExpectAnyArgsAndReturn (MMC5616WA_OK);
    mmc5616wa_temperature_measurement_get_ReturnThruPtr_raw_temperature (&raw_temperature);
    mmc5616wa_magnetic_20bit_to_gauss_ExpectAndReturn (raw.x, 0.0f);
    mmc5616wa_magnetic_20bit_to_gauss_ExpectAndReturn (raw.y, 1.0f);
    mmc5616wa_magnetic_20bit_to_gauss_ExpectAndReturn (raw.z, -1.0f);
    mmc5616wa_temperature_to_celsius_ExpectAndReturn (raw_temperature, 25.0f);
    TEST_ASSERT_EQUAL (RD_SUCCESS, ri_mmc5616wa_mode_set (&mode));
    TEST_ASSERT_EQUAL_UINT8 (RD_SENSOR_CFG_SLEEP, mode);
    TEST_ASSERT_EQUAL (RD_SUCCESS, ri_mmc5616wa_data_get (&data));
    TEST_ASSERT_EQUAL_UINT64 (MOCK_TIME_MS, data.timestamp_ms);
    TEST_ASSERT_EQUAL_FLOAT (0.0f, data_values[0]);
    TEST_ASSERT_EQUAL_FLOAT (1.0f, data_values[1]);
    TEST_ASSERT_EQUAL_FLOAT (-1.0f, data_values[2]);
    TEST_ASSERT_EQUAL_FLOAT (25.0f, data_values[3]);
    TEST_ASSERT_EQUAL_UINT32 (data.fields.bitfield, data.valid.bitfield);
}

void test_ri_mmc5616wa_data_get_without_sample_returns_invalid_state (void)
{
    float values[3] = {0};
    rd_sensor_data_t data =
    {
        .timestamp_ms = RD_UINT64_INVALID,
        .fields = {.datas.magnetometer_x_g = 1},
        .valid.bitfield = 0,
        .data = values
    };
    dev.mode = RD_SENSOR_CFG_SLEEP;
    dev.tsample = RD_UINT64_INVALID;
    TEST_ASSERT_EQUAL (RD_ERROR_INVALID_STATE, ri_mmc5616wa_data_get (&data));
}

void test_ri_mmc5616wa_fifo_use_sets_fifo_config (void)
{
    mmc5616wa_fifo_config_set_ExpectAnyArgsAndReturn (MMC5616WA_OK);
    TEST_ASSERT_EQUAL (RD_SUCCESS, ri_mmc5616wa_fifo_use (true));
    TEST_ASSERT_TRUE (dev.fifo_config.enable);
    TEST_ASSERT_TRUE (dev.fifo_config.address_loop_enable);
}