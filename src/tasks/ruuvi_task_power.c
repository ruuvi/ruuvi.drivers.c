#include "ruuvi_driver_enabled_modules.h"
#if RT_POWER_ENABLED

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_power.h"
#include "task_power.h"

ruuvi_driver_status_t rt_power_dcdc_init (const ruuvi_interface_power_regulators_t regulators)
{
    return ruuvi_interface_power_regulators_enable (regulators);
}
#endif