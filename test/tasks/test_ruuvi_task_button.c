#include "unity.h"

#include "ruuvi_driver_enabled_modules.h"

#include "ruuvi_task_button.h"
#include "mock_ruuvi_task_gpio.h"
#include "mock_ruuvi_interface_gpio.h"
#include "mock_ruuvi_interface_gpio_interrupt.h"
#include "mock_ruuvi_interface_log.h"

#include <stddef.h>

static bool m_b1_pressed;
static bool m_b2_pressed;
#define CEED_PIN1 10U
#define CEED_PIN2 11U
#define ACTIVE_STATE1 RI_GPIO_LOW
#define ACTIVE_STATE2 RI_GPIO_HIGH

static const ri_gpio_evt_t evt1 =
{
    .pin = CEED_PIN1,
    .slope = RI_GPIO_SLOPE_HITOLO
};

static void ceedling_cb1 (const ri_gpio_evt_t evt)
{
    m_b1_pressed = true;
}

static void ceedling_cb2 (const ri_gpio_evt_t evt)
{
    m_b2_pressed = true;
}

static const ri_gpio_id_t button_pins[] = { CEED_PIN1, CEED_PIN2 };
static const ri_gpio_state_t button_active[] = { ACTIVE_STATE1, ACTIVE_STATE2 };
static const rt_button_fp_t button_handlers[] = { ceedling_cb1, ceedling_cb2 };
static const rt_button_init_t init_data =
{
    .p_button_pins = button_pins,
    .p_button_active = button_active,
    .p_button_handlers = button_handlers,
    .num_buttons = 2
};

void setUp (void)
{
    ri_log_Ignore();
}

void tearDown (void)
{
    m_b1_pressed = false;
    m_b2_pressed = false;
}

/**
 * @brief Button initialization function.
 *
 * Requires GPIO and interrupts to be initialized.
 * Configures GPIO as pullup/-down according to button active state.
 *
 * @param[in] rt_init Initializatoin structure for button task.
 *
 * @retval RD_SUCCESS if buttons were initialized
 * @retval RD_ERROR_NULL if any array of rt_init is NULL or any element of
                         initialization array is NULL.
 * @retval RD_ERROR_INVALID_STATE if GPIO or GPIO interrupts aren't initialized.
 *
 * @warning behaviour is undefined if lengths of arrays don't match num_buttons.
 **/
void test_rt_button_init_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_slope_t slope = RI_GPIO_SLOPE_TOGGLE;
    ri_gpio_mode_t  mode1 = (ACTIVE_STATE1) ? RI_GPIO_MODE_INPUT_PULLDOWN :
                            RI_GPIO_MODE_INPUT_PULLUP;
    ri_gpio_mode_t  mode2 = (ACTIVE_STATE2) ? RI_GPIO_MODE_INPUT_PULLDOWN :
                            RI_GPIO_MODE_INPUT_PULLUP;
    rt_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_enable_ExpectAndReturn (CEED_PIN1, slope, mode1,
            (init_data.p_button_handlers[0]),
            RD_SUCCESS);
    ri_gpio_interrupt_enable_ExpectAndReturn (CEED_PIN2, slope, mode2,
            (init_data.p_button_handlers[1]),
            RD_SUCCESS);
    err_code = rt_button_init (&init_data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_button_init_twice (void)
{
    test_rt_button_init_ok();
    test_rt_button_init_ok();
}

void test_rt_button_init_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_slope_t slope = RI_GPIO_SLOPE_TOGGLE;
    ri_gpio_mode_t  mode1 = (ACTIVE_STATE1) ? RI_GPIO_MODE_INPUT_PULLDOWN :
                            RI_GPIO_MODE_INPUT_PULLUP;
    ri_gpio_mode_t  mode2 = (ACTIVE_STATE2) ? RI_GPIO_MODE_INPUT_PULLDOWN :
                            RI_GPIO_MODE_INPUT_PULLUP;
    err_code = rt_button_init (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
    err_code = rt_button_init (& (rt_button_init_t)
    {
        .p_button_pins = NULL,
        .p_button_active = button_active,
        .p_button_handlers = button_handlers,
        .num_buttons = 2
    });
    TEST_ASSERT (RD_ERROR_NULL == err_code);
    err_code = rt_button_init (& (rt_button_init_t)
    {
        .p_button_pins = button_pins,
        .p_button_active = NULL,
        .p_button_handlers = button_handlers,
        .num_buttons = 2
    });
    TEST_ASSERT (RD_ERROR_NULL == err_code);
    err_code = rt_button_init (& (rt_button_init_t)
    {
        .p_button_pins = button_pins,
        .p_button_active = button_active,
        .p_button_handlers = NULL,
        .num_buttons = 2
    });
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_button_init_no_gpio (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_slope_t slope = RI_GPIO_SLOPE_TOGGLE;
    ri_gpio_mode_t  mode1 = (ACTIVE_STATE1) ? RI_GPIO_MODE_INPUT_PULLDOWN :
                            RI_GPIO_MODE_INPUT_PULLUP;
    ri_gpio_mode_t  mode2 = (ACTIVE_STATE2) ? RI_GPIO_MODE_INPUT_PULLDOWN :
                            RI_GPIO_MODE_INPUT_PULLUP;
    rt_gpio_is_init_ExpectAndReturn (false);
    err_code = rt_button_init (&init_data);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_button_init_invalid_params (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gpio_slope_t slope = RI_GPIO_SLOPE_TOGGLE;
    ri_gpio_mode_t  mode1 = (ACTIVE_STATE1) ? RI_GPIO_MODE_INPUT_PULLDOWN :
                            RI_GPIO_MODE_INPUT_PULLUP;
    ri_gpio_mode_t  mode2 = (ACTIVE_STATE2) ? RI_GPIO_MODE_INPUT_PULLDOWN :
                            RI_GPIO_MODE_INPUT_PULLUP;
    const ri_gpio_id_t invalid_pins[] = { CEED_PIN1, 0xFFFF };
    const ri_gpio_state_t invalid_active[] = { ACTIVE_STATE1, 87 };
    const rt_button_fp_t invalid_handlers[] = { ceedling_cb1, NULL };
    rt_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_enable_ExpectAndReturn (CEED_PIN1, slope, mode1,
            (init_data.p_button_handlers[0]),
            RD_SUCCESS);
    ri_gpio_interrupt_enable_ExpectAndReturn (0xFFFF, slope, mode2,
            (init_data.p_button_handlers[1]),
            RD_ERROR_INVALID_PARAM);
    rt_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_disable_ExpectAndReturn (CEED_PIN1, RD_SUCCESS);
    ri_gpio_interrupt_disable_ExpectAndReturn (0xFFFF, RD_ERROR_INVALID_PARAM);
    rt_button_init_t init1 =
    {
        .p_button_pins = invalid_pins,
        .p_button_active = button_active,
        .p_button_handlers = button_handlers,
        .num_buttons = 2
    };
    err_code = rt_button_init (&init1);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
    rt_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_enable_ExpectAndReturn (CEED_PIN1, slope, mode1,
            (init_data.p_button_handlers[0]),
            RD_SUCCESS);
    rt_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_disable_ExpectAndReturn (CEED_PIN1, RD_SUCCESS);
    ri_gpio_interrupt_disable_ExpectAndReturn (CEED_PIN2, RD_SUCCESS);
    rt_button_init_t init2 =
    {
        .p_button_pins = button_pins,
        .p_button_active = invalid_active,
        .p_button_handlers = button_handlers,
        .num_buttons = 2
    };
    err_code = rt_button_init (&init2);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
    rt_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_enable_ExpectAndReturn (CEED_PIN1, slope, mode1,
            (init_data.p_button_handlers[0]),
            RD_SUCCESS);
    rt_gpio_is_init_ExpectAndReturn (true);
    ri_gpio_interrupt_disable_ExpectAndReturn (CEED_PIN1, RD_SUCCESS);
    ri_gpio_interrupt_disable_ExpectAndReturn (CEED_PIN2, RD_SUCCESS);
    rt_button_init_t init3 =
    {
        .p_button_pins = button_pins,
        .p_button_active = button_active,
        .p_button_handlers = invalid_handlers,
        .num_buttons = 2
    };
    err_code = rt_button_init (&init3);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}