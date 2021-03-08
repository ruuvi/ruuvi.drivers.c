#include "unity.h"

#include "ruuvi_interface_tmp117.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_i2c_tmp117.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_yield.h"

static rd_sensor_t tmp_ctx;
static const uint8_t mock_addr = 0x48U;

void setUp (void)
{
    ri_log_Ignore();
}

void tearDown (void)
{
}

static void soft_reset_Expect (void)
{
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, 2, RD_SUCCESS);
}

static void validate_id_Expect (void)
{
    static uint16_t reg_val = TMP117_VALUE_ID;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_DEVICE_ID, NULL, RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
}

static void validate_id_error_Expect (void)
{
    static uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_DEVICE_ID, NULL, RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
}

/** @brief @ref rd_sensor_init_fp */
void test_ri_tmp117_init_ok (void)
{
    const rd_sensor_data_fields_t expected =
    {
        .datas.temperature_c = 1
    };
    rd_sensor_is_init_ExpectAndReturn (&tmp_ctx, false);
    rd_sensor_initialize_Expect (&tmp_ctx);
    validate_id_Expect();
    soft_reset_Expect ();
    rd_status_t err_code = ri_tmp117_init (&tmp_ctx, RD_BUS_I2C, mock_addr);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (&ri_tmp117_init == tmp_ctx.init);
    TEST_ASSERT (&ri_tmp117_uninit == tmp_ctx.uninit);
    TEST_ASSERT (&ri_tmp117_samplerate_set == tmp_ctx.samplerate_set);
    TEST_ASSERT (&ri_tmp117_samplerate_get == tmp_ctx.samplerate_get);
    TEST_ASSERT (&ri_tmp117_resolution_set == tmp_ctx.resolution_set);
    TEST_ASSERT (&ri_tmp117_resolution_get == tmp_ctx.resolution_get);
    TEST_ASSERT (&ri_tmp117_scale_set == tmp_ctx.scale_set);
    TEST_ASSERT (&ri_tmp117_scale_get == tmp_ctx.scale_get);
    TEST_ASSERT (&ri_tmp117_mode_set == tmp_ctx.mode_set);
    TEST_ASSERT (&ri_tmp117_mode_get == tmp_ctx.mode_get);
    TEST_ASSERT (&ri_tmp117_dsp_set == tmp_ctx.dsp_set);
    TEST_ASSERT (&ri_tmp117_dsp_get == tmp_ctx.dsp_get);
    TEST_ASSERT (&ri_tmp117_data_get == tmp_ctx.data_get);
    TEST_ASSERT (expected.bitfield == tmp_ctx.provides.bitfield);
}

void test_ri_tmp117_init_invalid_id (void)
{
    const rd_sensor_data_fields_t expected =
    {
        .datas.temperature_c = 1
    };
    rd_sensor_is_init_ExpectAndReturn (&tmp_ctx, false);
    rd_sensor_initialize_Expect (&tmp_ctx);
    validate_id_error_Expect();
    rd_status_t err_code = ri_tmp117_init (&tmp_ctx, RD_BUS_I2C, mock_addr);
    TEST_ASSERT (RD_ERROR_NOT_FOUND == err_code);
}

void test_ri_tmp117_init_already_init (void)
{
    rd_sensor_is_init_ExpectAndReturn (&tmp_ctx, true);
    rd_status_t err_code = ri_tmp117_init (&tmp_ctx, RD_BUS_I2C, mock_addr);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_ri_tmp117_dsp_set_last (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_CFG_LAST;
    uint8_t parameter = 1;
    ri_i2c_tmp117_read_ExpectAndReturn ();
    ri_i2c_tmp117_write_ExpectAndReturn ();
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
}