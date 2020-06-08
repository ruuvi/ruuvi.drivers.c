#include "unity.h"

#include "ruuvi_driver_error.h"
#include "ruuvi_task_gpio.h"
#include "mock_ruuvi_interface_gpio.h"
#include "mock_ruuvi_interface_gpio_interrupt.h"
#include "mock_ruuvi_interface_log.h"

void setUp (void)
{
    ri_log_Ignore();
    ri_error_to_string_IgnoreAndReturn (0);
}

void tearDown (void)
{
}

/**
 * @brief initialise GPIO. Pins are in high-Z state by default.
 *
 * @return RUUVI_DRIVER_SUCCESS
 */
void test_ruuvi_task_gpio_init_ok (void)
{
    ri_gpio_is_init_ExpectAndReturn (false);
    ri_gpio_init_ExpectAndReturn (RD_SUCCESS);
    ri_gpio_interrupt_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_status_t err_code = rt_gpio_init ();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_ruuvi_task_gpio_init_twice (void)
{
    ri_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_is_init_ExpectAndReturn (true);
    rd_status_t err_code = rt_gpio_init ();
    TEST_ASSERT (RD_SUCCESS == err_code);
}



/**
 * @brief check that GPIO is initialized.
 *
 * @return True if GPIO is initialized, false otherwise.
 */
void rt_gpio_is_init_yes (void)
{
    ri_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_is_init_ExpectAndReturn (true);
    TEST_ASSERT (rt_gpio_init());
}

void rt_gpio_is_init_no (void)
{
    ri_gpio_is_init_ExpectAndReturn (false);
    TEST_ASSERT (!rt_gpio_init());
}

void rt_gpio_is_init_no_int (void)
{
    ri_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_is_init_ExpectAndReturn (false);
    TEST_ASSERT (!rt_gpio_init());
}