#ifdef TEST

#include "unity.h"

#include "ruuvi_interface_shtcx.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_yield.h"
#include "mock_sensirion_i2c.h"
#include "mock_shtc1.h"

static rd_sensor_t shtcx_ctx;
static const uint8_t mock_addr = 0x70U;

void test_ri_shtcx_init_ok (void);
void test_ri_shtcx_uninit_ok (void);

void setUp (void)
{
    ri_log_Ignore();
    test_ri_shtcx_init_ok();
}

void tearDown (void)
{
    test_ri_shtcx_uninit_ok ();
}


/** @brief @ref rd_sensor_init_fp */
void test_ri_shtcx_init_ok (void)
{
    const rd_sensor_data_fields_t expected =
    {
        .datas.temperature_c = 1,
        .datas.humidity_rh = 1
    };
    rd_sensor_is_init_ExpectAndReturn (&shtcx_ctx, false);
    rd_sensor_initialize_Expect (&shtcx_ctx);
    shtc1_probe_ExpectAndReturn (0);
    shtc1_enable_low_power_mode_Expect (0);
    shtc1_sleep_ExpectAndReturn (0);
    rd_status_t err_code = ri_shtcx_init (&shtcx_ctx, RD_BUS_I2C, mock_addr);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (&ri_shtcx_init == shtcx_ctx.init);
    TEST_ASSERT (&ri_shtcx_uninit == shtcx_ctx.uninit);
    TEST_ASSERT (&ri_shtcx_samplerate_set == shtcx_ctx.samplerate_set);
    TEST_ASSERT (&ri_shtcx_samplerate_get == shtcx_ctx.samplerate_get);
    TEST_ASSERT (&ri_shtcx_resolution_set == shtcx_ctx.resolution_set);
    TEST_ASSERT (&ri_shtcx_resolution_get == shtcx_ctx.resolution_get);
    TEST_ASSERT (&ri_shtcx_scale_set == shtcx_ctx.scale_set);
    TEST_ASSERT (&ri_shtcx_scale_get == shtcx_ctx.scale_get);
    TEST_ASSERT (&ri_shtcx_mode_set == shtcx_ctx.mode_set);
    TEST_ASSERT (&ri_shtcx_mode_get == shtcx_ctx.mode_get);
    TEST_ASSERT (&ri_shtcx_dsp_set == shtcx_ctx.dsp_set);
    TEST_ASSERT (&ri_shtcx_dsp_get == shtcx_ctx.dsp_get);
    TEST_ASSERT (&ri_shtcx_data_get == shtcx_ctx.data_get);
    TEST_ASSERT (expected.bitfield == shtcx_ctx.provides.bitfield);
}

void test_ri_shtcx_uninit_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    shtc1_enable_low_power_mode_Expect (1);
    shtc1_sleep_ExpectAndReturn (0);
    rd_sensor_uninitialize_Expect (&shtcx_ctx);
    err_code |= ri_shtcx_uninit (&shtcx_ctx, RD_BUS_I2C, mock_addr);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_shtcx_mode_set_sleep (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    shtc1_sleep_ExpectAndReturn (0);
    err_code |= ri_shtcx_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
}

// SHTC does not support continuous mode, this just modifies SW state of driver.
void test_ri_shtcx_mode_set_continuous (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    err_code |= ri_shtcx_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
}


void test_ri_shtcx_mode_set_single (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    rd_sensor_timestamp_get_ExpectAndReturn (1000);
    shtc1_wake_up_ExpectAndReturn (0);
    sensirion_sleep_usec_Expect (RI_SHTCX_WAKEUP_US);
    shtc1_measure_blocking_read_ExpectAnyArgsAndReturn (0);
    shtc1_sleep_ExpectAndReturn (0);
    err_code |= ri_shtcx_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
}

void test_ri_shtcx_mode_set_single_is (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    test_ri_shtcx_mode_set_continuous();
    err_code |= ri_shtcx_mode_set (&mode);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
}

void test_ri_shtcx_mode_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_shtcx_mode_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_shtcx_data_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_shtcx_data_get (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_shtcx_data_get_no_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data;
    err_code |= ri_shtcx_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_shtcx_data_get_single_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data;
    test_ri_shtcx_mode_set_single();
    rd_sensor_data_populate_ExpectAnyArgs();
    err_code |= ri_shtcx_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_shtcx_data_get_continuous (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data;
    test_ri_shtcx_mode_set_continuous();
    rd_sensor_timestamp_get_ExpectAndReturn (1000);
    shtc1_wake_up_ExpectAndReturn (0);
    sensirion_sleep_usec_Expect (RI_SHTCX_WAKEUP_US);
    shtc1_measure_blocking_read_ExpectAnyArgsAndReturn (0);
    shtc1_sleep_ExpectAndReturn (0);
    rd_sensor_data_populate_ExpectAnyArgs();
    err_code |= ri_shtcx_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

#endif //TEST
