#ifndef RUUVI_INTERFACE_PIN_INTERRUPT_H
#define RUUVI_INTERFACE_PIN_INTERRUPT_H
/**
 * @defgroup Gpio Gpio functions
 * @brief Functions for digitally reading and actuating GPIO pins.
 *
 * The GPIO functions do include interrupts, but they do not include PWM,
 * ADC or DAC functions.
 */
/*@{*/
/**
 * @file ruuvi_interface_gpio.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-01-30
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for basic GPIO writes and reads 
 *
 */

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
 * Initialize interrupt functionality to GPIO. Takes address of interrupt table as a pointer to avoid tying driver into a specific board with a specific number of GPIO
 * pins and to avoid including boards repository within the driver.
 *
 * parameter interrupt_table: Array of function pointers, initialized to all nulls. Size should be the number of GPIO+1, i.e. RUUVI_BOARD_GPIO_NUMBER + 1.
 * parameter max_interrupts: Size of interrupt table. 
 *
 * Return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_init(ruuvi_interface_gpio_interrupt_fp_t* interrupt_table, uint8_t max_interrupts);

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
ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_enable(uint8_t pin, 
                                                           ruuvi_interface_gpio_slope_t slope, 
                                                           ruuvi_interface_gpio_mode_t mode, 
                                                           ruuvi_interface_gpio_interrupt_fp_t handler);

#endif