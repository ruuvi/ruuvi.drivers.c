/**
 * GPIO interrupt definitions
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_PIN_INTERRUPT_H
#define RUUVI_INTERFACE_PIN_INTERRUPT_H

#include "nrf_drv_gpiote.h"

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"

typedef enum 
{
  RUUVI_INTERFACE_GPIO_SLOPE_HITOLO,
  RUUVI_INTERFACE_GPIO_SLOPE_LOTOHI,
  RUUVI_INTERFACE_GPIO_SLOPE_TOGGLE,
  RUUVI_INTERFACE_GPIO_SLOPE_UNKNOWN
}ruuvi_interface_gpio_slope_t;

typedef struct 
{
  ruuvi_interface_gpio_slope_t slope;
  uint8_t pin;
}ruuvi_interface_gpio_evt_t;

typedef void(*ruuvi_interface_gpio_interrupt_fp_t)(const ruuvi_interface_gpio_evt_t);

/**
 * Initialize interrupt functionality to GPIO
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_platform_pin_interrupt_init();

/**
 * Enable interrupt on a pin.
 *
 * parameter pin: pin to use as interrupt source
 * parameter slope: slope to interrupt on
 * parameter mode: GPIO input mode. Must be (RUUVI_INTERFACE_GPIO_)INPUT_PULLUP, INPUT_PULLDOWN or INPUT_NOPULL
 * parameter handler: function pointer which will be called with ruuvi_interface_gpio_evt_t as a parameter on interrupt.
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_platform_gpio_interrupt_enable(uint8_t pin, ruuvi_interface_gpio_slope_t slope, ruuvi_interface_gpio_mode_t mode, ruuvi_interface_gpio_interrupt_fp_t handler);

#endif