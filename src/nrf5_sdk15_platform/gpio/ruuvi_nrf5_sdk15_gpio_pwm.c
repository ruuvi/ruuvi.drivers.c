#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_gpio_interrupt.h"
#if RUUVI_NRF5_SDK15_GPIO_INTERRUPT_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_pwm.h"

rd_status_t ri_gpio_pwm_init (/*TODO*/)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_gpio_pwm_uninit (/*TODO*/)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_gpio_pwm_start (const ri_gpio_id_t pin, const ri_gpio_mode_t mode,
                               float * const frequency, float * const duty_cycle)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_gpio_pwm_stop (const ri_gpio_id_t pin /*, TODO */)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

bool ri_gpio_pwm_is_init (void)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}
#endif
