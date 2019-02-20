#ifndef RUUVI_INTERFACE_GPIO_INTERRUPT_H
#define RUUVI_INTERFACE_GPIO_INTERRUPT_H
/**
 * @addtogroup GPIO
 * @{
 */
/**
 * @file ruuvi_interface_gpio_interrupt.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-02-01
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for basic GPIO interrupt functions
 */
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
/**
 * Enumeration for GPIO slopes
 */
typedef enum 
{
  RUUVI_INTERFACE_GPIO_SLOPE_HITOLO, /**<  High to low transition */
  RUUVI_INTERFACE_GPIO_SLOPE_LOTOHI, /**<  Low to high transition */
  RUUVI_INTERFACE_GPIO_SLOPE_TOGGLE, /**<  Any transition */
  RUUVI_INTERFACE_GPIO_SLOPE_UNKNOWN /**<  Error or unknown value. */
}ruuvi_interface_gpio_slope_t;

/**
 * Event from GPIO
 */
typedef struct 
{
  ruuvi_interface_gpio_slope_t slope; /**< @ref ruuvi_interface_gpio_slope_t slope of event */
  uint8_t pin;                        /**< Pin of the event */
}ruuvi_interface_gpio_evt_t;

typedef void(*ruuvi_interface_gpio_interrupt_fp_t)(const ruuvi_interface_gpio_evt_t);

/**
 * @brief Initialize interrupt functionality to GPIO. 
 * Takes address of interrupt table as a pointer to avoid tying driver into a specific board with a specific number of GPIO
 * pins and to avoid including boards repository within the driver.
 *
 * @param interrupt_table Array of function pointers, initialized to all nulls. Size should be the number of GPIO+1, i.e. RUUVI_BOARD_GPIO_NUMBER + 1.
 * @param max_interrupts Size of interrupt table. 
 *
 * @return @ref RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_init(ruuvi_interface_gpio_interrupt_fp_t* const interrupt_table, const uint8_t max_interrupts);

/**
 * @brief Enable interrupt on a pin.
 *
 * Underlying implementation is allowed to use same interrupt channel for all pin interrupts, i.e. 
 * simultaneous interrupts might get detected as one and the priority of interrupts is undefined. 
 *
 * @param pin pin to use as interrupt source
 * @param slope slope to interrupt on
 * @param mode GPIO input mode. Must be (RUUVI_INTERFACE_GPIO_)INPUT_PULLUP, INPUT_PULLDOWN or INPUT_NOPULL
 * @param handler function pointer which will be called with ruuvi_interface_gpio_evt_t as a parameter on interrupt.
 *
 * @return @ref RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @warning Simultaneous interrupts may be lost. Check the underlying implementation.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_enable(const uint8_t pin, 
                                                            const ruuvi_interface_gpio_slope_t slope, 
                                                            const ruuvi_interface_gpio_mode_t mode, 
                                                            const ruuvi_interface_gpio_interrupt_fp_t handler);

#endif