#include "unity.h"

#include "ruuvi_task_sensor.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_task_flash.h"

void setUp (void)
{
    ri_log_Ignore();
    ri_log_sensor_configuration_Ignore();
}

void tearDown (void)
{
}

rd_status_t mock_configuration_ok (const rd_sensor_t * const p_sensor,
                                   rd_sensor_configuration_t * const p_configuration)
{
    return RD_SUCCESS;
}

rd_status_t mock_configuration_error (const rd_sensor_t * const p_sensor,
                                      rd_sensor_configuration_t * const p_configuration)
{
    return RD_ERROR_INVALID_PARAM;
}

/**
 * @brief Configure a sensor with given settings.
 *
 * @param[in,out] sensor In: Sensor to configure.
                         Out: Sensor->configuration will be set to actual configuration.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL if sensor is NULL.
 * @retval error code from sensor on other error.
 */
void test_rt_sensor_configure_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_t sensor =
    {
        .configuration_set = mock_configuration_ok
    };
    rt_sensor_ctx_t ctx =
    {
        .sensor = sensor
    };
    rd_sensor_is_init_ExpectAndReturn (& (ctx.sensor), true);
    err_code = rt_sensor_configure (&ctx);
    TEST_ASSERT (RD_SUCCESS == err_code);
}


void test_rt_sensor_configure_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_sensor_configure (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_sensor_configure_sensor_error (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_t sensor =
    {
        .configuration_set = mock_configuration_error
    };
    rt_sensor_ctx_t ctx =
    {
        .sensor = sensor
    };
    rd_sensor_is_init_ExpectAndReturn (& (ctx.sensor), true);
    err_code = rt_sensor_configure (&ctx);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
}

void test_rt_sensor_configure_sensor_not_innit (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_t sensor =
    {
        .configuration_set = mock_configuration_ok
    };
    rt_sensor_ctx_t ctx =
    {
        .sensor = sensor
    };
    rd_sensor_is_init_ExpectAndReturn (& (ctx.sensor), false);
    err_code = rt_sensor_configure (&ctx);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}
