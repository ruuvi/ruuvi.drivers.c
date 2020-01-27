/**
 * @file test_task_led.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-01-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "unity.h"
#include "mock_ruuvi_driver_error.h"
#include "mock_ruuvi_interface_gpio.h"
#include "mock_ruuvi_interface_log.h"
#include "ruuvi_task_led.h"

#define LEDS_NUMBER 3U
static const ri_gpio_id_t leds[LEDS_NUMBER] = {3U, 6U, 9U};
static const ri_gpio_state_t leds_on[LEDS_NUMBER] = 
{
    RI_GPIO_LOW, 
    RI_GPIO_HIGH, 
    RI_GPIO_LOW
};
static size_t configured_leds;

void setUp (void)
{
    ri_gpio_is_init_ExpectAndReturn (true);

    for (size_t ii = 0U; ii < LEDS_NUMBER; ii++)
    {
        ri_gpio_configure_ExpectAndReturn (leds[ii],
                RI_GPIO_MODE_OUTPUT_HIGHDRIVE,
                RD_SUCCESS);
        ri_gpio_write_ExpectAndReturn (leds[ii], !leds_on[ii],
                RD_SUCCESS);
    }

    rt_led_init(leds, leds_on, LEDS_NUMBER);
    uint16_t led = rt_led_activity_led_get();
    configured_leds = LEDS_NUMBER;
    TEST_ASSERT (RI_GPIO_ID_UNUSED == led);
}

void tearDown (void)
{
    for (size_t ii = 0U; ii < configured_leds; ii++)
    {
        ri_gpio_configure_ExpectAndReturn (leds[ii],
                RI_GPIO_MODE_HIGH_Z,
                RD_SUCCESS);
    }

    rt_led_uninit();
    uint16_t led = rt_led_activity_led_get();
    TEST_ASSERT (RI_GPIO_ID_UNUSED == led);
    configured_leds = 0;
}

/**
 * @brief LED initialization function.
 * - Returns error if leds were already initialized.
 * - Initializes GPIO if GPIO wasn't initialized.
 * - returns error code if GPIO cannot be initialized
 * - Configures GPIOs as high-drive output and sets LEDs as inactive.
 * - Sets activity led to uninitialized
 *
 * @retval RD_SUCCESS if no errors occured.
 * @retval RD_ERROR_INVALID_STATE if leds were already initialized.
 * @retval error code from stack on other error.
 **/
/* Case: Success, GPIO was initialzed */
void test_rt_led_init_leds_success_gpio_was_init (void)
{
    tearDown();
    ri_gpio_is_init_ExpectAndReturn (true);

    for (size_t ii = 0U; ii < LEDS_NUMBER; ii++)
    {
        ri_gpio_configure_ExpectAndReturn (leds[ii],
                RI_GPIO_MODE_OUTPUT_HIGHDRIVE,
                RD_SUCCESS);
        ri_gpio_write_ExpectAndReturn (leds[ii], !leds_on[ii],
                RD_SUCCESS);
    }

    rt_led_init(leds, leds_on, LEDS_NUMBER);
    uint16_t led = rt_led_activity_led_get();
    TEST_ASSERT (RI_GPIO_ID_UNUSED == led);
    configured_leds = LEDS_NUMBER;
}

/* Case: Success, GPIO was not initialzed */
void test_rt_led_init_leds_gpio_not_init (void)
{
    tearDown();
    ri_gpio_is_init_ExpectAndReturn (false);
    ri_gpio_init_ExpectAndReturn (RD_SUCCESS);

    for (size_t ii = 0U; ii < LEDS_NUMBER; ii++)
    {
        ri_gpio_configure_ExpectAndReturn (leds[ii],
                RI_GPIO_MODE_OUTPUT_HIGHDRIVE,
                RD_SUCCESS);
        ri_gpio_write_ExpectAndReturn (leds[ii], !leds_on[ii],
                RD_SUCCESS);
    }

    rt_led_init(leds, leds_on, LEDS_NUMBER);
    uint16_t aled = rt_led_activity_led_get();
    TEST_ASSERT (RI_GPIO_ID_UNUSED == aled);
    configured_leds = LEDS_NUMBER;
}

/* Case: fail on second init, GPIO was not initialzed */
void test_rt_led_init_leds_twice_fails (void)
{
    tearDown();
    ri_gpio_is_init_ExpectAndReturn (false);
    ri_gpio_init_ExpectAndReturn (RD_SUCCESS);

    for (size_t ii = 0U; ii < LEDS_NUMBER; ii++)
    {
        ri_gpio_configure_ExpectAndReturn (leds[ii],
                RI_GPIO_MODE_OUTPUT_HIGHDRIVE,
                RD_SUCCESS);
        ri_gpio_write_ExpectAndReturn (leds[ii], !leds_on[ii],
                RD_SUCCESS);
    }

    rt_led_init(leds, leds_on, LEDS_NUMBER);
    rd_status_t error = rt_led_init(leds, leds_on, LEDS_NUMBER);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == error);
    configured_leds = LEDS_NUMBER;
}

/* Case: GPIO was not initialzed, GPIO init fails */
void test_rt_led_init_leds_gpio_fails (void)
{
    tearDown();
    ri_gpio_is_init_ExpectAndReturn (false);
    ri_gpio_init_ExpectAndReturn (RD_ERROR_INTERNAL);
    rd_status_t error = rt_led_init(leds, leds_on, LEDS_NUMBER);
    TEST_ASSERT (RD_ERROR_INTERNAL == error);
}

/**
 * @brief LED write function. Set given LED ON or OFF.
 *
 * @param[in] led  LED to change, use constant from RUUVI_BOARDS
 * @param[in] state  true to turn led on, false to turn led off.
 *
 * @retval @c RD_SUCCESS if value was written
 * @retval @c RUUVI_ERROR_INVALID_PARAM  if GPIO pin is not led.
 * @retval @c RD_ERROR_INVALID_STATE if GPIO task is not initialized.
 **/
void test_rt_led_write_not_init()
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    err_code |= rt_led_write (1, true);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_led_write_not_led()
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= rt_led_write (RI_GPIO_ID_UNUSED, false);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
}

void test_rt_led_write_valid()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_write_ExpectAndReturn (leds[0], leds_on[0], RD_SUCCESS);
    err_code |= rt_led_write (leds[0], true);
    // Note: Expecting GPIO to get active value when writing true.
    ri_gpio_write_ExpectAndReturn (leds[1], leds_on[1], RD_SUCCESS);
    err_code |= rt_led_write (leds[1], true);
    ri_gpio_write_ExpectAndReturn (leds[0], !leds_on[0], RD_SUCCESS);
    err_code |= rt_led_write (leds[0], false);
    // Note: Expecting GPIO to get inactive value when writing false.
    ri_gpio_write_ExpectAndReturn (leds[1], !leds_on[1], RD_SUCCESS);
    err_code |= rt_led_write (leds[1], false);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

/**
 * @brief Set LED which is used to indicate activity.
 *
 * This function can be called before GPIO or LEDs are initialized.
 * Call with RI_GPIO_ID_UNUSED to disable activity indication.
 *
 * @param[in] led LED to indicate activity with.
 *
 * @retval RD_SUCCESS if valid led was set.
 * @retval RD_ERROR_INVALID_PARAM if there is no pin in LED.
 */
void test_rt_led_activity_led_set_valid ()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_write_ExpectAndReturn (leds[0], leds_on[0], RD_SUCCESS);
    ri_gpio_write_ExpectAndReturn (leds[0], !leds_on[0], RD_SUCCESS);
    err_code |= rt_led_activity_led_set (leds[0]);
    rt_led_activity_indicate (true);
    rt_led_activity_indicate (false);
    uint16_t led = rt_led_activity_led_get();
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (leds[0] == led);
}

void test_rt_led_activity_led_set_unused ()
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= rt_led_activity_led_set (RI_GPIO_ID_UNUSED);
    rt_led_activity_indicate (true);
    rt_led_activity_indicate (false);
    uint16_t led = rt_led_activity_led_get();
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RI_GPIO_ID_UNUSED == led);
}

void test_rt_led_activity_led_set_invalid ()
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= rt_led_activity_led_set (2312);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
}

void test_rt_led_is_led ()
{
    rd_status_t err_code = RD_SUCCESS;
    bool valid_led = false;

    for (size_t ii = 0U; ii < LEDS_NUMBER; ii++)
    {
        TEST_ASSERT (is_led (leds[ii]) > -1);
    }

    TEST_ASSERT (-1 == is_led (RI_GPIO_ID_UNUSED));
}