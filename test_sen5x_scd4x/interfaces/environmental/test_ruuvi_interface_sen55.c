#ifdef TEST

#include "unity.h"

#include "ruuvi_interface_sen55.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_yield.h"
#include "mock_sensirion_i2c_hal.h"
#include "mock_sensirion_i2c.h"
#include "mock_sen5x_i2c.h"

static rd_sensor_t g_ctx;
static const uint8_t g_mock_addr = 0x69U;

void test_ri_sen55_init_ok (void);
void test_ri_sen55_uninit_ok (void);

void setUp (void)
{
    ri_log_Ignore();
    test_ri_sen55_init_ok();
}

void tearDown (void)
{
    test_ri_sen55_uninit_ok();
}


/** @brief @ref rd_sensor_init_fp */
void test_ri_sen55_init_ok (void)
{
    const rd_sensor_data_fields_t expected =
    {
        .datas.humidity_rh = 1,
        .datas.pm_1_ugm3 = 1,
        .datas.pm_2_ugm3 = 1,
        .datas.pm_4_ugm3 = 1,
        .datas.pm_10_ugm3 = 1,
        .datas.temperature_c = 1,
        .datas.voc_index = 1,
        .datas.nox_index = 1,
    };
    rd_sensor_is_init_ExpectAndReturn (&g_ctx, false);
    rd_sensor_initialize_Expect (&g_ctx);
    sen5x_device_reset_ExpectAndReturn (0);
    ri_delay_ms_ExpectAndReturn (50, 0);
    unsigned char product_name[] = "SEN51";
    sen5x_get_product_name_IgnoreAndReturn (0);
    sen5x_get_serial_number_IgnoreAndReturn (0);
    sen5x_get_version_IgnoreAndReturn (0);
    rd_status_t err_code = ri_sen55_init (&g_ctx, RD_BUS_I2C, g_mock_addr);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (&ri_sen55_init == g_ctx.init);
    TEST_ASSERT (&ri_sen55_uninit == g_ctx.uninit);
    TEST_ASSERT (&ri_sen55_samplerate_set == g_ctx.samplerate_set);
    TEST_ASSERT (&ri_sen55_samplerate_get == g_ctx.samplerate_get);
    TEST_ASSERT (&ri_sen55_resolution_set == g_ctx.resolution_set);
    TEST_ASSERT (&ri_sen55_resolution_get == g_ctx.resolution_get);
    TEST_ASSERT (&ri_sen55_scale_set == g_ctx.scale_set);
    TEST_ASSERT (&ri_sen55_scale_get == g_ctx.scale_get);
    TEST_ASSERT (&ri_sen55_mode_set == g_ctx.mode_set);
    TEST_ASSERT (&ri_sen55_mode_get == g_ctx.mode_get);
    TEST_ASSERT (&ri_sen55_dsp_set == g_ctx.dsp_set);
    TEST_ASSERT (&ri_sen55_dsp_get == g_ctx.dsp_get);
    TEST_ASSERT (&ri_sen55_data_get == g_ctx.data_get);
    TEST_ASSERT (expected.bitfield == g_ctx.provides.bitfield);
}

void test_ri_sen55_uninit_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    sen5x_device_reset_ExpectAndReturn (0);
    rd_sensor_uninitialize_Expect (&g_ctx);
    err_code |= ri_sen55_uninit (&g_ctx, RD_BUS_I2C, g_mock_addr);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_sen55_mode_set_sleep (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    sen5x_stop_measurement_ExpectAndReturn (0);
    err_code |= ri_sen55_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
}

void test_ri_sen55_mode_set_continuous (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    sen5x_start_measurement_ExpectAndReturn (0);
    err_code |= ri_sen55_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
}


void test_ri_sen55_mode_set_single (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    rd_sensor_timestamp_get_ExpectAndReturn (1000);
    sen5x_start_measurement_ExpectAndReturn (0);
    sen5x_read_data_ready_ExpectAnyArgsAndReturn (0);
    bool flag_data_ready1 = false;
    sen5x_read_data_ready_ReturnThruPtr_data_ready (&flag_data_ready1);
    sen5x_read_data_ready_ExpectAnyArgsAndReturn (0);
    bool flag_data_ready2 = true;
    sen5x_read_data_ready_ReturnThruPtr_data_ready (&flag_data_ready2);
    sen5x_read_measured_values_ExpectAnyArgsAndReturn (0);
    uint16_t mass_concentration_pm1p0 = 100;
    uint16_t mass_concentration_pm2p5 = 101;
    uint16_t mass_concentration_pm4p0 = 102;
    uint16_t mass_concentration_pm10p0 = 103;
    int16_t ambient_humidity = 104;
    int16_t ambient_temperature = 105;
    int16_t voc_index = 106;
    int16_t nox_index = 107;
    sen5x_read_measured_values_ReturnThruPtr_mass_concentration_pm1p0 (
        &mass_concentration_pm1p0);
    sen5x_read_measured_values_ReturnThruPtr_mass_concentration_pm2p5 (
        &mass_concentration_pm2p5);
    sen5x_read_measured_values_ReturnThruPtr_mass_concentration_pm4p0 (
        &mass_concentration_pm4p0);
    sen5x_read_measured_values_ReturnThruPtr_mass_concentration_pm10p0 (
        &mass_concentration_pm10p0);
    sen5x_read_measured_values_ReturnThruPtr_ambient_humidity (&ambient_humidity);
    sen5x_read_measured_values_ReturnThruPtr_ambient_temperature (&ambient_temperature);
    sen5x_read_measured_values_ReturnThruPtr_voc_index (&voc_index);
    sen5x_read_measured_values_ReturnThruPtr_nox_index (&nox_index);
    sen5x_stop_measurement_ExpectAndReturn (0);
    err_code |= ri_sen55_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
}

void test_ri_sen55_mode_set_single_is (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    test_ri_sen55_mode_set_continuous();
    err_code |= ri_sen55_mode_set (&mode);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
}

void test_ri_sen55_mode_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_sen55_mode_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_sen55_data_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_sen55_data_get (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_sen55_data_get_no_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data = {0};
    err_code |= ri_sen55_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_sen55_data_get_single_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data;
    test_ri_sen55_mode_set_single();
    rd_sensor_data_populate_ExpectAnyArgs();
    err_code |= ri_sen55_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_sen55_data_get_continuous (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_data_t data;
    test_ri_sen55_mode_set_continuous();
    sen5x_read_data_ready_ExpectAnyArgsAndReturn (0);
    bool flag_data_ready = true;
    sen5x_read_data_ready_ReturnThruPtr_data_ready (&flag_data_ready);
    sen5x_read_measured_values_ExpectAnyArgsAndReturn (0);
    uint16_t mass_concentration_pm1p0 = 100;
    uint16_t mass_concentration_pm2p5 = 101;
    uint16_t mass_concentration_pm4p0 = 102;
    uint16_t mass_concentration_pm10p0 = 103;
    int16_t ambient_humidity = 104;
    int16_t ambient_temperature = 105;
    int16_t voc_index = 106;
    int16_t nox_index = 107;
    sen5x_read_measured_values_ReturnThruPtr_mass_concentration_pm1p0 (
        &mass_concentration_pm1p0);
    sen5x_read_measured_values_ReturnThruPtr_mass_concentration_pm2p5 (
        &mass_concentration_pm2p5);
    sen5x_read_measured_values_ReturnThruPtr_mass_concentration_pm4p0 (
        &mass_concentration_pm4p0);
    sen5x_read_measured_values_ReturnThruPtr_mass_concentration_pm10p0 (
        &mass_concentration_pm10p0);
    sen5x_read_measured_values_ReturnThruPtr_ambient_humidity (&ambient_humidity);
    sen5x_read_measured_values_ReturnThruPtr_ambient_temperature (&ambient_temperature);
    sen5x_read_measured_values_ReturnThruPtr_voc_index (&voc_index);
    sen5x_read_measured_values_ReturnThruPtr_nox_index (&nox_index);
    rd_sensor_timestamp_get_ExpectAndReturn (1000);
    rd_sensor_data_populate_ExpectAnyArgs();
    err_code |= ri_sen55_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

#endif //TEST
