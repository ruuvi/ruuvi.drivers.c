#ifndef RUUVI_INTERFACE_GPIO_H
#define RUUVI_INTERFACE_GPIO_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_enabled_modules.h"
#include <stdbool.h>
/**
 * @defgroup GPIO GPIO functions
 * @brief Functions for digitally reading and actuating GPIO pins.
 *
 * The GPIO functions do include interrupts, but they do not include PWM,
 * ADC or DAC functions.
 */
/*@{*/
/**
 * @file ruuvi_interface_gpio.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-04-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for basic GPIO writes and reads
 *
 */

/** @brief Enable implementation selected by application */
#if RI_GPIO_ENABLED
#  define RUUVI_NRF5_SDK15_GPIO_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

#define RI_GPIO_ID_UNUSED   0xFFFF //!< Use this value to signal that nothing should be done with this gpio,  i.e. UART CTS not used.

/**
 * GPIO modes supported by interface. If the underlying platform
 * does not support given mode, it shall return @ref RD_ERROR_NOT_SUPPORTED
 * on configuration attempt.
 */
typedef enum
{
    RI_GPIO_MODE_HIGH_Z,           //!< High-impedance mode, electrically disconnected.
    RI_GPIO_MODE_INPUT_NOPULL,     //!< Input, can be read. No pull resistors
    RI_GPIO_MODE_INPUT_PULLUP,     //!< Input, can be read. Pulled up by internal resistor, value depends on IC.
    RI_GPIO_MODE_INPUT_PULLDOWN,   //!< Input, can be read. Pulled dpwn by internal resistor, value depends on IC.
    RI_GPIO_MODE_OUTPUT_STANDARD,  //!< Push-pull output, can be written.
    RI_GPIO_MODE_OUTPUT_HIGHDRIVE, //!< Push-pull output, can be written. Higher current drive than standard.
    RI_GPIO_MODE_SINK_PULLUP_STANDARD,  //!< Sink only, pull-up, standard drive. I2C without external resistors.
    RI_GPIO_MODE_SINK_NOPULL_STANDARD,  //!< Sink only, nopull, standard drive. I2C with weak external resistors.
    RI_GPIO_MODE_SINK_PULLUP_HIGHDRIVE, //!< Sink only, pull-up, high-drive. I2C with optional external resistors.
    RI_GPIO_MODE_SINK_NOPULL_HIGHDRIVE  //!< Sink only, nopull,, high-drive. I2C with strong external resistors.
} ri_gpio_mode_t;

/**
 * @brief States of GPIO pins
 */
typedef enum
{
    RI_GPIO_LOW = false, //!< GPIO electrically low
    RI_GPIO_HIGH = true  //!< GPIO electrically high
} ri_gpio_state_t;

/** @brief port<<8 + pin */
typedef uint16_t ri_gpio_id_t;

/**
 * @brief Initializes GPIO module. Call this before other GPIO functions.
 * After initialization all GPIO pins shall be in High-Z mode.
 *
 *
 * @return RD_SUCCESS on success
 * @return RD_ERROR_INVALID_STATE if GPIO is already initialized
 */
rd_status_t ri_gpio_init (void);

/**
 * @brief Uninitializes GPIO module. Call this to reset GPIO to High-Z mode.
 * After uninitialization all GPIO pins shall be in High-Z mode.
 * Uninitialization can be called at any time, but behaviour is not defined
 * if some other peripheral (i.e. SPI) is using GPIO pins.
 *
 * @return RD_SUCCESS on success
 * @return error code from stack on error
 */
rd_status_t ri_gpio_uninit (void);

/**
 * @brief return true if GPIO is init, false otherwise.
 *
 * @return @c true if GPIO module is init
 * @return @c false if GPIO module is not init
 */
bool ri_gpio_is_init (void);

/**
 * @brief Configure a pin of a port into a mode.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param[in] pin Pin number.
 * @param[in] mode Mode to set the pin to. See @ref ri_gpio_mode_t for possible values.
 *
 * @return @ref RD_SUCCESS on success, error code on failure.
 * @return @ref RD_ERROR_NOT_SUPPORTED if underlying platform does not support given mode.
 */
rd_status_t ri_gpio_configure (const ri_gpio_id_t pin,
                               const ri_gpio_mode_t mode);

/**
 * @brief Toggle the state of a pin of a port.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param[in] pin Pin number.
 *
 * @return RD_SUCCESS on success, error code on failure.
 * @return RD_ERROR_INVALID_STATE if pin was not set as an output (optional).
 */
rd_status_t ri_gpio_toggle (const ri_gpio_id_t pin);

/**
 * @brief Write a pin of a port into given state
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param[in] pin   Pin number.
 * @param[in] state State to which the pin should be set to. See @ref ri_gpio_state_t for possible values
 *
 * @return RD_SUCCESS on success, error code on failure.
 * @return RD_ERROR_INVALID_STATE if pin was not set as an output (optional).
 */
rd_status_t ri_gpio_write (const ri_gpio_id_t pin,
                           const ri_gpio_state_t state);

/**
 * @brief Read state of a pin of a port into bool high
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param[in] pin   Pin number.
 * @param[out] p_state Pointer to a ri_gpio_state_t which will be set to the state of the pin.
 *
 * @return RD_SUCCESS on success, error code on failure.
 * @return RD_ERROR_NULL if *state is a null pointer.
 * @return RD_ERROR_INVALID_ADDRESS if pointer is invalid for any reason (optional).
 * @return RD_ERROR_INVALID_STATE if pin was not set as an input (optional).
 */
rd_status_t ri_gpio_read (const ri_gpio_id_t pin,
                          ri_gpio_state_t * const p_state);
/*@}*/
#endif
