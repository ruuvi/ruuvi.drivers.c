#include "unity.h"

#include "ruuvi_interface_spi_dps310.h"
#include "mock_ruuvi_interface_gpio.h"
#include "mock_ruuvi_interface_spi.h"

void setUp (void)
{
}

void tearDown (void)
{
}

/**
 * @brief SPI write function for DPS310
 *
 * Binds Ruuvi Interface SPI functions into Ruuvi's DPS310 driver.
 * Handles GPIO chip select, there is no forced delay to let the CS settle.
 *
 * @param[in] dev_id @ref SPI interface handle, i.e. pin number of the chip select pin of DPS310.
 * @param[in] reg_addr DPS310 register address to write.
 * @param[in] p_reg_data pointer to data to be written.
 * @param[in] len length of data to be written.
 **/
void test_ri_spi_dps310_write (void)
{
    uint32_t gpio_handle = 33U;
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t reg_addr = 0x01U;
    ri_gpio_write_ExpectAndReturn (0x101U, RI_GPIO_LOW, RD_SUCCESS);
    ri_spi_xfer_blocking_ExpectWithArrayAndReturn (&reg_addr, 1, 1, NULL, 0, 0, RD_SUCCESS);
    ri_spi_xfer_blocking_ExpectWithArrayAndReturn (data, 8, 8, NULL, 0, 0, RD_SUCCESS);
    ri_gpio_write_ExpectAndReturn (0x101U, RI_GPIO_HIGH, RD_SUCCESS);
    uint32_t retval = ri_spi_dps310_write (&gpio_handle, reg_addr, data, sizeof (data));
    TEST_ASSERT (0 == retval);
}
/**
 * @brief SPI Read function for DPS310
 *
 * Binds Ruuvi Interface SPI functions into Ruuvi DPS310 driver.
 * Handles GPIO chip select, there is no forced delay to let the CS settle.
 *
 * @param[in] dev_id @ref SPI interface handle, i.e. pin number of the chip select pin of DPS310.
 * @param[in] reg_addr DPS310 register address to read.
 * @param[in] p_reg_data pointer to data to be received.
 * @param[in] len length of data to be received.
 **/
void test_ri_spi_dps310_read (void)
{
    uint8_t gpio_handle = 33U;
    uint8_t data[8] = {0};
    uint8_t reg_addr = 0x01U;
    uint8_t read_cmd = 0x81U;
    ri_gpio_write_ExpectAndReturn (0x101U, RI_GPIO_LOW, RD_SUCCESS);
    ri_spi_xfer_blocking_ExpectWithArrayAndReturn (&read_cmd, 1, 1, NULL, 0, 0, RD_SUCCESS);
    ri_spi_xfer_blocking_ExpectAndReturn (NULL, 0, data, 8, RD_SUCCESS);
    ri_gpio_write_ExpectAndReturn (0x101U, RI_GPIO_HIGH, RD_SUCCESS);
    uint32_t retval = ri_spi_dps310_read (&gpio_handle, reg_addr, data, sizeof (data));
    TEST_ASSERT (0 == retval);
}
/** @} */