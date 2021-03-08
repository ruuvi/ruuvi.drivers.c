/**
 * GPIO interrupt implementations on Nordic SDK15.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_gpio_interrupt.h"
#if RUUVI_NRF5_SDK15_GPIO_INTERRUPT_ENABLED
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_nrf5_sdk15_gpio.h"
#include "ruuvi_interface_gpio.h"


#include <stdbool.h>
#include "nrf.h"
#include "nrf_drv_gpiote.h"

//Pointer to look-up table for event handlers
static ri_gpio_interrupt_fp_t * pin_event_handlers;
static uint8_t max_interrupts = 0;

rd_status_t ri_gpio_interrupt_init (
    ri_gpio_interrupt_fp_t * const interrupt_table,
    const uint16_t interrupt_table_size)
{
    if (NULL == interrupt_table) { return RD_ERROR_NULL; }

    if (!ri_gpio_is_init()) { return RD_ERROR_INVALID_STATE; }

    // Check module initialization status by max interrupts
    if (0 != max_interrupts) { return RD_ERROR_INVALID_STATE; }

    /* Driver initialization
       The GPIOTE driver is a shared resource that can be used by multiple modules in an application.
       Therefore, it can be initialized only once. If a module is using the driver,
       it must check if it has already been initialized by calling the function nrf_drv_gpiote_is_init.
       If this function returns false, the module must initialize the driver by calling the function nrf_drv_gpiote_init.

       The following code example shows how to initialize the driver:
    */
    ret_code_t err_code = NRF_SUCCESS;

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
    }

    pin_event_handlers = interrupt_table;
    max_interrupts = interrupt_table_size;
    return ruuvi_nrf5_sdk15_to_ruuvi_error (err_code);
}

rd_status_t ri_gpio_interrupt_uninit (void)
{
    if (0 == max_interrupts) { return RD_SUCCESS; }

    pin_event_handlers = NULL;
    max_interrupts = 0;
    return RD_SUCCESS;
}

bool ri_gpio_interrupt_is_init()
{
    return (0 != max_interrupts);
}

static void in_pin_handler (const nrf_drv_gpiote_pin_t pin,
                            const nrf_gpiote_polarity_t action)
{
    if (max_interrupts <= pin) { return; }

    ri_gpio_evt_t event;
    ri_gpio_state_t state;

    if (NULL != pin_event_handlers[pin])
    {
        switch (action)
        {
            case NRF_GPIOTE_POLARITY_LOTOHI:
                event.slope = RI_GPIO_SLOPE_LOTOHI;
                break;

            case NRF_GPIOTE_POLARITY_HITOLO:
                event.slope = RI_GPIO_SLOPE_HITOLO;
                break;

            default:
                // Determine slope from current state
                ri_gpio_read (nrf_to_ruuvi_pin (pin), &state);
                event.slope = (state == RI_GPIO_LOW) ? RI_GPIO_SLOPE_HITOLO : RI_GPIO_SLOPE_LOTOHI;
                break;
        }

        //Call event handler.
        event.pin = nrf_to_ruuvi_pin (pin);
        (pin_event_handlers[pin]) (event);
    }
}

rd_status_t ri_gpio_interrupt_enable (const
                                      ri_gpio_id_t pin,
                                      const ri_gpio_slope_t slope,
                                      const ri_gpio_mode_t mode,
                                      const ri_gpio_interrupt_fp_t handler)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!ri_gpio_interrupt_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (RI_GPIO_ID_UNUSED == pin)
    {
        // No action needed
    }
    else
    {
        // nRF5 devices have 32 pins per port. Pack the port-pin representation into 8 bits for interrupt table.
        uint8_t nrf_pin = ruuvi_to_nrf_pin_map (pin);

        if (nrf_pin >= max_interrupts) { return RD_ERROR_INVALID_PARAM; }

        ret_code_t nrf_code = NRF_SUCCESS;
        nrf_gpiote_polarity_t polarity = NRF_GPIOTE_POLARITY_TOGGLE;
        nrf_gpio_pin_pull_t pull = NRF_GPIO_PIN_NOPULL;

        switch (slope)
        {
            case RI_GPIO_SLOPE_TOGGLE:
                polarity = NRF_GPIOTE_POLARITY_TOGGLE;
                break;

            case RI_GPIO_SLOPE_LOTOHI:
                polarity = NRF_GPIOTE_POLARITY_LOTOHI;
                break;

            case RI_GPIO_SLOPE_HITOLO:
                polarity = NRF_GPIOTE_POLARITY_HITOLO;
                break;

            default:
                err_code |= RD_ERROR_INVALID_PARAM;
        }

        switch (mode)
        {
            case RI_GPIO_MODE_INPUT_NOPULL:
                pull = NRF_GPIO_PIN_NOPULL;
                break;

            case RI_GPIO_MODE_INPUT_PULLUP:
                pull = NRF_GPIO_PIN_PULLUP;
                break;

            case RI_GPIO_MODE_INPUT_PULLDOWN:
                pull = NRF_GPIO_PIN_PULLDOWN;
                break;

            default:
                err_code |= RD_ERROR_INVALID_PARAM;
        }

        //  high-accuracy mode consumes excess power
        //  is_watcher is used if we track an output pin.
        nrf_drv_gpiote_in_config_t in_config =
        {
            .is_watcher = false,
            .hi_accuracy = false,
            .pull = pull,
            .sense = polarity
        };
        pin_event_handlers[nrf_pin] = handler;
        nrf_code |= nrf_drv_gpiote_in_init (nrf_pin, &in_config, in_pin_handler);
        nrf_drv_gpiote_in_event_enable (nrf_pin, true);
        err_code |= ruuvi_nrf5_sdk15_to_ruuvi_error (nrf_code);
    }

    return err_code;
}

rd_status_t ri_gpio_interrupt_disable (const ri_gpio_id_t pin)
{
    rd_status_t err_code = RD_SUCCESS;

    if (RI_GPIO_ID_UNUSED != pin)
    {
        uint8_t nrf_pin = ruuvi_to_nrf_pin_map (pin);

        if (NULL != pin_event_handlers && NULL != pin_event_handlers[nrf_pin])
        {
            nrf_drv_gpiote_in_event_disable (nrf_pin);
            nrf_drv_gpiote_in_uninit (nrf_pin);
            pin_event_handlers[nrf_pin] = NULL;
        }
    }

    return err_code;
}

#endif