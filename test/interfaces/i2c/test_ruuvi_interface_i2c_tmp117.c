#ifdef TEST

#include "unity.h"

#include "ruuvi_interface_i2c_tmp117.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_interface_i2c.h"

#define MOCK_DEV_ID    (0x40U)
#define MOCK_REG_ADDR  (0x20U)
#define MOCK_REG_VAL   (0x1234U)
#define MOCK_REG_VAL_H (0x12U)
#define MOCK_REG_VAL_L (0x34U)

void setUp (void)
{
}

void tearDown (void)
{
}

/**
 * @brief I2C write function for TMP117
 *
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C addess of TMP117
 * @param[in] reg_addr TMP117 register address to write.
 * @param[in] reg_val 16-bit value to be written
 * @return RD_SUCCESS on success
 * @return RD_ERROR_TIMEOUT if device does not respond on bus
 **/
void test_ri_i2c_tmp117_write (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t array[3] = {MOCK_REG_ADDR, MOCK_REG_VAL_H, MOCK_REG_VAL_L};
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID,
            array,
            sizeof (array),
            sizeof (array),
            true,
            RD_SUCCESS);
    err_code |= ri_i2c_tmp117_write (MOCK_DEV_ID,
                                     MOCK_REG_ADDR,
                                     MOCK_REG_VAL);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ri_i2c_tmp117_write_timeout (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t array[3] = {MOCK_REG_ADDR, MOCK_REG_VAL_H, MOCK_REG_VAL_L};
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID,
            array,
            sizeof (array),
            sizeof (array),
            true,
            RD_ERROR_TIMEOUT);
    err_code |= ri_i2c_tmp117_write (MOCK_DEV_ID,
                                     MOCK_REG_ADDR,
                                     MOCK_REG_VAL);
    TEST_ASSERT (RD_ERROR_TIMEOUT == err_code);
}

/**
 * @brief I2C Read function for TMP117
 *
 * Binds Ruuvi Interface I2C functions for TMP117
 *
 * @param[in] dev_id @ref I2C interface handle, i.e. I2C addess of TMP117.
 * @param[in] reg_addr TMP117 register address to read.
 * @param[in] reg_val pointer to 16-bit data to be received.
 * @return RD_SUCCESS on success
 * @return RD_ERROR_TIMEOUT if device does not respond on bus
 **/
void test_ri_i2c_tmp117_read (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t array[3] = {MOCK_REG_ADDR, 0, 0};
    uint8_t ret_reg[2] = {MOCK_REG_VAL_H, MOCK_REG_VAL_L};;
    uint16_t reg = 0;
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID,
            array,
            1,
            1,
            false,
            RD_SUCCESS);
    ri_i2c_read_blocking_ExpectAndReturn (MOCK_DEV_ID,
                                          array,
                                          2,
                                          RD_SUCCESS);
    ri_i2c_read_blocking_IgnoreArg_p_rx();
    ri_i2c_read_blocking_ReturnArrayThruPtr_p_rx (ret_reg, 2);
    err_code |= ri_i2c_tmp117_read (MOCK_DEV_ID,
                                    MOCK_REG_ADDR,
                                    &reg);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (MOCK_REG_VAL == reg);
}

#endif // TEST
