#ifndef RUUVI_INTERFACE_GPIO_H
#define RUUVI_INTERFACE_GPIO_H
#include "ruuvi_driver_error.h"
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

#define RUUVI_INTERFACE_GPIO_PORT_UNUSED 0xFF   //!< Use this value to signal that nothing should be done with this port, i.e. UART CTS not used.
#define RUUVI_INTERFACE_GPIO_PIN_UNUSED  0xFF   //!< Use this value to signal that nothing should be done with this pin,  i.e. UART CTS not used.
#define RUUVI_INTERFACE_GPIO_ID_UNUSED   0xFFFF //!< Use this value to signal that nothing should be done with this gpio,  i.e. UART CTS not used.

/**
 * GPIO modes supported by interface. If the underlying platform
 * does not support given mode, it shall return @ref RUUVI_DRIVER_ERROR_NOT_SUPPORTED
 * on configuration attempt.
 */
typedef enum
{
  RUUVI_INTERFACE_GPIO_MODE_HIGH_Z,          //!< High-impedance mode, electrically disconnected.
  RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL,    //!< Input, can be read. No pull resistors
  RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP,    //!< Input, can be read. Pulled up by internal resistor, value depends on IC.
  RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN,  //!< Input, can be read. Pulled dpwn by internal resistor, value depends on IC.
  RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD, //!< Push-pull output, can be written.
  RUUVI_INTERFACE_GPIO_MODE_OUTPUT_HIGHDRIVE //!< Push-pull output, can be written. Higher current drive than standard.
} ruuvi_interface_gpio_mode_t;

/**
 * @brief States of GPIO pins
 */
typedef enum
{
  RUUVI_INTERFACE_GPIO_LOW = false, //!< GPIO electrically low
  RUUVI_INTERFACE_GPIO_HIGH = true  //!< GPIO electrically high
} ruuvi_interface_gpio_state_t;

/**
 * @brief GPIO pin map
 */
typedef struct
{
  uint8_t  pin;  //!< Pin number
  uint8_t  port; //!< Port of the GPIO pin
}ruuvi_interface_gpio_port_pin_t;

/** @brief Shorthand for using plain GPIOs in code. */
typedef union {
  uint16_t pin;                             //<! Single number representing port + pin
  ruuvi_interface_gpio_port_pin_t port_pin; //!< Explicit port + pin
}ruuvi_interface_gpio_id_t;

/**
 * @brief Initializes GPIO module. Call this before other GPIO functions.
 * After initialization all GPIO pins shall be in High-Z mode.
 * 
 *
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if GPIO is already initialized
 */
ruuvi_driver_status_t ruuvi_interface_gpio_init(void);

/**
 * @brief Uninitializes GPIO module. Call this to reset GPIO to High-Z mode. 
 * After uninitialization all GPIO pins shall be in High-Z mode. 
 * Uninitialization can be called at any time, but behaviour is not defined
 * if some other peripheral (i.e. SPI) is using GPIO pins. 
 *
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return error code from stack on error
 */
ruuvi_driver_status_t ruuvi_interface_gpio_uninit(void);

/**
 * @brief return true if GPIO is init, false otherwise.
 *
 * @return @c true if GPIO module is init
 * @return @c false if GPIO module is not init 
 */
bool  ruuvi_interface_gpio_is_init(void);

/**
 * @brief Configure a pin of a port into a mode.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param pin[in] Pin number.
 * @param mode[in] Mode to set the pin to. See @ref ruuvi_interface_gpio_mode_t for possible values.
 *
 * @return @ref RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @return @ref RUUVI_DRIVER_ERROR_NOT_SUPPORTED if underlying platform does not support given mode.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_configure(const ruuvi_interface_gpio_id_t pin,
    const ruuvi_interface_gpio_mode_t mode);

/**
 * @brief Toggle the state of a pin of a port.
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param pin[in] Pin number.
 *
 * @return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an output (optional).
 */
ruuvi_driver_status_t ruuvi_interface_gpio_toggle(const ruuvi_interface_gpio_id_t pin);

/**
 * @brief Write a pin of a port into given state
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param pin[in]   Pin number.
 * @param state[in] State to which the pin should be set to. See @ref ruuvi_interface_gpio_state_t for possible values
 *
 * @return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an output (optional).
 */
ruuvi_driver_status_t ruuvi_interface_gpio_write(const ruuvi_interface_gpio_id_t pin,
    const ruuvi_interface_gpio_state_t state);

/**
 * @brief Read state of a pin of a port into bool high
 * If there are several ports the platform driver must implement a conversion function from port + pin to uint8_t.
 *
 * @param pin[in]   Pin number.
 * @param p_state[out] Pointer to a ruuvi_interface_gpio_state_t which will be set to the state of the pin.
 *
 * @return RUUVI_DRIVER_SUCCESS on success, error code on failure.
 * @return RUUVI_DRIVER_ERROR_NULL if *state is a null pointer.
 * @return RUUVI_DRIVER_ERROR_INVALID_ADDRESS if pointer is invalid for any reason (optional).
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if pin was not set as an input (optional).
 */
ruuvi_driver_status_t ruuvi_interface_gpio_read(const ruuvi_interface_gpio_id_t pin,
    ruuvi_interface_gpio_state_t* const p_state);
/*@}*/
#endif