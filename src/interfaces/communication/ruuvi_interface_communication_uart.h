#ifndef RUUVI_INTERFACE_COMMUNICATION_UART_H
#define RUUVI_INTERFACE_COMMUNICATION_UART_H
/**
 * @file ruuvi_interface_communication_uart.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-05-06
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Sending and receiving data over UART
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_gpio.h"

#include <stdbool.h>

#if RI_UART_ENABLED
#   define RUUVI_NRF5_SDK15_UART_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/** @brief Supported baudrates. */
typedef enum
{
    RI_UART_BAUD_9600,    //!< Run at 9600 baud.
    RI_UART_BAUD_115200,  //!< Run at 115200 baud.
    RI_UART_BAUDRATE_NUM  //!< Number of options.
} ri_uart_baudrate_t;

/** @brief UART initialization data. */
typedef struct
{
    bool hwfc_enabled;   //!< True to enable hardware flow control.
    bool parity_enabled; //!< True to enable parity.
    ri_gpio_id_t cts;    //!< CTS pin. Can be RI_GPIO_UNUSED if not using HWFC.
    ri_gpio_id_t rts;    //!< RTS pin. Can be RI_GPIO_UNUSED if not using HWFC.
    ri_gpio_id_t tx;     //!< TX pin.
    ri_gpio_id_t rx;     //!< RX pin.
    ri_uart_baudrate_t baud; //!< @ref ri_uart_baudrate_t.
} ri_uart_init_t;

/**
 * @brief Initialize UART.
 *
 * This only setups the function interface, you must call @ref ri_uart_config
 * to setup pins, HWFC, baudrate tec.
 *
 * @param[out] channel Interface used for communicating through uart.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if UART is already initialized.
 * @retval RD_ERROR_NULL Channel is NULL.
 */
rd_status_t ri_uart_init (ri_comm_channel_t * const channel);

/**
 * @brief Configure UART.
 *
 * @param[out] config Interface used for communicating through uart.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL Channel is NULL.
 */
rd_status_t ri_uart_config (const ri_uart_init_t * const config);

/*
 * @brief Uninitializes UART.
 *
 * @param[out] channel comm api to send and receive data via uart.
 *
 * @retval RD_SUCCESS on success or if radio was not initialized.
 * @retval RD_ERROR_INVALID_STATE if radio hardware was initialized by another radio module.
 */
rd_status_t ri_uart_uninit (ri_comm_channel_t * const channel);

#endif
