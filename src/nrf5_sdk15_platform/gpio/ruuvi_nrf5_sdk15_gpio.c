#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_gpio.h"
#if (RUUVI_NRF5_SDK15_GPIO_ENABLED || DOXYGEN || CEEDLING)

#include "ruuvi_interface_gpio.h"
#include "ruuvi_driver_error.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include <stdbool.h>

/**
 * @addtogroup GPIO
 * @{
 */
/**
* @file ruuvi_nrf5_sdk15_gpio.c
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2020-01-21
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
*
* Implementations for basic GPIO writes and reads on nRF5 SDK15.
*
*/

/** @brief flag to keep track on if GPIO is initialized */
static bool m_gpio_is_init = false;

/**
 * @brief convert @ref ri_gpio_id_t to nRF GPIO.
 */
static inline uint8_t ruuvi_to_nrf_pin_map (const ri_gpio_id_t pin)
{
    return ( (pin >> 3U) & 0xE0U) + (pin & 0x1FU);
}

/**
 * @brief convert nRF GPIO to @ref ri_gpio_id_t.
 */
static inline ri_gpio_id_t nrf_to_ruuvi_pin (nrf_drv_gpiote_pin_t pin)
{
    return ( (pin >> 5) << 8) + (pin & 0x1F);
}

rd_status_t ri_gpio_init (void)
{
    if (m_gpio_is_init) { return RD_ERROR_INVALID_STATE; }

    m_gpio_is_init = true;
    return RD_SUCCESS;
}

rd_status_t ri_gpio_uninit (void)
{
    rd_status_t status = RD_SUCCESS;

    if (false == m_gpio_is_init)
    {
        return RD_SUCCESS;
    }

    // Number of pins is defined by nrf_gpio.h
    for (uint8_t iii = 0; iii < NUMBER_OF_PINS; iii++)
    {
        ri_gpio_id_t pin = nrf_to_ruuvi_pin (iii);
        status |= ri_gpio_configure (pin, RI_GPIO_MODE_HIGH_Z);
    }

    m_gpio_is_init = false;
    return status;
}

bool  ri_gpio_is_init (void)
{
    return m_gpio_is_init;
}

rd_status_t ri_gpio_configure (const ri_gpio_id_t pin,
                               const ri_gpio_mode_t mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (RI_GPIO_ID_UNUSED != pin)
    {
        const uint8_t nrf_pin = ruuvi_to_nrf_pin_map (pin);

        switch (mode)
        {
            case RI_GPIO_MODE_HIGH_Z:
                nrf_gpio_cfg_default (nrf_pin);
                break;

            case RI_GPIO_MODE_INPUT_NOPULL:
                nrf_gpio_cfg_input (nrf_pin, NRF_GPIO_PIN_NOPULL);
                break;

            case RI_GPIO_MODE_INPUT_PULLUP:
                nrf_gpio_cfg_input (nrf_pin, NRF_GPIO_PIN_PULLUP);
                break;

            case RI_GPIO_MODE_INPUT_PULLDOWN:
                nrf_gpio_cfg_input (nrf_pin, NRF_GPIO_PIN_PULLDOWN);
                break;

            case RI_GPIO_MODE_OUTPUT_STANDARD:
                nrf_gpio_cfg_output (nrf_pin);
                break;

            case RI_GPIO_MODE_OUTPUT_HIGHDRIVE:
                nrf_gpio_cfg (nrf_pin,
                              NRF_GPIO_PIN_DIR_OUTPUT,
                              NRF_GPIO_PIN_INPUT_DISCONNECT,
                              NRF_GPIO_PIN_NOPULL,
                              NRF_GPIO_PIN_H0H1,
                              NRF_GPIO_PIN_NOSENSE);
                break;

            case RI_GPIO_MODE_SINK_PULLUP_STANDARD:
                nrf_gpio_cfg (nrf_pin,
                              NRF_GPIO_PIN_DIR_OUTPUT,
                              NRF_GPIO_PIN_INPUT_CONNECT,
                              NRF_GPIO_PIN_PULLUP,
                              NRF_GPIO_PIN_S0D1,
                              NRF_GPIO_PIN_NOSENSE);
                break;

            case RI_GPIO_MODE_SINK_NOPULL_STANDARD:
                nrf_gpio_cfg (nrf_pin,
                              NRF_GPIO_PIN_DIR_OUTPUT,
                              NRF_GPIO_PIN_INPUT_CONNECT,
                              NRF_GPIO_PIN_NOPULL,
                              NRF_GPIO_PIN_S0D1,
                              NRF_GPIO_PIN_NOSENSE);
                break;

            case RI_GPIO_MODE_SINK_PULLUP_HIGHDRIVE:
                nrf_gpio_cfg (nrf_pin,
                              NRF_GPIO_PIN_DIR_OUTPUT,
                              NRF_GPIO_PIN_INPUT_CONNECT,
                              NRF_GPIO_PIN_PULLUP,
                              NRF_GPIO_PIN_H0D1,
                              NRF_GPIO_PIN_NOSENSE);
                break;

            case RI_GPIO_MODE_SINK_NOPULL_HIGHDRIVE:
                nrf_gpio_cfg (nrf_pin,
                              NRF_GPIO_PIN_DIR_OUTPUT,
                              NRF_GPIO_PIN_INPUT_CONNECT,
                              NRF_GPIO_PIN_NOPULL,
                              NRF_GPIO_PIN_S0D1,
                              NRF_GPIO_PIN_NOSENSE);
                break;

            default:
                err_code |= RD_ERROR_INVALID_PARAM;
        }
    }

    return err_code;
}

rd_status_t ri_gpio_toggle (const ri_gpio_id_t pin)
{
    if (RI_GPIO_ID_UNUSED != pin)
    {
        const uint8_t nrf_pin = ruuvi_to_nrf_pin_map (pin);
        nrf_gpio_pin_toggle (nrf_pin);
    }

    return RD_SUCCESS;
}

rd_status_t ri_gpio_write (const ri_gpio_id_t pin,
                           const ri_gpio_state_t state)
{
    rd_status_t err_code = RD_SUCCESS;

    if (RI_GPIO_ID_UNUSED != pin)
    {
        const uint8_t nrf_pin = ruuvi_to_nrf_pin_map (pin);

        if (RI_GPIO_HIGH == state)
        {
            nrf_gpio_pin_set (nrf_pin);
        }
        else if (RI_GPIO_LOW  == state)
        {
            nrf_gpio_pin_clear (nrf_pin);
        }
        else
        {
            err_code |= RD_ERROR_INVALID_PARAM;
        }
    }

    return err_code;
}

rd_status_t ri_gpio_read (const ri_gpio_id_t pin,
                          ri_gpio_state_t * const state)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == state)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (RI_GPIO_ID_UNUSED != pin)
    {
        const uint8_t nrf_pin = ruuvi_to_nrf_pin_map (pin);
        bool high = nrf_gpio_pin_read (nrf_pin);

        if (true == high)
        {
            *state = RI_GPIO_HIGH;
        }

        if (false == high)
        {
            *state = RI_GPIO_LOW;
        }
    }

    return err_code;
}
/** @} */
#endif
