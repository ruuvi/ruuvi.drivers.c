#include "unity.h"

#include "mmc5616wa.h"
#include "ruuvi_interface_i2c_mmc5616wa.h"
#include "mock_ruuvi_interface_i2c.h"

#define MOCK_DEV_ID   (0x30U)
#define MOCK_REG_ADDR (0x1BU)

static uint8_t m_handle;

void setUp (void)
{
    m_handle = MOCK_DEV_ID;
}


void tearDown (void)
{
}

void test_ri_i2c_mmc5616wa_write_ok (void)
{
    const uint8_t reg_data[] = {0x12U, 0x34U};
    uint8_t tx[] = {MOCK_REG_ADDR, 0x12U, 0x34U};
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID,
            tx,
            sizeof (tx),
            sizeof (tx),
            true,
            RD_SUCCESS);
    TEST_ASSERT_EQUAL_INT32 (MMC5616WA_OK,
                             ri_i2c_mmc5616wa_write (&m_handle, MOCK_REG_ADDR,
                                     reg_data, sizeof (reg_data)));
}

void test_ri_i2c_mmc5616wa_write_i2c_error (void)
{
    const uint8_t reg_data[] = {0x12U};
    uint8_t tx[] = {MOCK_REG_ADDR, 0x12U};
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID,
            tx,
            sizeof (tx),
            sizeof (tx),
            true,
            RD_ERROR_TIMEOUT);
    TEST_ASSERT_EQUAL_INT32 (MMC5616WA_E_IO,
                             ri_i2c_mmc5616wa_write (&m_handle, MOCK_REG_ADDR,
                                     reg_data, sizeof (reg_data)));
}

void test_ri_i2c_mmc5616wa_write_null (void)
{
    const uint8_t reg_data[] = {0x12U};
    TEST_ASSERT_EQUAL_INT32 (MMC5616WA_E_NULL,
                             ri_i2c_mmc5616wa_write (NULL, MOCK_REG_ADDR,
                                     reg_data, sizeof (reg_data)));
    TEST_ASSERT_EQUAL_INT32 (MMC5616WA_E_NULL,
                             ri_i2c_mmc5616wa_write (&m_handle, MOCK_REG_ADDR,
                                     NULL, sizeof (reg_data)));
}

void test_ri_i2c_mmc5616wa_read_ok (void)
{
    uint8_t rx[3] = {0};
    const uint8_t returned[] = {0xAAU, 0xBBU, 0xCCU};
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID,
            & (uint8_t) {MOCK_REG_ADDR},
            1U,
            1U,
            false,
            RD_SUCCESS);
    ri_i2c_read_blocking_ExpectAndReturn (MOCK_DEV_ID, rx, sizeof (rx), RD_SUCCESS);
    ri_i2c_read_blocking_IgnoreArg_p_rx();
    ri_i2c_read_blocking_ReturnArrayThruPtr_p_rx (returned, sizeof (returned));
    TEST_ASSERT_EQUAL_INT32 (MMC5616WA_OK,
                             ri_i2c_mmc5616wa_read (&m_handle, MOCK_REG_ADDR,
                                     rx, sizeof (rx)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY (returned, rx, sizeof (returned));
}

void test_ri_i2c_mmc5616wa_read_write_error (void)
{
    uint8_t rx[1] = {0};
    ri_i2c_write_blocking_ExpectWithArrayAndReturn (MOCK_DEV_ID,
            & (uint8_t) {MOCK_REG_ADDR},
            1U,
            1U,
            false,
            RD_ERROR_TIMEOUT);
    TEST_ASSERT_EQUAL_INT32 (MMC5616WA_E_IO,
                             ri_i2c_mmc5616wa_read (&m_handle, MOCK_REG_ADDR,
                                     rx, sizeof (rx)));
}

void test_ri_i2c_mmc5616wa_read_null (void)
{
    uint8_t rx[1] = {0};
    TEST_ASSERT_EQUAL_INT32 (MMC5616WA_E_NULL,
                             ri_i2c_mmc5616wa_read (NULL, MOCK_REG_ADDR,
                                     rx, sizeof (rx)));
    TEST_ASSERT_EQUAL_INT32 (MMC5616WA_E_NULL,
                             ri_i2c_mmc5616wa_read (&m_handle, MOCK_REG_ADDR,
                                     NULL, sizeof (rx)));
}