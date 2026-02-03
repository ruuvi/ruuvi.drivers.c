/**
 * @file test_ruuvi_interface_sths34pf80.c
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @author Ruuvi Innovations Ltd
 * @date 2025-12-22
 *
 * Unit tests for STHS34PF80 thermal infrared presence sensor driver.
 */
#ifdef TEST

#include "unity.h"

#include "ruuvi_interface_sths34pf80.h"
#include "ruuvi_driver_error.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_i2c_sths34pf80.h"
#include "mock_ruuvi_interface_yield.h"
#include "mock_sths34pf80_reg.h"

#include <string.h>

static rd_sensor_t m_sensor;
static const rd_bus_t m_bus = RD_BUS_I2C;
static const uint8_t m_handle = 0x5AU;  // Default I2C address

/* Forward declarations for setUp/tearDown helpers */
static void init_sensor_ok (void);
static void uninit_sensor_ok (void);

void setUp (void)
{
    memset (&m_sensor, 0, sizeof (m_sensor));
}

void tearDown (void)
{
    if (NULL != m_sensor.uninit)
    {
        uninit_sensor_ok();
    }
}

/* -------------------------------------------------------------------------- */
/* Helper functions                                                           */
/* -------------------------------------------------------------------------- */

static void expect_whoami_ok (void)
{
    static uint8_t whoami = STHS34PF80_ID;
    sths34pf80_device_id_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_device_id_get_ReturnThruPtr_val (&whoami);
}

static void expect_whoami_wrong (void)
{
    static uint8_t whoami = 0x00U;  // Wrong ID
    sths34pf80_device_id_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_device_id_get_ReturnThruPtr_val (&whoami);
}

static void expect_whoami_fail (void)
{
    sths34pf80_device_id_get_ExpectAnyArgsAndReturn (-1);
}

static void expect_boot_ok (void)
{
    sths34pf80_boot_set_ExpectAnyArgsAndReturn (0);
    ri_delay_ms_ExpectAndReturn (10U, RD_SUCCESS);
}

static void expect_default_config_ok (void)
{
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    sths34pf80_block_data_update_set_ExpectAnyArgsAndReturn (0);
}

static void init_sensor_ok (void)
{
    rd_sensor_initialize_Expect (&m_sensor);
    expect_whoami_ok();
    expect_boot_ok();
    expect_default_config_ok();
    rd_status_t err_code = ri_sths34pf80_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
}

static void uninit_sensor_ok (void)
{
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    rd_sensor_uninitialize_Expect (&m_sensor);
    rd_status_t err_code = ri_sths34pf80_uninit (&m_sensor, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
}

/* -------------------------------------------------------------------------- */
/* Init / Uninit tests                                                        */
/* -------------------------------------------------------------------------- */

void test_ri_sths34pf80_init_ok (void)
{
    const rd_sensor_data_fields_t expected =
    {
        .datas.temperature_c = 1,
        .datas.presence = 1,
        .datas.motion = 1,
        .datas.ir_object = 1
    };
    rd_sensor_initialize_Expect (&m_sensor);
    expect_whoami_ok();
    expect_boot_ok();
    expect_default_config_ok();
    rd_status_t err_code = ri_sths34pf80_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_init, m_sensor.init);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_uninit, m_sensor.uninit);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_samplerate_set, m_sensor.samplerate_set);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_samplerate_get, m_sensor.samplerate_get);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_resolution_set, m_sensor.resolution_set);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_resolution_get, m_sensor.resolution_get);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_scale_set, m_sensor.scale_set);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_scale_get, m_sensor.scale_get);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_mode_set, m_sensor.mode_set);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_mode_get, m_sensor.mode_get);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_dsp_set, m_sensor.dsp_set);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_dsp_get, m_sensor.dsp_get);
    TEST_ASSERT_EQUAL_PTR (&ri_sths34pf80_data_get, m_sensor.data_get);
    TEST_ASSERT_EQUAL_HEX32 (expected.bitfield, m_sensor.provides.bitfield);
}

void test_ri_sths34pf80_init_null (void)
{
    rd_status_t err_code = ri_sths34pf80_init (NULL, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_init_already_init (void)
{
    init_sensor_ok();
    // Second init should fail because internal m_is_init flag is set
    rd_status_t err_code = ri_sths34pf80_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_ERROR_INVALID_STATE, err_code);
}

void test_ri_sths34pf80_init_wrong_bus (void)
{
    rd_status_t err_code = ri_sths34pf80_init (&m_sensor, RD_BUS_SPI, m_handle);
    TEST_ASSERT_EQUAL (RD_ERROR_INVALID_PARAM, err_code);
}

void test_ri_sths34pf80_init_whoami_fail (void)
{
    rd_sensor_initialize_Expect (&m_sensor);
    expect_whoami_fail();
    rd_sensor_uninitialize_Expect (&m_sensor);
    rd_status_t err_code = ri_sths34pf80_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_ERROR_INTERNAL, err_code);
}

void test_ri_sths34pf80_init_whoami_wrong_id (void)
{
    rd_sensor_initialize_Expect (&m_sensor);
    expect_whoami_wrong();
    rd_sensor_uninitialize_Expect (&m_sensor);
    rd_status_t err_code = ri_sths34pf80_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_ERROR_NOT_FOUND, err_code);
}

void test_ri_sths34pf80_uninit_ok (void)
{
    init_sensor_ok();
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    rd_sensor_uninitialize_Expect (&m_sensor);
    rd_status_t err_code = ri_sths34pf80_uninit (&m_sensor, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    // Prevent tearDown from double-uninit
    m_sensor.uninit = NULL;
}

void test_ri_sths34pf80_uninit_null (void)
{
    rd_status_t err_code = ri_sths34pf80_uninit (NULL, m_bus, m_handle);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

/* -------------------------------------------------------------------------- */
/* Samplerate tests                                                           */
/* -------------------------------------------------------------------------- */

void test_ri_sths34pf80_samplerate_set_null (void)
{
    init_sensor_ok();
    rd_status_t err_code = ri_sths34pf80_samplerate_set (NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_samplerate_get_null (void)
{
    init_sensor_ok();
    rd_status_t err_code = ri_sths34pf80_samplerate_get (NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_samplerate_set_1hz (void)
{
    init_sensor_ok();
    uint8_t samplerate = 1U;
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (1U, samplerate);
}

void test_ri_sths34pf80_samplerate_set_default (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_DEFAULT;
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (1U, samplerate);
}

void test_ri_sths34pf80_samplerate_set_max_low_oversampling (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_MAX;
    // With low oversampling (AVG_TMOS_2, 8, or 32), max ODR is 30Hz
    sths34pf80_avg_tobject_num_t avg = STHS34PF80_AVG_TMOS_8;
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (30U, samplerate);
}

void test_ri_sths34pf80_samplerate_set_max_avg128 (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_MAX;
    // With AVG_TMOS_128, max ODR is 8Hz
    sths34pf80_avg_tobject_num_t avg = STHS34PF80_AVG_TMOS_128;
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (8U, samplerate);
}

void test_ri_sths34pf80_samplerate_set_max_avg256 (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_MAX;
    // With AVG_TMOS_256, max ODR is 4Hz
    sths34pf80_avg_tobject_num_t avg = STHS34PF80_AVG_TMOS_256;
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (4U, samplerate);
}

void test_ri_sths34pf80_samplerate_set_max_avg512 (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_MAX;
    // With AVG_TMOS_512, max ODR is 2Hz
    sths34pf80_avg_tobject_num_t avg = STHS34PF80_AVG_TMOS_512;
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (2U, samplerate);
}

void test_ri_sths34pf80_samplerate_set_max_avg1024 (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_MAX;
    // With AVG_TMOS_1024, max ODR is 1Hz
    sths34pf80_avg_tobject_num_t avg = STHS34PF80_AVG_TMOS_1024;
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (1U, samplerate);
}

void test_ri_sths34pf80_samplerate_set_max_avg2048 (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_MAX;
    // With AVG_TMOS_2048, max ODR is 0.5Hz (returned as RI_STHS34PF80_SAMPLERATE_0HZ50)
    sths34pf80_avg_tobject_num_t avg = STHS34PF80_AVG_TMOS_2048;
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RI_STHS34PF80_SAMPLERATE_0HZ50, samplerate);
}

void test_ri_sths34pf80_samplerate_set_max_read_fail (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_MAX;
    // Simulate I2C read failure when getting oversampling
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (RD_ERROR_TIMEOUT);
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_ERROR_TIMEOUT, err_code);
}

void test_ri_sths34pf80_samplerate_get_default (void)
{
    init_sensor_ok();
    uint8_t samplerate = 0U;
    rd_status_t err_code = ri_sths34pf80_samplerate_get (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (1U, samplerate);  // Default is 1 Hz
}

void test_ri_sths34pf80_samplerate_set_3hz_rounds_up_to_4hz (void)
{
    init_sensor_ok();
    uint8_t samplerate = 3U;
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_GREATER_OR_EQUAL (3U, samplerate);  // Must be >= requested
    TEST_ASSERT_EQUAL (4U, samplerate);             // Should round to 4 Hz
}

void test_ri_sths34pf80_samplerate_set_5hz_rounds_up_to_8hz (void)
{
    init_sensor_ok();
    uint8_t samplerate = 5U;
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_GREATER_OR_EQUAL (5U, samplerate);  // Must be >= requested
    TEST_ASSERT_EQUAL (8U, samplerate);             // Should round to 8 Hz
}

void test_ri_sths34pf80_samplerate_set_10hz_rounds_up_to_15hz (void)
{
    init_sensor_ok();
    uint8_t samplerate = 10U;
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_GREATER_OR_EQUAL (10U, samplerate);  // Must be >= requested
    TEST_ASSERT_EQUAL (15U, samplerate);             // Should round to 15 Hz
}

void test_ri_sths34pf80_samplerate_set_20hz_rounds_up_to_30hz (void)
{
    init_sensor_ok();
    uint8_t samplerate = 20U;
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_GREATER_OR_EQUAL (20U, samplerate);  // Must be >= requested
    TEST_ASSERT_EQUAL (30U, samplerate);             // Should round to 30 Hz
}

void test_ri_sths34pf80_samplerate_set_31hz_not_supported (void)
{
    init_sensor_ok();
    uint8_t samplerate = 31U;
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_ERROR_NOT_SUPPORTED, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_ERR_NOT_SUPPORTED, samplerate);
}

/* -------------------------------------------------------------------------- */
/* Resolution/Scale tests (not supported, should return defaults)            */
/* -------------------------------------------------------------------------- */

void test_ri_sths34pf80_resolution_set_null (void)
{
    init_sensor_ok();
    rd_status_t err_code = ri_sths34pf80_resolution_set (NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_resolution_set_default (void)
{
    init_sensor_ok();
    uint8_t resolution = RD_SENSOR_CFG_DEFAULT;
    rd_status_t err_code = ri_sths34pf80_resolution_set (&resolution);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_DEFAULT, resolution);
}

void test_ri_sths34pf80_scale_set_null (void)
{
    init_sensor_ok();
    rd_status_t err_code = ri_sths34pf80_scale_set (NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_scale_set_default (void)
{
    init_sensor_ok();
    uint8_t scale = RD_SENSOR_CFG_DEFAULT;
    rd_status_t err_code = ri_sths34pf80_scale_set (&scale);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_DEFAULT, scale);
}

/* -------------------------------------------------------------------------- */
/* DSP tests                                                                  */
/* -------------------------------------------------------------------------- */

void test_ri_sths34pf80_dsp_set_null (void)
{
    init_sensor_ok();
    uint8_t dsp = RD_SENSOR_CFG_DEFAULT;
    rd_status_t err_code = ri_sths34pf80_dsp_set (NULL, &dsp);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
    err_code = ri_sths34pf80_dsp_set (&dsp, NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_dsp_get_null (void)
{
    init_sensor_ok();
    uint8_t dsp = RD_SENSOR_CFG_DEFAULT;
    rd_status_t err_code = ri_sths34pf80_dsp_get (NULL, &dsp);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
    err_code = ri_sths34pf80_dsp_get (&dsp, NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_dsp_set_default (void)
{
    init_sensor_ok();
    uint8_t dsp = RD_SENSOR_CFG_DEFAULT;
    uint8_t parameter = RD_SENSOR_CFG_DEFAULT;
    rd_status_t err_code = ri_sths34pf80_dsp_set (&dsp, &parameter);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_DEFAULT, dsp);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_DEFAULT, parameter);
}

/* -------------------------------------------------------------------------- */
/* Mode tests                                                                 */
/* -------------------------------------------------------------------------- */

void test_ri_sths34pf80_mode_set_null (void)
{
    init_sensor_ok();
    rd_status_t err_code = ri_sths34pf80_mode_set (NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_mode_get_null (void)
{
    init_sensor_ok();
    rd_status_t err_code = ri_sths34pf80_mode_get (NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

void test_ri_sths34pf80_mode_set_sleep (void)
{
    init_sensor_ok();
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    rd_status_t err_code = ri_sths34pf80_mode_set (&mode);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_SLEEP, mode);
}

void test_ri_sths34pf80_mode_set_continuous (void)
{
    init_sensor_ok();
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    rd_status_t err_code = ri_sths34pf80_mode_set (&mode);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_CONTINUOUS, mode);
}

void test_ri_sths34pf80_mode_set_single (void)
{
    init_sensor_ok();
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    // Set ODR, delay, read sample, then set ODR off
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    ri_delay_ms_ExpectAndReturn (50U, RD_SUCCESS);
    sths34pf80_func_status_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tambient_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tobject_raw_get_ExpectAnyArgsAndReturn (0);
    rd_sensor_timestamp_get_ExpectAndReturn (1000U);
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    rd_status_t err_code = ri_sths34pf80_mode_set (&mode);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_SLEEP, mode);
}

void test_ri_sths34pf80_mode_set_single_while_continuous (void)
{
    init_sensor_ok();
    // First set continuous
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    ri_sths34pf80_mode_set (&mode);
    // Now try single - should fail
    mode = RD_SENSOR_CFG_SINGLE;
    rd_status_t err_code = ri_sths34pf80_mode_set (&mode);
    TEST_ASSERT_EQUAL (RD_ERROR_INVALID_STATE, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_CONTINUOUS, mode);
}

void test_ri_sths34pf80_mode_get_sleep (void)
{
    init_sensor_ok();
    uint8_t mode = 0xFFU;
    rd_status_t err_code = ri_sths34pf80_mode_get (&mode);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_SLEEP, mode);
}

/* -------------------------------------------------------------------------- */
/* Data get tests                                                             */
/* -------------------------------------------------------------------------- */

void test_ri_sths34pf80_data_get_null (void)
{
    init_sensor_ok();
    rd_status_t err_code = ri_sths34pf80_data_get (NULL);
    TEST_ASSERT_EQUAL (RD_ERROR_NULL, err_code);
}

/* Callback to capture rd_sensor_data_populate arguments */
static rd_sensor_data_t m_captured_env_data;
static void capture_populate_args (rd_sensor_data_t * const target,
                                   const rd_sensor_data_t * const provided,
                                   const rd_sensor_data_fields_t requested,
                                   int cmock_num_calls)
{
    (void) target;
    (void) requested;
    (void) cmock_num_calls;
    memcpy (&m_captured_env_data, provided, sizeof (rd_sensor_data_t));
}

void test_ri_sths34pf80_data_get_no_sample (void)
{
    init_sensor_ok();
    rd_sensor_data_t data = {0};
    float values[4] = {0};
    data.data = values;
    data.fields.datas.temperature_c = 1;
    data.fields.datas.presence = 1;
    data.fields.datas.motion = 1;
    data.fields.datas.ir_object = 1;
    memset (&m_captured_env_data, 0, sizeof (m_captured_env_data));
    rd_sensor_data_populate_AddCallback (capture_populate_args);
    rd_sensor_data_populate_ExpectAnyArgs();
    rd_status_t err_code = ri_sths34pf80_data_get (&data);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    // Before any sample is taken, valid fields must be empty
    TEST_ASSERT_EQUAL_HEX32 (0, m_captured_env_data.valid.bitfield);
    TEST_ASSERT_EQUAL_UINT64 (RD_UINT64_INVALID, m_captured_env_data.timestamp_ms);
}

void test_ri_sths34pf80_data_get_continuous (void)
{
    init_sensor_ok();
    // Set continuous mode
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    ri_sths34pf80_mode_set (&mode);
    // Get data - should check DRDY and read fresh sample if ready
    rd_sensor_data_t data = {0};
    float values[4] = {0};
    data.data = values;
    data.fields.datas.temperature_c = 1;
    data.fields.datas.presence = 1;
    data.fields.datas.motion = 1;
    data.fields.datas.ir_object = 1;
    // Expect DRDY check - return data ready
    static sths34pf80_drdy_status_t drdy_status = {.drdy = 1};
    sths34pf80_drdy_status_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_drdy_status_get_ReturnThruPtr_val (&drdy_status);
    sths34pf80_func_status_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tambient_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tobject_raw_get_ExpectAnyArgsAndReturn (0);
    rd_sensor_timestamp_get_ExpectAndReturn (2000U);
    rd_sensor_data_populate_ExpectAnyArgs();
    rd_status_t err_code = ri_sths34pf80_data_get (&data);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
}

#endif // TEST
