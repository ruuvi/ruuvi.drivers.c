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

// Import debug flag from source file
#ifndef SHTS_DEBUG_DATA_IN_ACCELERATION
#define SHTS_DEBUG_DATA_IN_ACCELERATION (1U)
#endif

// Import data field index constants from source
#if SHTS_DEBUG_DATA_IN_ACCELERATION
#define STHS34PF80_DEBUG_TOBJECT         (0)
#define STHS34PF80_DEBUG_TMOTION         (1)
#define STHS34PF80_DEBUG_TPRESENCE       (2)
#define STHS34PF80_TAMBIENT_C            (3)
#define STHS34PF80_PRESENCE_FLAG         (4)
#define STHS34PF80_MOTION_FLAG           (5)
#define STHS34PF80_TOBJECT_RAW           (6)
#else
#define STHS34PF80_TAMBIENT_C            (0)
#define STHS34PF80_PRESENCE_FLAG         (1)
#define STHS34PF80_MOTION_FLAG           (2)
#define STHS34PF80_TOBJECT_RAW           (3)
#endif

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

static void expect_avg_get_low_oversampling (void)
{
    static sths34pf80_avg_tobject_num_t avg = STHS34PF80_AVG_TMOS_8;
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
}

static void expect_sample_read (void)
{
    sths34pf80_func_status_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tambient_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tobject_raw_get_ExpectAnyArgsAndReturn (0);
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    sths34pf80_tpresence_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tmotion_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tamb_shock_raw_get_ExpectAnyArgsAndReturn (0);
#endif
    rd_sensor_timestamp_get_ExpectAndReturn (1000U);
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
    rd_sensor_data_fields_t expected = {0};
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    expected.datas.acceleration_x_g = 1;
    expected.datas.acceleration_y_g = 1;
    expected.datas.acceleration_z_g = 1;
#endif
    expected.datas.temperature_c = 1;
    expected.datas.presence = 1;
    expected.datas.motion = 1;
    expected.datas.ir_object = 1;
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
    expect_avg_get_low_oversampling();
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (1U, samplerate);
}

void test_ri_sths34pf80_samplerate_set_default (void)
{
    init_sensor_ok();
    uint8_t samplerate = RD_SENSOR_CFG_DEFAULT;
    expect_avg_get_low_oversampling();
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
    expect_avg_get_low_oversampling();
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_GREATER_OR_EQUAL (3U, samplerate);  // Must be >= requested
    TEST_ASSERT_EQUAL (4U, samplerate);             // Should round to 4 Hz
}

void test_ri_sths34pf80_samplerate_set_5hz_rounds_up_to_8hz (void)
{
    init_sensor_ok();
    uint8_t samplerate = 5U;
    expect_avg_get_low_oversampling();
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_GREATER_OR_EQUAL (5U, samplerate);  // Must be >= requested
    TEST_ASSERT_EQUAL (8U, samplerate);             // Should round to 8 Hz
}

void test_ri_sths34pf80_samplerate_set_10hz_rounds_up_to_15hz (void)
{
    init_sensor_ok();
    uint8_t samplerate = 10U;
    expect_avg_get_low_oversampling();
    rd_status_t err_code = ri_sths34pf80_samplerate_set (&samplerate);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_GREATER_OR_EQUAL (10U, samplerate);  // Must be >= requested
    TEST_ASSERT_EQUAL (15U, samplerate);             // Should round to 15 Hz
}

void test_ri_sths34pf80_samplerate_set_20hz_rounds_up_to_30hz (void)
{
    init_sensor_ok();
    uint8_t samplerate = 20U;
    expect_avg_get_low_oversampling();
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
    // Continuous mode with ODR_OFF triggers samplerate_set(DEFAULT), which caps based on avg
    expect_avg_get_low_oversampling();
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    rd_status_t err_code = ri_sths34pf80_mode_set (&mode);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_CONTINUOUS, mode);
}

void test_ri_sths34pf80_mode_set_single (void)
{
    init_sensor_ok();
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    // Single mode calls samplerate_set(MAX) which reads oversampling twice (MAX calc + cap)
    sths34pf80_avg_tobject_num_t avg = STHS34PF80_AVG_TMOS_8;
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
    sths34pf80_avg_tobject_num_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_avg_tobject_num_get_ReturnThruPtr_val (&avg);
    // Set ODR to max (30 Hz with low oversampling)
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    // Delay = (1000/30) + 10 = 43 ms
    ri_delay_ms_ExpectAndReturn (43U, RD_SUCCESS);
    // Read sample
    expect_sample_read();
    // Set ODR off
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    rd_status_t err_code = ri_sths34pf80_mode_set (&mode);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    TEST_ASSERT_EQUAL (RD_SENSOR_CFG_SLEEP, mode);
}

void test_ri_sths34pf80_mode_set_single_while_continuous (void)
{
    init_sensor_ok();
    // First set continuous - needs avg mock for samplerate_set(DEFAULT)
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    expect_avg_get_low_oversampling();
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
static float m_captured_data_array[7];  // Space for all fields including debug
static void capture_populate_args (rd_sensor_data_t * const target,
                                   const rd_sensor_data_t * const provided,
                                   const rd_sensor_data_fields_t requested,
                                   int cmock_num_calls)
{
    (void) target;
    (void) requested;
    (void) cmock_num_calls;
    // Copy the struct
    memcpy (&m_captured_env_data, provided, sizeof (rd_sensor_data_t));

    // Copy the data array before it goes out of scope
    if (provided->data != NULL)
    {
        memcpy (m_captured_data_array, provided->data, sizeof (m_captured_data_array));
        m_captured_env_data.data = m_captured_data_array;
    }
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
    // Set continuous mode - needs avg mock for samplerate_set(DEFAULT)
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    expect_avg_get_low_oversampling();
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
    expect_sample_read();
    rd_sensor_data_populate_ExpectAnyArgs();
    rd_status_t err_code = ri_sths34pf80_data_get (&data);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
}

/**
 * @brief Test that data fields are placed in correct array indices
 *
 * This test validates that the data returned by data_get places values
 * at the correct indices matching the STHS34PF80_* constants.
 */
void test_ri_sths34pf80_data_indices_correct (void)
{
    init_sensor_ok();
    // Set some known raw values in the sensor context
    // We'll trigger a sample read by setting mode to continuous and having DRDY set
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    expect_avg_get_low_oversampling();
    sths34pf80_odr_set_ExpectAnyArgsAndReturn (0);
    ri_sths34pf80_mode_set (&mode);
    // Set up test data
    const int16_t test_tambient = 2500;  // 25.00°C
    const int16_t test_tobject = 1234;
    const uint8_t test_presence = 1;
    const uint8_t test_motion = 1;
    // Prepare data get with DRDY and sample read
    rd_sensor_data_t data = {0};
    float values[7] = {0};  // 4 base + 3 debug fields
    data.data = values;
    data.fields.datas.temperature_c = 1;
    data.fields.datas.presence = 1;
    data.fields.datas.motion = 1;
    data.fields.datas.ir_object = 1;
    // Mock the DRDY check and sample read
    static sths34pf80_drdy_status_t drdy_status = {.drdy = 1};
    sths34pf80_drdy_status_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_drdy_status_get_ReturnThruPtr_val (&drdy_status);
    // Mock func_status with test flags
    static sths34pf80_func_status_t func_status = {.pres_flag = test_presence, .mot_flag = test_motion};
    sths34pf80_func_status_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_func_status_get_ReturnThruPtr_val (&func_status);
    // Mock temperature and object reads
    static int16_t tambient_ret = test_tambient;
    static int16_t tobject_ret = test_tobject;
    sths34pf80_tambient_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tambient_raw_get_ReturnThruPtr_val (&tambient_ret);
    sths34pf80_tobject_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tobject_raw_get_ReturnThruPtr_val (&tobject_ret);
    // Mock debug reads if enabled
    const int16_t test_tpresence = 567;
    const int16_t test_tmotion = 890;
    const int16_t test_tamb_shock = 111;
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    static int16_t tpresence_ret = test_tpresence;
    static int16_t tmotion_ret = test_tmotion;
    static int16_t tamb_shock_ret = test_tamb_shock;
    sths34pf80_tpresence_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tpresence_raw_get_ReturnThruPtr_val (&tpresence_ret);
    sths34pf80_tmotion_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tmotion_raw_get_ReturnThruPtr_val (&tmotion_ret);
    sths34pf80_tamb_shock_raw_get_ExpectAnyArgsAndReturn (0);
    sths34pf80_tamb_shock_raw_get_ReturnThruPtr_val (&tamb_shock_ret);
#endif
    rd_sensor_timestamp_get_ExpectAndReturn (3000U);
    // Capture the populated data
    memset (&m_captured_env_data, 0, sizeof (m_captured_env_data));
    rd_sensor_data_populate_AddCallback (capture_populate_args);
    rd_sensor_data_populate_ExpectAnyArgs();
    rd_status_t err_code = ri_sths34pf80_data_get (&data);
    TEST_ASSERT_EQUAL (RD_SUCCESS, err_code);
    // Verify data is at correct indices using the STHS34PF80_* constants
    // These indices must match the constants defined in the source file
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    TEST_ASSERT_EQUAL (0, STHS34PF80_DEBUG_TOBJECT);
    TEST_ASSERT_EQUAL (1, STHS34PF80_DEBUG_TMOTION);
    TEST_ASSERT_EQUAL (2, STHS34PF80_DEBUG_TPRESENCE);
    TEST_ASSERT_EQUAL (3, STHS34PF80_TAMBIENT_C);
    TEST_ASSERT_EQUAL (4, STHS34PF80_PRESENCE_FLAG);
    TEST_ASSERT_EQUAL (5, STHS34PF80_MOTION_FLAG);
    TEST_ASSERT_EQUAL (6, STHS34PF80_TOBJECT_RAW);
#else
    TEST_ASSERT_EQUAL (0, STHS34PF80_TAMBIENT_C);
    TEST_ASSERT_EQUAL (1, STHS34PF80_PRESENCE_FLAG);
    TEST_ASSERT_EQUAL (2, STHS34PF80_MOTION_FLAG);
    TEST_ASSERT_EQUAL (3, STHS34PF80_TOBJECT_RAW);
#endif
    // Verify actual data values at those indices
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    // Debug fields appear as acceleration for plotting (scaled by 1/1000)
    TEST_ASSERT_FLOAT_WITHIN (0.01f, (float) test_tobject / 1000.0f,
                              m_captured_env_data.data[STHS34PF80_DEBUG_TOBJECT]);
    TEST_ASSERT_FLOAT_WITHIN (0.01f, (float) test_tmotion / 1000.0f,
                              m_captured_env_data.data[STHS34PF80_DEBUG_TMOTION]);
    TEST_ASSERT_FLOAT_WITHIN (0.01f, (float) test_tpresence / 1000.0f,
                              m_captured_env_data.data[STHS34PF80_DEBUG_TPRESENCE]);
#endif
    // Regular fields (shifted by +3 when debug is enabled)
    TEST_ASSERT_FLOAT_WITHIN (0.01f, 25.0f, m_captured_env_data.data[STHS34PF80_TAMBIENT_C]);
    TEST_ASSERT_EQUAL_FLOAT ( (float) test_presence,
                              m_captured_env_data.data[STHS34PF80_PRESENCE_FLAG]);
    TEST_ASSERT_EQUAL_FLOAT ( (float) test_motion,
                              m_captured_env_data.data[STHS34PF80_MOTION_FLAG]);
    TEST_ASSERT_EQUAL_FLOAT ( (float) test_tobject,
                              m_captured_env_data.data[STHS34PF80_TOBJECT_RAW]);
}

#endif // TEST
