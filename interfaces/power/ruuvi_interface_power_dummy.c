#include "ruuvi_driver_enabled_modules.h"
#if (!APPLICATION_POWER_ENABLED)
/**
 * @addtogroup Power
 * @{
 */

/**
 * @file ruuvi_interface_power_dummy.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-01-31
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Dummy functions which implement interface to allow using partial implementations of platforms.
 *
 */

void ruuvi_interface_power_reset(void)
{
}

/** @} */
#endif