/**
 * @addtogroup button_tasks
 */
/*@{*/
/**
 * @file task_button.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-01-25
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#if RT_BUTTON_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_task_button.h"

#include <stddef.h>

rd_status_t rt_button_init (const rt_button_init_t* const rt_init)
{
    rd_status_t err_code = RD_SUCCESS;
    /*
    ri_gpio_slope_t slope = RI_GPIO_SLOPE_TOGGLE;
    ri_gpio_mode_t  mode  = RI_GPIO_MODE_INPUT_PULLDOWN;

    if (RI_GPIO_LOW == rt_init->p_button_active[0])
    {
        mode  = RI_GPIO_MODE_INPUT_PULLUP;
    }

    err_code |= ri_gpio_interrupt_enable (rt_init->p_button_pins[0], slope,
                mode, rt_init->p_button_handlers[0]);
                */
    return err_code;
}

#else // RT_BUTTON_ENABLED
rd_status_t rt_button_init (const rt_button_init_t* const rt_init)
{
    return RD_SUCCESS;
}
#endif
/*@}*/