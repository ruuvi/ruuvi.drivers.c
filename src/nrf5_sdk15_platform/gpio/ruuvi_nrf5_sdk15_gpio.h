#ifndef RUUVI_NRF5_SDK15_GPIO_H
#define RUUVI_NRF5_SDK15_GPIO_H
#include "ruuvi_interface_gpio.h"
/**
 * @addtogroup GPIO
 * @{
 */
/**
* @file ruuvi_nrf5_sdk15_gpio.h
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2019-04-27
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
*
* Implementations for basic GPIO writes and reads on nRF5 SDK15.
*
*/

/**
 * @brief convert @ref ri_gpio_id_t to nRF GPIO.
 *
 * @param[in] pin Ruuvi pin to convert to platform
 * @return Nordic pin corresponding to Ruuvi pin.
 */
static inline uint8_t ruuvi_to_nrf_pin_map (const ri_gpio_id_t pin)
{
    return (pin >> 8U) + (pin & 0x1FU);
}

static inline ri_gpio_id_t nrf_to_ruuvi_pin (const uint8_t pin)
{
    return ( (pin & 0xE0U) << 3U) + (pin & 0x1FU);
}

/** @} */
#endif