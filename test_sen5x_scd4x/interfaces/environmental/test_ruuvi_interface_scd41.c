#ifdef TEST

#include "unity.h"

#include "ruuvi_interface_scd41.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_yield.h"
#include "mock_scd4x_i2c.h"

static rd_sensor_t g_ctx;
static const uint8_t g_mock_addr = 0x62U;

void test_ri_scd41_init_ok (void);
void test_ri_scd41_uninit_ok (void);

void setUp (void)
{
    ri_log_Ignore();
    test_ri_scd41_init_ok();
}

void tearDown (void)
{
    test_ri_scd41_uninit_ok();
}


/** @brief @ref rd_sensor_init_fp */
void test_ri_scd41_init_ok (void)
{
    const rd_sensor_data_fields_t expected =
    {
        .datas.co2_ppm = 1,
        .datas.temperature_c = 1,
        .datas.humidity_rh = 1
    };
    rd_sensor_is_init_ExpectAndReturn (&g_ctx, false);
    rd_sensor_initialize_Expect (&g_ctx);
    ri_delay_ms_ExpectAndReturn (1000, 0);
    scd4x_wake_up_ExpectAndReturn (0);
    scd4x_stop_periodic_measurement_ExpectAndReturn (0);
    ri_delay_ms_ExpectAndReturn (500, 0);
    scd4x_reinit_ExpectAndReturn (0);
    scd4x_get_serial_number_ExpectAnyArgsAndReturn (0);
    rd_status_t err_code = ri_scd41_init (&g_ctx, RD_BUS_I2C, g_mock_addr);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (&ri_scd41_init == g_ctx.init);
    TEST_ASSERT (&ri_scd41_uninit == g_ctx.uninit);
    TEST_ASSERT (&ri_scd41_samplerate_set == g_ctx.samplerate_set);
    TEST_ASSERT (&ri_scd41_samplerate_get == g_ctx.samplerate_get);
    TEST_ASSERT (&ri_scd41_resolution_set == g_ctx.resolution_set);
    TEST_ASSERT (&ri_scd41_resolution_get == g_ctx.resolution_get);
    TEST_ASSERT (&ri_scd41_scale_set == g_ctx.scale_set);
    TEST_ASSERT (&ri_scd41_scale_get == g_ctx.scale_get);
    TEST_ASSERT (&ri_scd41_mode_set == g_ctx.mode_set);
    TEST_ASSERT (&ri_scd41_mode_get == g_ctx.mode_get);
    TEST_ASSERT (&ri_scd41_dsp_set == g_ctx.dsp_set);
    TEST_ASSERT (&ri_scd41_dsp_get == g_ctx.dsp_get);
    TEST_ASSERT (&ri_scd41_data_get == g_ctx.data_get);
    TEST_ASSERT (expected.bitfield == g_ctx.provides.bitfield);
}

void test_ri_scd41_uninit_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    scd4x_stop_periodic_measurement_ExpectAndReturn (0);
    ri_delay_ms_ExpectAndReturn (500, 0);
    rd_sensor_uninitialize_Expect (&g_ctx);
    err_code |= ri_scd41_uninit (&g_ctx, RD_BUS_I2C, g_mock_addr);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_scd41_mode_set_sleep (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    scd4x_stop_periodic_measurement_ExpectAndReturn (0);
    ri_delay_ms_ExpectAndReturn (500, 0);
    scd4x_power_down_ExpectAndReturn (0);
    err_code |= ri_scd41_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
}

void test_ri_scd41_mode_set_continuous (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    scd4x_wake_up_ExpectAndReturn (0);
    ri_delay_ms_ExpectAndReturn (20, 0);
    scd4x_start_periodic_measurement_ExpectAndReturn (0);
    err_code |= ri_scd41_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
}


void test_ri_scd41_mode_set_single (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    scd4x_wake_up_ExpectAndReturn (0);
    ri_delay_ms_ExpectAndReturn (20, 0);
    scd4x_measure_single_shot_ExpectAndReturn (0);
    ri_delay_ms_ExpectAndReturn (5000, 0);
    scd4x_get_data_ready_flag_ExpectAnyArgsAndReturn (0);
    bool data_ready_flag = true;
    scd4x_get_data_ready_flag_ReturnThruPtr_data_ready_flag (&data_ready_flag);
    scd4x_read_measurement_ExpectAnyArgsAndReturn (0);
    uint16_t co2 = 100;
    int32_t temperature = 0;
    int32_t humidity = 0;
    scd4x_read_measurement_ReturnThruPtr_co2 (&co2);
    scd4x_read_measurement_ReturnThruPtr_temperature_m_deg_c (&temperature);
    scd4x_read_measurement_ReturnThruPtr_humidity_m_percent_rh (&humidity);
    scd4x_measure_single_shot_ExpectAndReturn (0);
    ri_delay_ms_ExpectAndReturn (5000, 0);
    scd4x_get_data_ready_flag_ExpectAnyArgsAndReturn (0);
    data_ready_flag = true;
    scd4x_get_data_ready_flag_ReturnThruPtr_data_ready_flag (&data_ready_flag);
    rd_sensor_timestamp_get_ExpectAndReturn (1000);
    scd4x_read_measurement_ExpectAnyArgsAndReturn (0);
    co2 = 100;
    temperature = 101;
    humidity = 102;
    scd4x_read_measurement_ReturnThruPtr_co2 (&co2);
    scd4x_read_measurement_ReturnThruPtr_temperature_m_deg_c (&temperature);
    scd4x_read_measurement_ReturnThruPtr_humidity_m_percent_rh (&humidity);
    scd4x_power_down_ExpectAndReturn (0);
    err_code |= ri_scd41_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
}

void test_ri_scd41_mode_set_single_is (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    test_ri_scd41_mode_set_continuous();
    err_code |= ri_scd41_mode_set (&mode);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
}

void test_ri_scd41_mode_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_scd41_mode_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_scd41_data_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_scd41_data_get (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_scd41_data_get_no_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data;
    err_code |= ri_scd41_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_scd41_data_get_single_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data;
    test_ri_scd41_mode_set_single();
    rd_sensor_data_populate_ExpectAnyArgs();
    err_code |= ri_scd41_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_scd41_data_get_continuous (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data;
    test_ri_scd41_mode_set_continuous();
    scd4x_get_data_ready_flag_ExpectAnyArgsAndReturn (0);
    bool data_ready_flag = true;
    scd4x_get_data_ready_flag_ReturnThruPtr_data_ready_flag (&data_ready_flag);
    scd4x_read_measurement_ExpectAnyArgsAndReturn (0);
    uint16_t co2 = 100;
    int32_t temperature = 101;
    int32_t humidity = 102;
    scd4x_read_measurement_ReturnThruPtr_co2 (&co2);
    scd4x_read_measurement_ReturnThruPtr_temperature_m_deg_c (&temperature);
    scd4x_read_measurement_ReturnThruPtr_humidity_m_percent_rh (&humidity);
    rd_sensor_timestamp_get_ExpectAndReturn (1000);
    rd_sensor_data_populate_ExpectAnyArgs();
    err_code |= ri_scd41_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

#endif //TEST
