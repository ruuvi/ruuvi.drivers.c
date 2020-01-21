#ifndef RUUVI_INTERFACE_UART_H
#define RUUVI_INTERFACE_UART_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup UART UART functions
 * @brief Functions for using UART bus
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_uart.h
* @brief Interface for UART operations
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-10-01
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */

/**
 * Baudrate for UART
 */
typedef enum
{
    RUUVI_INTERFACE_UART_BAUD_9600,   //!< 9600 bauds
    RUUVI_INTERFACE_UART_BAUD_115200 //!< 2 Mbps
} ruuvi_interface_uart_baud_t;

/**
 * Configuration for initializing UART
 */
typedef struct
{
    ruuvi_interface_gpio_id_t tx;     //!< pin number of TX.
    ruuvi_interface_gpio_id_t rx;     //!< pin number of RX.
    ruuvi_interface_gpio_id_t cts;    //!< pin number of CTS.
    ruuvi_interface_gpio_id_t rts;    //!< pin number of RTS.
    ruuvi_interface_uart_baud_t baud; //!< Baud rate, see @ref ruuvi_interface_uart_baud_t.
    bool parity;                      //!< True -> parity bit enabled. False -> disabled.
    bool hwfc;                        //!< True -> Hardware flow control enabled. False -> disabled.
} ruuvi_interface_uart_init_config_t;

/**
 * @brief Callback function for received data.
 */
//typedef void (*ruuvi_interface_uart_cb_t)(const uint8_t* const data, const size_t data_length);

/**
 * @brief Initialize UART driver with given settings
 *
 * This function also handles configuring the GPIO pins as required
 *
 * @param config Configuration of the UART peripheral. Will setup given pins as required.
 * @return RUUVI_DRIVER_SUCCESS if no error occurred
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if UART was already initialized
 **/
ruuvi_driver_status_t ruuvi_interface_uart_init (const ruuvi_interface_uart_init_config_t *
        const config);

/**
 * @brief check if UART interface is already initialized.
 *
 * @return @c true if UART is initialized
 * @return @c false otherwise
 */
bool ruuvi_interface_uart_is_init();

/**
 * @brief Uninitialize UART driver.
 *
 * This function might not uninitialize the UART GPIO pins, only the underlying peripheral.
 * Uninitialized GPIOs explicitly if that is required.
 *
 * @return RUUVI_DRIVER_SUCCESS
 * @warning Uninitializes the UART peripheral, may or may not uninitialize the associated gpio pins
 **/
ruuvi_driver_status_t ruuvi_interface_uart_uninit();

/**
 * @brief UART write function.
 * Function is blocking and will not sleep while transaction is ongoing.
 *
 * @param p_tx pointer to data to be sent. Must be in RAM.
 * @param tx_len length of data to be sent.
 * @return RUUVI_DRIVER_SUCCSS when data was sent.
 * @return RUUVI_DRIVER_ERROR_NULL if p_tx is NULL.
 * @return RUUVI_DRIVER_ERROR_INVALID_ADDRESS if p_tx is not in RAM
 * @return error code from stack on other error.
 **/
ruuvi_driver_status_t ruuvi_interface_uart_send_blocking (const uint8_t * const p_tx,
        const size_t tx_len);

/**
 * @brief Configure a callback to be called once data is received.
 */
//void ruuvi_interface_uart_rx_cb_set(const ruuvi_interface_uart_cb_t cb);
/* @} */
#endif