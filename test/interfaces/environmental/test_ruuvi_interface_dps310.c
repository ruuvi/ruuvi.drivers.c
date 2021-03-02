#include "unity.h"

#include "mock_dps310.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_spi_dps310.h"
#include "mock_ruuvi_interface_yield.h"
#include "ruuvi_interface_dps310.h"

#include "string.h"

#define SIM_TIME_MS (1000U)
#define SIM_TEMPERATURE_C (25.5F)
#define SIM_PRESSURE_PA (100032.4F)

static void dummy_sleep (const uint32_t ms)
{
    // No action needed.
}

rd_sensor_t dps_ctx;
dps310_ctx_t init_ctx =
{
    .write = &ri_spi_dps310_write,
    .read  = &ri_spi_dps310_read,
    .sleep = &dummy_sleep
};

void setUp (void)
{
    memset (&dps_ctx, 0, sizeof (dps_ctx));
    ri_log_Ignore();
}

void tearDown (void)
{
}

/** @brief @ref rd_sensor_init_fp */
void test_ri_dps310_init_ok (void)
{
    const rd_sensor_data_fields_t expected =
    {
        .datas.temperature_c = 1,
        .datas.pressure_pa = 1
    };
    dps_ctx.p_ctx = &init_ctx;
    rd_sensor_is_init_ExpectAndReturn (&dps_ctx, false);
    rd_sensor_initialize_Expect (&dps_ctx);
    dps310_init_ExpectAndReturn (&init_ctx, DPS310_SUCCESS);
    rd_status_t err_code = ri_dps310_init (&dps_ctx, RD_BUS_SPI, 1U);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (&ri_dps310_init == dps_ctx.init);
    TEST_ASSERT (&ri_dps310_uninit == dps_ctx.uninit);
    TEST_ASSERT (&ri_dps310_samplerate_set == dps_ctx.samplerate_set);
    TEST_ASSERT (&ri_dps310_samplerate_get == dps_ctx.samplerate_get);
    TEST_ASSERT (&ri_dps310_resolution_set == dps_ctx.resolution_set);
    TEST_ASSERT (&ri_dps310_resolution_get == dps_ctx.resolution_get);
    TEST_ASSERT (&ri_dps310_scale_set == dps_ctx.scale_set);
    TEST_ASSERT (&ri_dps310_scale_get == dps_ctx.scale_get);
    TEST_ASSERT (&ri_dps310_mode_set == dps_ctx.mode_set);
    TEST_ASSERT (&ri_dps310_mode_get == dps_ctx.mode_get);
    TEST_ASSERT (&ri_dps310_dsp_set == dps_ctx.dsp_set);
    TEST_ASSERT (&ri_dps310_dsp_get == dps_ctx.dsp_get);
    TEST_ASSERT (&ri_dps310_data_get == dps_ctx.data_get);
    TEST_ASSERT (expected.bitfield == dps_ctx.provides.bitfield);
}

void test_ri_dps310_init_singleton (void)
{
    rd_sensor_is_init_ExpectAndReturn (&dps_ctx, false);
    rd_sensor_initialize_Expect (&dps_ctx);
    dps310_init_ExpectAndReturn (NULL, DPS310_SUCCESS);
    dps310_init_IgnoreArg_ctx();
    rd_status_t err_code = ri_dps310_init (&dps_ctx, RD_BUS_SPI, 1U);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (&ri_dps310_init == dps_ctx.init);
    TEST_ASSERT (&ri_dps310_uninit == dps_ctx.uninit);
    TEST_ASSERT (&ri_dps310_samplerate_set == dps_ctx.samplerate_set);
    TEST_ASSERT (&ri_dps310_samplerate_get == dps_ctx.samplerate_get);
    TEST_ASSERT (&ri_dps310_resolution_set == dps_ctx.resolution_set);
    TEST_ASSERT (&ri_dps310_resolution_get == dps_ctx.resolution_get);
    TEST_ASSERT (&ri_dps310_scale_set == dps_ctx.scale_set);
    TEST_ASSERT (&ri_dps310_scale_get == dps_ctx.scale_get);
    TEST_ASSERT (&ri_dps310_mode_set == dps_ctx.mode_set);
    TEST_ASSERT (&ri_dps310_mode_get == dps_ctx.mode_get);
    TEST_ASSERT (&ri_dps310_dsp_set == dps_ctx.dsp_set);
    TEST_ASSERT (&ri_dps310_dsp_get == dps_ctx.dsp_get);
    TEST_ASSERT (&ri_dps310_data_get == dps_ctx.data_get);
    TEST_ASSERT (NULL != dps_ctx.p_ctx);
    dps310_ctx_t * p_dps_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    TEST_ASSERT (NULL != p_dps_ctx->comm_ctx);
    uint8_t * comm_ctx = (uint8_t *) p_dps_ctx->comm_ctx;
    TEST_ASSERT (1U == *comm_ctx);
}

void test_ri_dps310_init_singleton_i2c_notimpl (void)
{
    rd_sensor_is_init_ExpectAndReturn (&dps_ctx, false);
    rd_sensor_initialize_Expect (&dps_ctx);
    rd_status_t err_code = ri_dps310_init (&dps_ctx, RD_BUS_I2C, 1U);
    TEST_ASSERT (RD_ERROR_NOT_IMPLEMENTED == err_code);
}

void test_ri_dps310_init_singleton_uart_notsupp (void)
{
    rd_sensor_is_init_ExpectAndReturn (&dps_ctx, false);
    rd_sensor_initialize_Expect (&dps_ctx);
    rd_status_t err_code = ri_dps310_init (&dps_ctx, RD_BUS_UART, 1U);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
}

void test_ri_dps310_init_null (void)
{
    rd_status_t err_code = ri_dps310_init (NULL, RD_BUS_SPI, 0);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}


void test_ri_dps310_init_twice (void)
{
    dps_ctx.p_ctx = &init_ctx;
    rd_sensor_is_init_ExpectAndReturn (&dps_ctx, true);
    rd_status_t err_code = ri_dps310_init (&dps_ctx, RD_BUS_SPI, 1U);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_ri_dps310_uninit_ok (void)
{
    dps_ctx.p_ctx = &init_ctx;
    dps310_uninit_ExpectAndReturn (&init_ctx, DPS310_SUCCESS);
    rd_sensor_uninitialize_Expect (&dps_ctx);
    rd_status_t err_code = ri_dps310_uninit (&dps_ctx, RD_BUS_SPI, 1U);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_dps310_uninit_null (void)
{
    dps_ctx.p_ctx = &init_ctx;
    rd_status_t err_code = ri_dps310_uninit (NULL, RD_BUS_SPI, 1U);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_dps310_samplerate_set_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    // Take address of singleton context to simulate state updates in driver
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;

    for (uint8_t rate = 0U; rate <= 129U; rate ++)
    {
        // DEFAULT rate, 1/s
        if (0U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_1;
            p_ctx->pres_mr = DPS310_MR_1;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_1,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_1,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (1U == rate);
            rate = 0;
        }
        else if (1U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_1;
            p_ctx->pres_mr = DPS310_MR_1;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_1,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_1,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (1U == rate);
        }
        else if (2U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_2;
            p_ctx->pres_mr = DPS310_MR_2;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_2,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_2,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (2U == rate);
        }
        else if (3U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_4;
            p_ctx->pres_mr = DPS310_MR_4;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_4,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_4,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (4U == rate);
        }
        else if (5U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_8;
            p_ctx->pres_mr = DPS310_MR_8;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_8,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_8,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (8U == rate);
        }
        else if (9U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_16;
            p_ctx->pres_mr = DPS310_MR_16;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_16,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_16,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (16U == rate);
        }
        else if (17U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_32;
            p_ctx->pres_mr = DPS310_MR_32;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_32,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_32,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (32U == rate);
        }
        else if (33U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_64;
            p_ctx->pres_mr = DPS310_MR_64;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_64,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_64,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (64U == rate);
        }
        else if (65U == rate)
        {
            p_ctx->temp_mr = DPS310_MR_128;
            p_ctx->pres_mr = DPS310_MR_128;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_128,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_osr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                DPS310_MR_128,
                                                0,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_osr();
            ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (128U == rate);
        }
        else
        {
            err_code = ri_dps310_samplerate_set (&rate);
            TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
        }
    }
}

void test_ri_dps310_samplerate_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    // Take address of singleton context to simulate state updates in driver
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    err_code = ri_dps310_samplerate_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_ri_dps310_samplerate_set_invalid_state (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    // Take address of singleton context to simulate state updates in driver
    uint8_t rate = 1U;
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_CONTINUOUS;
    err_code = ri_dps310_samplerate_set (&rate);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_ri_dps310_samplerate_set_min (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    // Take address of singleton context to simulate state updates in driver
    uint8_t rate = RD_SENSOR_CFG_MIN;
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    p_ctx->temp_mr = DPS310_MR_1;
    p_ctx->pres_mr = DPS310_MR_1;
    dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                        DPS310_MR_1,
                                        0,
                                        DPS310_SUCCESS);
    dps310_config_temp_IgnoreArg_temp_osr();
    dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                        DPS310_MR_1,
                                        0,
                                        DPS310_SUCCESS);
    dps310_config_pres_IgnoreArg_pres_osr();
    err_code = ri_dps310_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (1U == rate);
}

void test_ri_dps310_samplerate_set_max (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    // Take address of singleton context to simulate state updates in driver
    uint8_t rate = RD_SENSOR_CFG_MAX;
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    p_ctx->temp_mr = DPS310_MR_128;
    p_ctx->pres_mr = DPS310_MR_128;
    dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                        DPS310_MR_128,
                                        0,
                                        DPS310_SUCCESS);
    dps310_config_temp_IgnoreArg_temp_osr();
    dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                        DPS310_MR_128,
                                        0,
                                        DPS310_SUCCESS);
    dps310_config_pres_IgnoreArg_pres_osr();
    err_code = ri_dps310_samplerate_set (&rate);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (128U == rate);
}
// sanplerate_get is tested through set, not need to test separately.

void test_ri_dps310_resolution_set_default (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t resolution = RD_SENSOR_CFG_DEFAULT;
    ri_dps310_resolution_set (&resolution);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

void test_ri_dps310_resolution_set_min (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t resolution = RD_SENSOR_CFG_MIN;
    ri_dps310_resolution_set (&resolution);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

void test_ri_dps310_resolution_set_max (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t resolution = RD_SENSOR_CFG_MAX;
    ri_dps310_resolution_set (&resolution);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

void test_ri_dps310_resolution_set_no_change (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t resolution = RD_SENSOR_CFG_NO_CHANGE;
    ri_dps310_resolution_set (&resolution);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == resolution);
}

// resolution get gets tested by resolution set.

void test_ri_dps310_scale_set_default (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t scale = RD_SENSOR_CFG_DEFAULT;
    ri_dps310_scale_set (&scale);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == scale);
}

void test_ri_dps310_scale_set_min (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t scale = RD_SENSOR_CFG_MIN;
    ri_dps310_scale_set (&scale);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == scale);
}

void test_ri_dps310_scale_set_max (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t scale = RD_SENSOR_CFG_MAX;
    ri_dps310_scale_set (&scale);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == scale);
}

void test_ri_dps310_scale_set_no_change (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t scale = RD_SENSOR_CFG_NO_CHANGE;
    ri_dps310_scale_set (&scale);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == scale);
}


void test_ri_dps310_dsp_set_last (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t dsp_func  = RD_SENSOR_DSP_LAST;
    uint8_t dsp_param = RD_SENSOR_CFG_DEFAULT;
    p_ctx->temp_osr = DPS310_OS_1;
    p_ctx->pres_osr = DPS310_OS_1;
    dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                        0,
                                        DPS310_OS_1,
                                        DPS310_SUCCESS);
    dps310_config_temp_IgnoreArg_temp_mr();
    dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                        0,
                                        DPS310_OS_1,
                                        DPS310_SUCCESS);
    dps310_config_pres_IgnoreArg_pres_mr();
    err_code = ri_dps310_dsp_set (&dsp_func, &dsp_param);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_DSP_LAST == dsp_func);
}


void test_ri_dps310_dsp_set_os (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    // Take address of singleton context to simulate state updates in driver
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t dsp_func = RD_SENSOR_DSP_OS;

    for (uint8_t os = 0U; os < 129U; os ++)
    {
        uint8_t dsp_func = RD_SENSOR_DSP_OS;

        // DEFAULT os, 1/s
        if (0U == os)
        {
            p_ctx->temp_osr = DPS310_OS_1;
            p_ctx->pres_osr = DPS310_OS_1;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_1,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_1,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (1U == os);
            os = 0;
        }
        else if (1U == os)
        {
            p_ctx->temp_osr = DPS310_OS_1;
            p_ctx->pres_osr = DPS310_OS_1;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_1,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_1,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (1U == os);
        }
        else if (2U == os)
        {
            p_ctx->temp_osr = DPS310_OS_2;
            p_ctx->pres_osr = DPS310_OS_2;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_2,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_2,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (2U == os);
        }
        else if (3U == os)
        {
            p_ctx->temp_osr = DPS310_OS_4;
            p_ctx->pres_osr = DPS310_OS_4;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_4,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_4,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (4U == os);
        }
        else if (5U == os)
        {
            p_ctx->temp_osr = DPS310_OS_8;
            p_ctx->pres_osr = DPS310_OS_8;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_8,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_8,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (8U == os);
        }
        else if (9U == os)
        {
            p_ctx->temp_osr = DPS310_OS_16;
            p_ctx->pres_osr = DPS310_OS_16;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_16,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_16,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (16U == os);
        }
        else if (17U == os)
        {
            p_ctx->temp_osr = DPS310_OS_32;
            p_ctx->pres_osr = DPS310_OS_32;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_32,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_32,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (32U == os);
        }
        else if (33U == os)
        {
            p_ctx->temp_osr = DPS310_OS_64;
            p_ctx->pres_osr = DPS310_OS_64;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_64,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_64,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (64U == os);
        }
        else if (65U == os)
        {
            p_ctx->temp_osr = DPS310_OS_128;
            p_ctx->pres_osr = DPS310_OS_128;
            dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_128,
                                                DPS310_SUCCESS);
            dps310_config_temp_IgnoreArg_temp_mr();
            dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                                0,
                                                DPS310_OS_128,
                                                DPS310_SUCCESS);
            dps310_config_pres_IgnoreArg_pres_mr();
            ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (128U == os);
        }
        else
        {
            err_code = ri_dps310_dsp_set (&dsp_func, &os);
            TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
        }
    }
}

void test_ri_dps310_dsp_set_default (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t dsp_func  = RD_SENSOR_CFG_DEFAULT;
    uint8_t dsp_param = RD_SENSOR_CFG_DEFAULT;
    p_ctx->temp_osr = DPS310_OS_1;
    p_ctx->pres_osr = DPS310_OS_1;
    dps310_config_temp_ExpectAndReturn (dps_ctx.p_ctx,
                                        0,
                                        DPS310_OS_1,
                                        DPS310_SUCCESS);
    dps310_config_temp_IgnoreArg_temp_mr();
    dps310_config_pres_ExpectAndReturn (dps_ctx.p_ctx,
                                        0,
                                        DPS310_OS_1,
                                        DPS310_SUCCESS);
    dps310_config_pres_IgnoreArg_pres_mr();
    err_code = ri_dps310_dsp_set (&dsp_func, &dsp_param);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_DSP_LAST == dsp_func);
    TEST_ASSERT (1U == dsp_param);
}

// DSP_GET is tested by setters.

void test_ri_dps310_mode_set_single_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton ();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    static float temperature_c = SIM_TEMPERATURE_C;
    static float pressure_pa   = SIM_PRESSURE_PA;
    dps310_measure_temp_once_sync_ExpectAndReturn (p_ctx, NULL, DPS310_SUCCESS);
    dps310_measure_temp_once_sync_IgnoreArg_result ();
    dps310_measure_temp_once_sync_ReturnThruPtr_result (&temperature_c);
    dps310_measure_pres_once_sync_ExpectAndReturn (p_ctx, NULL, DPS310_SUCCESS);
    dps310_measure_pres_once_sync_IgnoreArg_result ();
    dps310_measure_pres_once_sync_ReturnThruPtr_result (&pressure_pa);
    rd_sensor_timestamp_get_ExpectAndReturn (SIM_TIME_MS);
    rd_sensor_data_set_Expect (NULL, RD_SENSOR_PRES_FIELD, SIM_PRESSURE_PA);
    rd_sensor_data_set_IgnoreArg_target();
    rd_sensor_data_set_Expect (NULL, RD_SENSOR_TEMP_FIELD, SIM_TEMPERATURE_C);
    rd_sensor_data_set_IgnoreArg_target();
    err_code = ri_dps310_mode_set (&mode);
    TEST_ASSERT (RD_SENSOR_CFG_SLEEP == mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_dps310_mode_set_continuous_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    dps310_measure_continuous_async_ExpectAndReturn (p_ctx, DPS310_SUCCESS);
    err_code = ri_dps310_mode_set (&mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    // Use a separate call to mode_get to simulate mode_set changing DPS internal state-
    p_ctx->device_status = DPS310_CONTINUOUS;
    err_code = ri_dps310_mode_get (&mode);
    TEST_ASSERT (RD_SENSOR_CFG_CONTINUOUS == mode);
}

void test_ri_dps310_data_get_no_data (void)
{
    // Run singleton test to initialize sensor context.
    test_ri_dps310_init_singleton();
    dps310_ctx_t * const p_ctx = (dps310_ctx_t *) dps_ctx.p_ctx;
    p_ctx->device_status = DPS310_READY;
    const rd_sensor_data_fields_t fields =
    {
        .datas.temperature_c = 1,
        .datas.pressure_pa = 1
    };
    float values[2];
    rd_sensor_data_t data =
    {
        .fields = fields,
        .data = values
    };
    rd_status_t err_code = ri_dps310_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (0 == data.valid.bitfield);
}

void test_ri_dps310_data_get_single (void)
{
    const rd_sensor_data_fields_t fields =
    {
        .datas.temperature_c = 1,
        .datas.pressure_pa = 1
    };
    float values[2];
    rd_sensor_data_t data =
    {
        .fields = fields,
        .data = values
    };
    // Simulate single sample being taken.
    test_ri_dps310_mode_set_single_ok();
    rd_sensor_data_populate_ExpectAnyArgs();
    rd_status_t err_code = ri_dps310_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_dps310_data_get_continuous (void)
{
    const rd_sensor_data_fields_t fields =
    {
        .datas.temperature_c = 1,
        .datas.pressure_pa = 1
    };
    float values[2];
    rd_sensor_data_t data =
    {
        .fields = fields,
        .data = values
    };
    static float temperature_c = SIM_TEMPERATURE_C;
    static float pressure_pa   = SIM_PRESSURE_PA;
    // Simulate continuous mode.
    test_ri_dps310_mode_set_continuous_ok();
    dps310_get_last_result_ExpectAnyArgsAndReturn (DPS310_SUCCESS);
    dps310_get_last_result_ReturnThruPtr_temp (&temperature_c);
    dps310_get_last_result_ReturnThruPtr_pres (&pressure_pa);
    rd_sensor_timestamp_get_ExpectAndReturn (SIM_TIME_MS);
    rd_sensor_data_set_Expect (NULL, RD_SENSOR_PRES_FIELD, SIM_PRESSURE_PA);
    rd_sensor_data_set_IgnoreArg_target();
    rd_sensor_data_set_Expect (NULL, RD_SENSOR_TEMP_FIELD, SIM_TEMPERATURE_C);
    rd_sensor_data_set_IgnoreArg_target();
    rd_sensor_data_populate_ExpectAnyArgs();
    rd_status_t err_code = ri_dps310_data_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}
