#include "unity.h"

#include "ruuvi_interface_tmp117.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_i2c_tmp117.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_yield.h"

static rd_sensor_t tmp_ctx;
static const uint8_t mock_addr = 0x48U;

void test_ri_tmp117_init_ok (void);
void test_ri_tmp117_uninit_ok (void);

void setUp (void)
{
    ri_log_Ignore();
    test_ri_tmp117_init_ok();
}

void tearDown (void)
{
    test_ri_tmp117_uninit_ok ();
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
    tearDown ();
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


void test_ri_tmp117_uninit_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint16_t reg_val = TMP117_VALUE_MODE_SLEEP;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, reg_val,
                                         RD_SUCCESS);
    rd_sensor_uninitialize_Expect (&tmp_ctx);
    ri_tmp117_uninit (&tmp_ctx, RD_BUS_I2C, 1);
    TEST_ASSERT (RD_SUCCESS == err_code);
    setUp ();
}

void test_ri_tmp117_dsp_set_default (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_CFG_DEFAULT;
    uint8_t parameter = RD_SENSOR_CFG_DEFAULT;
    uint16_t reg_val = TMP117_VALUE_OS_1;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, reg_val,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (1 == parameter);
}

void test_ri_tmp117_dsp_set_os_8 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_DSP_OS;
    uint8_t parameter = 2;
    uint16_t reg_val = TMP117_VALUE_OS_8;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, reg_val,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (8 == parameter);
}

void test_ri_tmp117_dsp_set_os_32 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_DSP_OS;
    uint8_t parameter = 10;
    uint16_t reg_val = TMP117_VALUE_OS_32;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, reg_val,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (32 == parameter);
}

void test_ri_tmp117_dsp_set_os_64 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_DSP_OS;
    uint8_t parameter = 40;
    uint16_t reg_val = TMP117_VALUE_OS_64;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, reg_val,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (64 == parameter);
}

void test_ri_tmp117_dsp_set_os_ns (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_DSP_OS;
    uint8_t parameter = 65;
    uint16_t reg_val = TMP117_VALUE_OS_64;
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
}

void test_ri_tmp117_dsp_set_os_max (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_DSP_OS;
    uint8_t parameter = RD_SENSOR_CFG_MAX;
    uint16_t reg_val = TMP117_VALUE_OS_64;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, reg_val,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (64 == parameter);
}

void test_ri_tmp117_dsp_set_os_min (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_DSP_OS;
    uint8_t parameter = RD_SENSOR_CFG_MIN;
    uint16_t reg_val = TMP117_VALUE_OS_8;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, reg_val,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (8 == parameter);
}

void test_ri_tmp117_dsp_set_os_nc (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_CFG_NO_CHANGE;
    uint8_t parameter = 8;
    uint16_t reg_val = TMP117_VALUE_OS_1;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_dsp_set (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (1 == parameter);
}

void test_ri_tmp117_dsp_set_os_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = RD_SENSOR_DSP_OS;
    uint8_t parameter = RD_SENSOR_CFG_MAX;
    err_code = ri_tmp117_dsp_set (&dsp, NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
    err_code = ri_tmp117_dsp_set (NULL, NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
    err_code = ri_tmp117_dsp_set (NULL, &parameter);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_tmp117_dsp_get_default (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0;
    uint8_t parameter = 0;
    uint16_t reg_val = TMP117_VALUE_OS_1;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_dsp_get (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_DSP_LAST == dsp);
    TEST_ASSERT (1 == parameter);
}


void test_ri_tmp117_dsp_get_os_8 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0;
    uint8_t parameter = 0;
    uint16_t reg_val = TMP117_VALUE_OS_8;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_dsp_get (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_DSP_OS == dsp);
    TEST_ASSERT (8 == parameter);
}

void test_ri_tmp117_dsp_get_os_32 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0;
    uint8_t parameter = 0;
    uint16_t reg_val = TMP117_VALUE_OS_32;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_dsp_get (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_DSP_OS == dsp);
    TEST_ASSERT (32 == parameter);
}

void test_ri_tmp117_dsp_get_os_64 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0;
    uint8_t parameter = 0;
    uint16_t reg_val = TMP117_VALUE_OS_64;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_dsp_get (&dsp, &parameter);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_DSP_OS == dsp);
    TEST_ASSERT (64 == parameter);
}

void test_ri_tmp117_dsp_get_os_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t dsp = 0;
    uint8_t parameter = 0;
    err_code = ri_tmp117_dsp_get (&dsp, NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
    err_code = ri_tmp117_dsp_get (NULL, NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
    err_code = ri_tmp117_dsp_get (NULL, &parameter);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_tmp117_samplerate_set_1 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 1;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_1000_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (1 == rate);
}

void test_ri_tmp117_samplerate_set_2 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 2;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_500_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (2 == rate);
}

void test_ri_tmp117_samplerate_set_4 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 3;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_250_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (4 == rate);
}

void test_ri_tmp117_samplerate_set_8 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 5;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_125_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (8 == rate);
}

void test_ri_tmp117_samplerate_set_64 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 9;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_16_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (64 == rate);
}

void test_ri_tmp117_samplerate_set_custom_1 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = RD_SENSOR_CFG_CUSTOM_1;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_4000_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CUSTOM_1 == rate);
}

void test_ri_tmp117_samplerate_set_custom_2 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = RD_SENSOR_CFG_CUSTOM_2;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_8000_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CUSTOM_2 == rate);
}

void test_ri_tmp117_samplerate_set_custom_3 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = RD_SENSOR_CFG_CUSTOM_3;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_16000_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CUSTOM_3 == rate);
}

void test_ri_tmp117_samplerate_set_nc (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = RD_SENSOR_CFG_NO_CHANGE;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_tmp117_samplerate_set_min (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = RD_SENSOR_CFG_MIN;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_16000_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_CUSTOM_3 == rate);
}

void test_ri_tmp117_samplerate_set_default (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = RD_SENSOR_CFG_DEFAULT;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_1000_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (1 == rate);
}

void test_ri_tmp117_samplerate_set_max (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = RD_SENSOR_CFG_MAX;
    uint16_t reg_val = 0;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    ri_i2c_tmp117_write_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION,
                                         TMP117_VALUE_CC_16_MS,
                                         RD_SUCCESS);
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (64 == rate);
}

void test_ri_tmp117_samplerate_set_ns (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 65;
    err_code |= ri_tmp117_samplerate_set (&rate);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
}

void test_ri_tmp117_samplerate_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_tmp117_samplerate_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_tmp117_samplerate_get_1 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 0;
    uint16_t reg_val = TMP117_VALUE_CC_1000_MS;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_samplerate_get (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (1 == rate);
}

void test_ri_tmp117_samplerate_get_2 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 0;
    uint16_t reg_val = TMP117_VALUE_CC_500_MS;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_samplerate_get (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (2 == rate);
}

void test_ri_tmp117_samplerate_get_4 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 0;
    uint16_t reg_val = TMP117_VALUE_CC_250_MS;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_samplerate_get (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (4 == rate);
}

void test_ri_tmp117_samplerate_get_8 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 0;
    uint16_t reg_val = TMP117_VALUE_CC_125_MS;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_samplerate_get (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (8 == rate);
}

void test_ri_tmp117_samplerate_get_64 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 0;
    uint16_t reg_val = TMP117_VALUE_CC_16_MS;
    ri_i2c_tmp117_read_ExpectAndReturn (mock_addr, TMP117_REG_CONFIGURATION, NULL,
                                        RD_SUCCESS);
    ri_i2c_tmp117_read_IgnoreArg_reg_val();
    ri_i2c_tmp117_read_ReturnThruPtr_reg_val (&reg_val);
    err_code |= ri_tmp117_samplerate_get (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (64 == rate);
}

void test_ri_tmp117_samplerate_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_tmp117_samplerate_get (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_tmp117_resolution_set_default (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t resolution = RD_SENSOR_CFG_DEFAULT;
    err_code |= ri_tmp117_resolution_set (&resolution);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

void test_ri_tmp117_resolution_set_min (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t resolution = RD_SENSOR_CFG_MIN;
    err_code |= ri_tmp117_resolution_set (&resolution);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

void test_ri_tmp117_resolution_set_max (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t resolution = RD_SENSOR_CFG_MAX;
    err_code |= ri_tmp117_resolution_set (&resolution);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

void test_ri_tmp117_resolution_set_no_change (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t resolution = RD_SENSOR_CFG_NO_CHANGE;
    err_code |= ri_tmp117_resolution_set (&resolution);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

void test_ri_tmp117_resolution_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_tmp117_resolution_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_tmp117_resolution_get_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t resolution = RD_SENSOR_CFG_NO_CHANGE;
    err_code |= ri_tmp117_resolution_get (&resolution);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

void test_ri_tmp117_resolution_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_tmp117_resolution_get (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}