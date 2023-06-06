#ifdef TEST

#include "unity.h"

#include "ruuvi_interface_i2c_sen5x_scd4x.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_interface_yield.h"
#include "mock_ruuvi_interface_i2c.h"

#define MOCK_DEV_ID    (0x40U)
#define MOCK_REG_ADDR  (0x20U)
#define MOCK_REG_VAL_H (0x12U)
#define MOCK_REG_VAL_L (0x34U)

void setUp (void)
{
}

void tearDown (void)
{
}

/**
 * @brief I2C write function for Sensirion SEN5X / SCD4X
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C address of the sensor
 * @param[in] reg_addr register address to write.
 * @param[in] reg_val 16-bit value to be written
 * @return RD_SUCCESS on success
 * @return RD_ERROR_TIMEOUT if device does not respond on bus
 **/
void test_sensirion_i2c_hal_write (void)
{
    const uint8_t array[] = {MOCK_REG_ADDR, MOCK_REG_VAL_H, MOCK_REG_VAL_L};
    const uint8_t data[] = {MOCK_REG_ADDR, MOCK_REG_VAL_H, MOCK_REG_VAL_L};
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID, array, sizeof (array),
            sizeof (array), true, RD_SUCCESS);
    const int8_t res = sensirion_i2c_hal_write (MOCK_DEV_ID, data, sizeof (data));
    TEST_ASSERT_EQUAL (0, res);
}

void test_sensirion_i2c_hal_write_timeout (void)
{
    const uint8_t array[] = {MOCK_REG_ADDR, MOCK_REG_VAL_H, MOCK_REG_VAL_L};
    const uint8_t data[] = {MOCK_REG_ADDR, MOCK_REG_VAL_H, MOCK_REG_VAL_L};
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID,
            array,
            sizeof (array),
            sizeof (array),
            true,
            RD_ERROR_TIMEOUT);
    const int8_t res = sensirion_i2c_hal_write (MOCK_DEV_ID, data, sizeof (data));
    TEST_ASSERT_EQUAL (1, res);
}

/**
 * @brief I2C Read function for Sensirion SEN5X / SCD4X
 *
 * Binds Ruuvi Interface I2C functions for Sensirion SEN5X / SCD4X
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C address of the sensor
 * @param[in] reg_addr register address to read.
 * @param[in] reg_val pointer to 16-bit data to be received.
 * @return RD_SUCCESS on success
 * @return RD_ERROR_TIMEOUT if device does not respond on bus
 **/
void test_sensirion_i2c_hal_read (void)
{
    uint8_t array[2] = {0, 0};
    uint8_t data[2] = {0, 0};
    uint8_t ret_reg[2] = {MOCK_REG_VAL_H, MOCK_REG_VAL_L};
    uint16_t reg = 0;
    ri_i2c_read_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID, array, sizeof (array),
            sizeof (array), RD_SUCCESS);
    ri_i2c_read_blocking_IgnoreArg_p_rx();
    ri_i2c_read_blocking_ReturnArrayThruPtr_p_rx (ret_reg, 2);
    const int8_t res = sensirion_i2c_hal_read (MOCK_DEV_ID, data, sizeof (data));
    TEST_ASSERT_EQUAL (0, res);
    TEST_ASSERT_EQUAL (MOCK_REG_VAL_H, data[0]);
    TEST_ASSERT_EQUAL (MOCK_REG_VAL_L, data[1]);
}

#endif // TEST
