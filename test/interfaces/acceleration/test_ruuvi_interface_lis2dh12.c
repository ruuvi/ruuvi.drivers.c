#include "unity.h"

#include "ruuvi_interface_lis2dh12.h"
#include "mock_lis2dh12_reg.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_driver_sensor.h"
#include "mock_ruuvi_interface_spi_lis2dh12.h"
#include "mock_ruuvi_interface_yield.h"

static rd_sensor_t m_sensor;
static const rd_bus_t m_bus = RD_BUS_SPI;
static const uint8_t m_handle = 1;
static uint8_t wai = LIS2DH12_ID;
static const stmdev_ctx_t m_ctx = 
{
    .write_reg = &ri_spi_lis2dh12_write,
    .read_reg = &ri_spi_lis2dh12_read,
    .handle = &dev.handle
};

void setUp (void)
{
    rd_sensor_t m_sensor = {0};
    wai = LIS2DH12_ID;
}

void tearDown (void)
{
    if (NULL != m_sensor.uninit)
    {
        m_sensor.uninit (&m_sensor, m_bus, m_handle);
    }
}

static rd_status_t wai_ok (void)
{
    // Function takes pointer to a local variable, do not mock.
    
    
    lis2dh12_device_id_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    lis2dh12_device_id_get_ReturnThruPtr_buff (&wai);
    return RD_SUCCESS;
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

    lis2dh12_pin_int1_config_set_ExpectWithArrayAndReturn(&m_ctx, 1, &ctrl, 1, RD_SUCCESS);
}

static void activity_interrupt_use_ok (const bool use, float * const ths)
{

}


static rd_status_t clear_sensor_state_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code = LIS_SUCCESS;
    // Disable FIFO, activity
    fifo_use_ok (false);
    fifo_interrupt_use_ok (false);
    float ths = 0;
    activity_interrupt_use_ok (false, &ths);
    // Enable temperature sensor
    lis2dh12_temperature_meas_set_ExpectAndReturn (&dev.ctx, LIS2DH12_TEMP_ENABLE,
            RD_SUCCESS);
    // Disable Block Data Update, allow values to update even if old is not read
    lis2dh12_block_data_update_set_ExpectAndReturn (&dev.ctx, PROPERTY_ENABLE, RD_SUCCESS);
    // Disable filtering
    lis2dh12_high_pass_on_outputs_set_ExpectAndReturn (&dev.ctx, PROPERTY_DISABLE,
            RD_SUCCESS);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }

    return err_code;
}

void test_ruuvi_interface_lis2dh12_init_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_initialize_Expect (&m_sensor);
    err_code |= wai_ok();
    err_code |= clear_sensor_state_ok();
    err_code |= ri_lis2dh12_init (&m_sensor, m_bus, m_handle);
    TEST_ASSERT (RD_SUCCESS == err_code);
}
