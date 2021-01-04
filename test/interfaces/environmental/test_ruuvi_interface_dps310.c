#include "unity.h"

#include "mock_dps310.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_interface_spi_dps310.h"
#include "mock_ruuvi_interface_yield.h"
#include "ruuvi_interface_dps310.h"

#include "string.h"

rd_sensor_t dps_ctx;
dps310_ctx_t init_ctx =
{
    .write = &ri_spi_dps310_write,
    .read  = &ri_spi_dps310_read,
    .sleep = &ri_delay_ms
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

void test_ri_dps310_init_null (void)
{
    rd_status_t err_code = ri_dps310_init (NULL, RD_BUS_SPI, 0);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

#if 0
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_dps310_uninit (rd_sensor_t *
                              environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_scale_get (uint8_t * scale);
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