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
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
/** @brief Enable implementation selected by application */
#if RI_GPIO_ENABLED
#  define RUUVI_NRF5_SDK15_GPIO_INTERRUPT_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/**
 * Enumeration for GPIO slopes
 */
typedef enum
{
    RI_GPIO_SLOPE_HITOLO, /**<  High to low transition */
    RI_GPIO_SLOPE_LOTOHI, /**<  Low to high transition */
    RI_GPIO_SLOPE_TOGGLE, /**<  Any transition */
    RI_GPIO_SLOPE_UNKNOWN /**<  Error or unknown value. */
} ri_gpio_slope_t;

/**
 * @brief Event from GPIO
 */
typedef struct
{
    /** @brief @ref ri_gpio_slope_t slope of event */
    ri_gpio_slope_t slope;
    /**@brief Pin of the event */
    ri_gpio_id_t pin;
} ri_gpio_evt_t;

typedef void (*ri_gpio_interrupt_fp_t) (const ri_gpio_evt_t);

/**
 * @brief Initialize interrupt functionality to GPIO.
 * Takes address of interrupt table as a pointer to avoid tying driver into a specific board with a specific number of GPIO
 * pins and to avoid including boards repository within the driver. The interrupt table must be retained in the RAM.
 *
 * - Initialization must return @c RD_ERROR_INVALID_STATE if GPIO is uninitialized
 * - Initialization must return @c RD_SUCCESS on first call.
 * - Initialization must return @c RD_ERROR_INVALID_STATE on second call.
 * - Initialization must return @c RD_SUCCESS after uninitializtion.
 * - Initialization must return @c RD_ERROR_NULL if interrupt handler table is @c NULL.
 *
 * @param[in] interrupt_table Array of function pointers, initialized to all nulls. Size should be the number of GPIO+1, i.e. RUUVI_BOARD_GPIO_NUMBER + 1.
 * @param[in] max_interrupts Size of interrupt table.
 *
 * @return @ref RD_SUCCESS on success, error code on failure.
 */
rd_status_t ri_gpio_interrupt_init (
    ri_gpio_interrupt_fp_t * const interrupt_table, const uint16_t max_interrupts);

/**
 * @brief Uninitialize interrupt functionality of GPIO.
 *
 * @return @ref RD_SUCCESS on success, error code on failure.
 */
rd_status_t ri_gpio_interrupt_uninit (void);

/**
 * @brief Check if interrupt module is initialized
 *
 * @return @c true if module is initialized, false otherwise
 */
bool ri_gpio_interrupt_is_init (void);

/**
 * @brief Enable interrupt on a pin.
 *
 * Underlying implementation is allowed to use same interrupt channel for all pin interrupts, i.e.
 * simultaneous interrupts might get detected as one and the priority of interrupts is undefined.
 *
 * - Return RD_ERROR_INVALID_STATE if GPIO or GPIO_INTERRUPT are not initialized
 * - Interrupt function shall be called exactly once when input is configured as low-to-high while input is low and
 *   input goes low-to-high, high-to-low.
 * - Interrupt function shall not be called after interrupt has been disabled
 * - Interrupt function shall be called exactly once when input is configured as high-to-low while input is low and
 *   input goes low-to-high, high-to-low.
 * - Interrupt function shall be called exactly twice when input is configured as toggle while input is low and
 *   input goes low-to-high, high-to-low.
 * - Interrupt pin shall be at logic HIGH when interrupt is enabled with a pull-up and the pin is not loaded externally
 * - Interrupt pin shall be at logic LOW when interrupt is enabled with a pull-down and the pin is not loaded externally
 *
 * @param[in] pin pin to use as interrupt source
 * @param[in] slope slope to interrupt on
 * @param[in] mode GPIO input mode. Must be (RI_GPIO_)INPUT_PULLUP, INPUT_PULLDOWN or INPUT_NOPULL
 * @param[in] handler function pointer which will be called with ri_gpio_evt_t as a parameter on interrupt.
 *
 * @return @ref RD_SUCCESS on success, error code on failure.
 * @warning Simultaneous interrupts may be lost. Check the underlying implementation.
 */
rd_status_t ri_gpio_interrupt_enable (const ri_gpio_id_t pin,
                                      const ri_gpio_slope_t slope,
                                      const ri_gpio_mode_t mode,
                                      const ri_gpio_interrupt_fp_t handler);

/**
 * @brief Disable interrupt on a pin.
 *
 * Pin will be left as @ref RI_GPIO_MODE_HIGH_Z.
 *
 * @param[in] pin pin to disable as interrupt source.
 *
 * @retval RD_SUCCESS on success.

 */
rd_status_t ri_gpio_interrupt_disable (const ri_gpio_id_t pin);

/** @} */

#endif
