/**
 * @addtogroup button_tasks
 */
/*@{*/
/**
 * @file ruuvi_task_button.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-01-25
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_task_button.h"
#if RT_BUTTON_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_task_button.h"
#include "ruuvi_task_gpio.h"

#include <stddef.h>

static rd_status_t init_input_check (const rt_button_init_t * const rt_init)
{
    rd_status_t err_code;

    if (NULL == rt_init)
    {
        err_code = RD_ERROR_NULL;
    }
    else if (NULL    == rt_init->p_button_pins
             || NULL == rt_init->p_button_active
             || NULL == rt_init->p_button_handlers)
    {
        err_code = RD_ERROR_NULL;
    }
    else if (false == rt_gpio_is_init())
    {
        err_code = RD_ERROR_INVALID_STATE;
    }
    else
    {
        err_code = RD_SUCCESS;
    }

    return err_code;
}

rd_status_t rt_button_init (const rt_button_init_t * const rt_init)
{
    rd_status_t err_code = init_input_check (rt_init);

    if (RD_SUCCESS == err_code)
    {
        for (size_t ii = 0; (ii < rt_init->num_buttons) && (RD_SUCCESS == err_code); ii++)
        {
            ri_gpio_mode_t mode = RI_GPIO_MODE_HIGH_Z;

            if (NULL == rt_init->p_button_handlers[ii])
            {
                err_code |= RD_ERROR_NULL;
            }

            if (RI_GPIO_HIGH == rt_init->p_button_active[ii])
            {
                mode = RI_GPIO_MODE_INPUT_PULLDOWN;
            }
            else if (RI_GPIO_LOW == rt_init->p_button_active[ii])
            {
                mode = RI_GPIO_MODE_INPUT_PULLUP;
            }
            else
            {
                err_code |= RD_ERROR_INVALID_PARAM;
            }

            if (RD_SUCCESS == err_code)
            {
                err_code = ri_gpio_interrupt_enable (rt_init->p_button_pins[ii],
                                                     RI_GPIO_SLOPE_TOGGLE, mode,
                                                     rt_init->p_button_handlers[ii]);
            }
        }

        if (RD_SUCCESS != err_code)
        {
            // Ignore any further errors, return original
            (void) rt_button_uninit (rt_init);
        }
    }

    return err_code;
}

rd_status_t rt_button_uninit (const rt_button_init_t * const rt_init)
{
    rd_status_t err_code = init_input_check (rt_init);

    if (RD_SUCCESS == err_code)
    {
        for (size_t ii = 0; (ii < rt_init->num_buttons) && (RD_SUCCESS == err_code); ii++)
        {
            err_code = ri_gpio_interrupt_disable (rt_init->p_button_pins[ii]);
        }
    }

    return err_code;
}

#else // RT_BUTTON_ENABLED
rd_status_t rt_button_init (const rt_button_init_t * const rt_init)
{
    return RD_ERROR_NOT_ENABLED;
}

rd_status_t rt_button_uninit (const rt_button_init_t * const rt_init)
{
    return RD_ERROR_NOT_ENABLED;
}
#endif
/*@}*/