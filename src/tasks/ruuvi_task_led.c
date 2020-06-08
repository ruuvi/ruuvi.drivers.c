/**
  * @addtogroup led_tasks
  */
/*@{*/
/**
 * @file task_led.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-11-19
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */
#include "ruuvi_driver_enabled_modules.h"
#if RT_LED_ENABLED
#include "ruuvi_task_led.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include <stddef.h>

static uint16_t m_activity_led;
static bool     m_initialized;
static ri_gpio_id_t    m_led_list[RT_MAX_LED_CFG];
static ri_gpio_state_t m_led_active_state[RT_MAX_LED_CFG];
//Store user configured led number in case there's extra space in array.
static size_t          m_num_leds;

#ifndef CEEDLING
static
#endif
int8_t is_led (const ri_gpio_id_t led)
{
    int8_t led_valid = -1;

    for (size_t ii = 0U;
            (ii < m_num_leds) && (0 > led_valid);
            ii++)
    {
        if (led == m_led_list[ii])
        {
            led_valid = ii;
        }
    }

    return led_valid;
}

static inline ri_gpio_state_t led_to_pin_state (ri_gpio_id_t led, bool active)
{
    ri_gpio_state_t state = RI_GPIO_LOW;
    int8_t index = is_led (led);

    if (index > -1)
    {
        if (RI_GPIO_HIGH == m_led_active_state[index])
        {
            state = active ? RI_GPIO_HIGH : RI_GPIO_LOW;
        }
        else
        {
            state = active ? RI_GPIO_LOW : RI_GPIO_HIGH;
        }
    }

    return state;
}

rd_status_t rt_led_init (const uint16_t * const leds,
                         const ri_gpio_state_t * const active_states,
                         const size_t num_leds)
{
    rd_status_t err_code = RD_SUCCESS;

    if (m_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        if (!ri_gpio_is_init())
        {
            err_code |= ri_gpio_init();
        }
    }

    if (RD_SUCCESS == err_code)
    {
        for (size_t ii = 0u; ii < num_leds; ii++)
        {
            m_led_list[ii] = leds[ii];
            m_led_active_state[ii] = active_states[ii];
            err_code |= ri_gpio_configure (m_led_list[ii],
                                           RI_GPIO_MODE_OUTPUT_HIGHDRIVE);
            err_code |= ri_gpio_write (m_led_list[ii], !m_led_active_state[ii]);
        }

        m_activity_led = RI_GPIO_ID_UNUSED;
        m_num_leds = num_leds;
        m_initialized = true;
    }

    return err_code;
}

rd_status_t rt_led_uninit (void)
{
    rd_status_t err_code = RD_SUCCESS;

    for (size_t ii = 0U; ii < m_num_leds; ii++)
    {
        err_code |= ri_gpio_configure (m_led_list[ii], RI_GPIO_MODE_HIGH_Z);
    }

    m_activity_led = RI_GPIO_ID_UNUSED;
    m_initialized = false;
    m_num_leds = 0;
    return err_code;
}

rd_status_t rt_led_write (const uint16_t led, const bool active)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!m_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (0 > is_led (led))
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }
    else
    {
        const ri_gpio_state_t state = led_to_pin_state (led, active);
        err_code |= ri_gpio_write (led, state);
    }

    return err_code;
}

void rt_led_activity_indicate (const bool active)
{
    // Error code cannot be returned, ignore
    (void) rt_led_write (m_activity_led, active);
}

rd_status_t rt_led_activity_led_set (uint16_t led)
{
    rd_status_t err_code = RD_SUCCESS;

    if (RI_GPIO_ID_UNUSED == led)
    {
        m_activity_led = RI_GPIO_ID_UNUSED;
    }
    else if (0 > is_led (led))
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }
    else
    {
        m_activity_led = led;
    }

    return err_code;
}

uint16_t rt_led_activity_led_get (void)
{
    return m_activity_led;
}
#endif
/*@}*/
