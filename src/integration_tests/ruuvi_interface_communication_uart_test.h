#ifndef RUUVI_INTERFACE_COMMUNICATION_UART_TEST_H
#define RUUVI_INTERFACE_COMMUNICATION_UART_TEST_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication_uart.h"
#include "ruuvi_interface_communication.h"
#include <stdbool.h>

#define TEST_TIMEOUT_MS (100U) //!< If TX/RX test not ready in this time, fail

/**
 * @addtogroup UART
 * @{
 */
/**
 * @file ruuvi_interface_communication_uart_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-06-03
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Test implementation of UART interface.
 */
/*
 * @brief Run UART integration tests.
 *
 * Writing and reading requires external GPIO IO connection to test sending and receiving data
 * via UART. HWFC is not tested.
 *
 * @param[in] printfp Function pointer to which test JSON is sent.
 * @param[in] input  Pin used as RXD.
 * @param[in] output Pin used as TXD.
 * @retval true if error occured in test.
 * @retval false if no errors occured.
 */
bool ri_communication_uart_run_integration_test (const rd_test_print_fp printfp,
        const ri_gpio_id_t input, const ri_gpio_id_t output);
/** @} */
#endif
