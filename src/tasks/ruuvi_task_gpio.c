#include "ruuvi_driver_enabled_modules.h"
#if RT_GPIO_ENABLED
#include "ruuvi_task_gpio.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"

static ri_gpio_interrupt_fp_t interrupt_table[RT_GPIO_INT_TABLE_SIZE]
    = {0}; //!< Stores interrupts associated with GPIO events.

rd_status_t rt_gpio_init()
{
    rd_status_t err_code = RD_SUCCESS;

    if (!rt_gpio_is_init())
    {
        err_code |= ri_gpio_init();
        err_code |= ri_gpio_interrupt_init (interrupt_table, sizeof (interrupt_table));
    }

    return err_code;
}

bool rt_gpio_is_init()
{
    return ri_gpio_is_init() && ri_gpio_interrupt_is_init();
}
#endif