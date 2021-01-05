#include "unity.h"

#include "mock_dps310.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_spi_dps310.h"
#include "mock_ruuvi_interface_yield.h"
#include "ruuvi_interface_dps310.h"

#include "string.h"

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
    dps_ctx.p_ctx = &init_ctx;
    rd_sensor_is_init_ExpectAndReturn (&dps_ctx, false);
    dps310_init_ExpectAndReturn (&init_ctx, DPS310_SUCCESS);
    rd_sensor_initialize_Expect (&dps_ctx);
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
}

void test_ri_dps310_init_singleton (void)
{
    rd_sensor_is_init_ExpectAndReturn (&dps_ctx, false);
    dps310_init_ExpectAndReturn (NULL, DPS310_SUCCESS);
    dps310_init_IgnoreArg_ctx();
    rd_sensor_initialize_Expect (&dps_ctx);
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

    for (uint8_t rate = 0U; rate < 129U; rate ++)
    {
        // DEFAULT rate, 1/s
        if (0U == rate)
        {
            p_ctx->temp_mr = 1U;
            p_ctx->pres_mr = 1U;
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

#if 0
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_dps310_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_dps310_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_mode_set (uint8_t * mode);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_mode_get (uint8_t * mode);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_dps310_data_get (rd_sensor_data_t * const data);
#endif