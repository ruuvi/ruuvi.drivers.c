#include "unity.h"

#include "ruuvi_interface_lis2dh12.h"
#include "mock_lis2dh12_reg.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_spi_lis2dh12.h"
#include "mock_ruuvi_interface_yield.h"

#include <string.h>

static rd_sensor_t m_sensor;
static rd_sensor_t m_sensor_ni;
static const rd_bus_t m_bus = RD_BUS_SPI;
static const uint8_t m_handle = 1;
static stmdev_ctx_t m_ctx =
{
    .write_reg = &ri_spi_lis2dh12_write,
    .read_reg = &ri_spi_lis2dh12_read,
    .handle = &dev.handle
};

void setUp (void)
{
    memset (&m_sensor, 0, sizeof (m_sensor));
    memset (&m_sensor_ni, 0, sizeof (m_sensor_ni));
    m_ctx.write_reg = &ri_spi_lis2dh12_write;
    m_ctx.read_reg = &ri_spi_lis2dh12_read;
    m_ctx.handle = &dev.handle;
    rd_sensor_initialize_Expect (&m_sensor_ni);
    rd_sensor_initialize (&m_sensor_ni);
}

void tearDown (void)
{
    if (NULL != m_sensor.uninit)
    {
        rd_sensor_uninitialize_Expect (&m_sensor);
        lis2dh12_data_rate_set_ExpectAndReturn (& (dev.ctx), LIS2DH12_POWER_DOWN, RD_SUCCESS);
        m_sensor.uninit (&m_sensor, m_bus, m_handle);
    }
}

static void wai_ok (void)
{
    // Function takes pointer to a local variable, do not mock.
    static uint8_t wai = LIS2DH12_ID;
    lis2dh12_device_id_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_device_id_get_ReturnThruPtr_buff (&wai);
}

static void wai_diffrent (void)
{
    // Function takes pointer to a local variable, do not mock.
    static uint8_t wai = LIS2DH12_ID + 1;
    lis2dh12_device_id_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_device_id_get_ReturnThruPtr_buff (&wai);
}



static void fifo_use_ok (const bool use)
{
    static lis2dh12_fm_t mode;

    if (use) { mode = LIS2DH12_DYNAMIC_STREAM_MODE; }
    else { mode = LIS2DH12_BYPASS_MODE; }

    lis2dh12_fifo_set_ExpectAndReturn (& (dev.ctx), use, RD_SUCCESS);
    lis2dh12_fifo_mode_set_ExpectAndReturn (& (dev.ctx), mode, RD_SUCCESS);
}

static void fifo_interrupt_use_ok (const bool use)
{
    rd_status_t err_code = RD_SUCCESS;
    static lis2dh12_ctrl_reg3_t ctrl = { 0 };

    if (true == use)
    {
        // Setting the FTH [4:0] bit in the FIFO_CTRL_REG (2Eh) register to an N value,
        // the number of X, Y and Z data samples that should be read at the rise of the watermark interrupt is up to (N+1).
        lis2dh12_fifo_watermark_set_ExpectAndReturn (& (dev.ctx), 31, RD_SUCCESS);
        ctrl.i1_wtm = PROPERTY_ENABLE;
    }

    lis2dh12_pin_int1_config_set_ExpectWithArrayAndReturn (&m_ctx, 1, &ctrl, 1, RD_SUCCESS);
}

static void activity_interrupt_use_ok (const bool use, float * const ths)
{
    static lis2dh12_hp_t high_pass = LIS2DH12_ON_INT1_GEN;
    static lis2dh12_ctrl_reg6_t ctrl6 = { 0 };
    static lis2dh12_int1_cfg_t  cfg = { 0 };

    if (use)
    {
        //TODO
    }
    else
    {
        high_pass = LIS2DH12_DISC_FROM_INT_GENERATOR;
    }

    lis2dh12_high_pass_int_conf_set_ExpectAndReturn (& (dev.ctx), high_pass, RD_SUCCESS);
    lis2dh12_int1_gen_conf_set_ExpectWithArrayAndReturn (&m_ctx, 1, &cfg, 1, RD_SUCCESS);
    lis2dh12_pin_int2_config_set_ExpectWithArrayAndReturn (&m_ctx, 1, &ctrl6, 1, RD_SUCCESS);
}


static void clear_sensor_state_ok (void)
{
    fifo_use_ok (false);
    fifo_interrupt_use_ok (false);
    float ths = 0;
    activity_interrupt_use_ok (false, &ths);
    lis2dh12_temperature_meas_set_ExpectAndReturn (&dev.ctx, LIS2DH12_TEMP_ENABLE,
            RD_SUCCESS);
    lis2dh12_block_data_update_set_ExpectAndReturn (&dev.ctx, PROPERTY_ENABLE, RD_SUCCESS);
    lis2dh12_high_pass_on_outputs_set_ExpectAndReturn (&dev.ctx, PROPERTY_DISABLE,
            RD_SUCCESS);
}

static void clear_sensor_state_fail (void)
{
    fifo_use_ok (false);
    fifo_interrupt_use_ok (false);
    float ths = 0;
    activity_interrupt_use_ok (false, &ths);
    lis2dh12_temperature_meas_set_ExpectAndReturn (&dev.ctx, LIS2DH12_TEMP_ENABLE,
            RD_SUCCESS);
    lis2dh12_block_data_update_set_ExpectAndReturn (&dev.ctx, PROPERTY_ENABLE, RD_SUCCESS);
    lis2dh12_high_pass_on_outputs_set_ExpectAndReturn (&dev.ctx, PROPERTY_DISABLE,
            RD_ERROR_TIMEOUT);
}

static void selftest_ok (void)
{
    static uint8_t data_raw_acceleration_zero[6] = {0};
    static uint8_t data_raw_acceleration_pos[6] = {0xFF, 0x07, 0xFF, 0x07, 0xFF, 0x07};
    static uint8_t data_raw_acceleration_neg[6] = {0xFF, 0xE0, 0xFF, 0xE0, 0xFF, 0xE0};
    lis2dh12_data_rate_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ODR_400Hz, RD_SUCCESS);
    lis2dh12_full_scale_set_ExpectAndReturn (&dev.ctx, LIS2DH12_2g, RD_SUCCESS);
    lis2dh12_operating_mode_set_ExpectAndReturn (&dev.ctx, LIS2DH12_NM_10bit, RD_SUCCESS);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_DISABLE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_POSITIVE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_NEGATIVE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_DISABLE, RD_SUCCESS);
    lis2dh12_data_rate_set_ExpectAndReturn (&dev.ctx, LIS2DH12_POWER_DOWN, RD_SUCCESS);
}

static void selftest_zero_pos (void)
{
    static uint8_t data_raw_acceleration_zero[6] = {0};
    static uint8_t data_raw_acceleration_neg[6] = {0xFF, 0xE0, 0xFF, 0xE0, 0xFF, 0xE0};
    lis2dh12_data_rate_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ODR_400Hz, RD_SUCCESS);
    lis2dh12_full_scale_set_ExpectAndReturn (&dev.ctx, LIS2DH12_2g, RD_SUCCESS);
    lis2dh12_operating_mode_set_ExpectAndReturn (&dev.ctx, LIS2DH12_NM_10bit, RD_SUCCESS);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_DISABLE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_POSITIVE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_NEGATIVE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_neg, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_DISABLE, RD_SUCCESS);
    lis2dh12_data_rate_set_ExpectAndReturn (&dev.ctx, LIS2DH12_POWER_DOWN, RD_SUCCESS);
}

static void selftest_zero_neg (void)
{
    static uint8_t data_raw_acceleration_zero[6] = {0};
    static uint8_t data_raw_acceleration_pos[6] = {0xFF, 0x07, 0xFF, 0x07, 0xFF, 0x07};
    lis2dh12_data_rate_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ODR_400Hz, RD_SUCCESS);
    lis2dh12_full_scale_set_ExpectAndReturn (&dev.ctx, LIS2DH12_2g, RD_SUCCESS);
    lis2dh12_operating_mode_set_ExpectAndReturn (&dev.ctx, LIS2DH12_NM_10bit, RD_SUCCESS);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_DISABLE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_POSITIVE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_pos, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_NEGATIVE, RD_SUCCESS);
    ri_delay_ms_ExpectAndReturn (SELF_TEST_DELAY_MS, RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_acceleration_raw_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_acceleration_raw_get_ReturnArrayThruPtr_buff (data_raw_acceleration_zero, 6);
    lis2dh12_self_test_set_ExpectAndReturn (&dev.ctx, LIS2DH12_ST_DISABLE, RD_SUCCESS);
    lis2dh12_data_rate_set_ExpectAndReturn (&dev.ctx, LIS2DH12_POWER_DOWN, RD_SUCCESS);
}

void test_ruuvi_interface_lis2dh12_init_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (&m_sensor);
    ri_delay_ms_ExpectAndReturn (10U, RD_SUCCESS);
    wai_ok();
    clear_sensor_state_ok();
    selftest_ok();
    err_code |= ri_lis2dh12_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (!strcmp (m_sensor.name, "LIS2DH12"));
    TEST_ASSERT (m_sensor.init != m_sensor_ni.init);
}

void test_ruuvi_interface_lis2dh12_init_st_zero_pos (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (&m_sensor);
    ri_delay_ms_ExpectAndReturn (10U, RD_SUCCESS);
    wai_ok();
    clear_sensor_state_ok();
    selftest_zero_pos();
    rd_sensor_uninitialize_Expect (&m_sensor);
    err_code |= ri_lis2dh12_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT (RD_ERROR_SELFTEST == err_code);
    TEST_ASSERT (!strcmp (m_sensor.name, "LIS2DH12"));
    TEST_ASSERT (m_sensor.init == m_sensor_ni.init);
}

void test_ruuvi_interface_lis2dh12_init_st_zero_neg (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (&m_sensor);
    ri_delay_ms_ExpectAndReturn (10U, RD_SUCCESS);
    wai_ok();
    clear_sensor_state_ok();
    selftest_zero_neg();
    rd_sensor_uninitialize_Expect (&m_sensor);
    err_code |= ri_lis2dh12_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT (RD_ERROR_SELFTEST == err_code);
    TEST_ASSERT (!strcmp (m_sensor.name, "LIS2DH12"));
    TEST_ASSERT (m_sensor.init == m_sensor_ni.init);
}

void test_ruuvi_interface_lis2dh12_init_wai_fail (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (&m_sensor);
    ri_delay_ms_ExpectAndReturn (10U, RD_SUCCESS);
    wai_diffrent();
    rd_sensor_uninitialize_Expect (&m_sensor);
    err_code |= ri_lis2dh12_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT (RD_ERROR_NOT_FOUND == err_code);
    TEST_ASSERT (!strcmp (m_sensor.name, "LIS2DH12"));
    TEST_ASSERT (m_sensor.init == m_sensor_ni.init);
}

void test_ruuvi_interface_lis2dh12_init_bus_not_supported (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (&m_sensor);
    rd_sensor_uninitialize_Expect (&m_sensor);
    err_code |= ri_lis2dh12_init (&m_sensor, RD_BUS_PDM, m_handle);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
    TEST_ASSERT (!strcmp (m_sensor.name, "LIS2DH12"));
    TEST_ASSERT (m_sensor.init == m_sensor_ni.init);
}

void test_ruuvi_interface_lis2dh12_init_clear_fail (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_is_init_ExpectAnyArgsAndReturn (false);
    rd_sensor_initialize_Expect (&m_sensor);
    ri_delay_ms_ExpectAndReturn (10U, RD_SUCCESS);
    wai_ok();
    clear_sensor_state_fail();
    rd_sensor_uninitialize_Expect (&m_sensor);
    err_code |= ri_lis2dh12_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT (RD_ERROR_INTERNAL == err_code);
    TEST_ASSERT (!strcmp (m_sensor.name, "LIS2DH12"));
    TEST_ASSERT (m_sensor.init == m_sensor_ni.init);
}
