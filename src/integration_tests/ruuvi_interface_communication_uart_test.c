#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_communication_uart_test.h"
#include "ruuvi_interface_communication_uart.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_yield.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/**
 * @addtogroup UART
 * @{
 */
/**
 * @file ruuvi_interface_communication_uart_test.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-06-03
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Integration test UART implementation.
 */

static ri_comm_channel_t m_channel;
static uint8_t m_has_sent;
static uint8_t m_has_received;
static ri_comm_message_t rx_data;

static rd_status_t uart_isr (ri_comm_evt_t evt,
                             void * p_data, size_t data_len)
{
    switch (evt)
    {
        case RI_COMM_CONNECTED:
            // Will never trigger on UART
            break;

        case RI_COMM_DISCONNECTED:
            // Will never trigger on UART
            break;

        case RI_COMM_SENT:
            m_has_sent++;
            break;

        case RI_COMM_RECEIVED:
            m_has_received++;
            rx_data.data_length = RI_COMM_MESSAGE_MAX_LENGTH;
            m_channel.read (&rx_data);
            break;

        default:
            break;
    }

    return RD_SUCCESS;
}

/*
 * Initializes UART hardware.
 *
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_INVALID_STATE if UART is already initialized
 */
static bool ri_uart_init_test (const rd_test_print_fp printfp)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"init\":");
    err_code |= ri_uart_init (&m_channel);

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_uart_init (&m_channel);

        if (RD_ERROR_INVALID_STATE == err_code)
        {
            err_code = ri_uart_uninit (&m_channel);
            err_code = ri_uart_init (&m_channel);

            if (RD_SUCCESS != err_code)
            {
                status = true;
            }
        }
        else
        {
            status = true;
        }
    }
    else
    {
        status = true;
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    ri_uart_uninit (&m_channel);
    return status;
}

static rd_status_t uart_init_test (const ri_gpio_id_t input,
                                   const ri_gpio_id_t output)
{
    rd_status_t err_code = RD_SUCCESS;
    m_has_sent = false;
    m_has_received = false;
    memset (&rx_data, 0, sizeof (rx_data));
    ri_uart_init_t config =
    {
        .hwfc_enabled = false,
        .parity_enabled = false,
        .cts  = RI_GPIO_ID_UNUSED,
        .rts  = RI_GPIO_ID_UNUSED,
        .tx   = output,
        .rx   = input,
        .baud = RI_UART_BAUD_115200
    };
    err_code |= ri_uart_init (&m_channel);
    m_channel.on_evt = uart_isr;
    // TODO: Test different baudrates, parities etc.
    err_code |= ri_uart_config (&config);
    return err_code;
}

static bool uart_run_test (const char * const test_data)
{
    bool status = false;
    bool timeout = false;
    m_has_sent = false;
    m_has_received = false;
    memset (&rx_data, 0, sizeof (rx_data));
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg = { 0 };
    msg.repeat_count = 1;
    size_t written = 0;
    written = snprintf ( (char *) & msg.data, RI_COMM_MESSAGE_MAX_LENGTH, "%s", test_data);
    msg.data_length = (written > RI_COMM_MESSAGE_MAX_LENGTH)
                      ? RI_COMM_MESSAGE_MAX_LENGTH : written;
    ri_rtc_init();
    err_code |= m_channel.send (&msg);

    while (! (m_has_sent && m_has_received)
            && (!timeout))
    {
        if (TEST_TIMEOUT_MS < ri_rtc_millis())
        {
            timeout = true;
        }
    }

    if ( (RD_SUCCESS != err_code)
            || memcmp (&rx_data, &msg, sizeof (ri_comm_message_t))
            || timeout
            || (m_has_received != 1))
    {
        status = true;
    }

    ri_rtc_uninit();
    return status;
}

/**
 * Send data, receive it, verify send and receive match.
 */
bool ri_uart_tx_test (const rd_test_print_fp printfp, const ri_gpio_id_t input,
                      const ri_gpio_id_t output)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"tx_rx\":");
    err_code |= uart_init_test (input, output);
    char test_data[] =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor"
        " incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis "
        "nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
        "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
        "fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
        "sunt in culpa qui officia deserunt mollit anim id est laborum";
    test_data[RI_COMM_MESSAGE_MAX_LENGTH] = '\n';

    if (RD_SUCCESS == err_code)
    {
        status |= uart_run_test (test_data);
    }
    else
    {
        status = true;
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    (void) ri_uart_uninit (&m_channel);
    return status;
}

/**
 * Test overflow of RX buffer before LF
 */
bool ri_uart_rx_test (const rd_test_print_fp printfp, const ri_gpio_id_t input,
                      const ri_gpio_id_t output)
{
    bool status = false;
    bool timeout = false;
    m_has_sent = 0;
    m_has_received = 0;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"rx_corrupt\":");
    err_code |= uart_init_test (input, output);
    const char test_data[] =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
        "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis "
        "nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
        "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
        "fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
        "culpa qui officia deserunt mollit anim id est laborum\n";
    ri_comm_message_t msg = { 0 };
    msg.repeat_count = 1;
    memcpy (msg.data, test_data, RI_COMM_MESSAGE_MAX_LENGTH);
    msg.data_length = RI_COMM_MESSAGE_MAX_LENGTH;

    if (RD_SUCCESS == err_code)
    {
        m_channel.on_evt = uart_isr;
        ri_rtc_init();
        err_code |= m_channel.send (&msg);

        while (! (m_has_sent && m_has_received)
                && (!timeout))
        {
            if (TEST_TIMEOUT_MS < ri_rtc_millis())
            {
                timeout = true;
            }
        }

        size_t written = snprintf ( (char *) msg.data, RI_COMM_MESSAGE_MAX_LENGTH, "%s",
                                    test_data + RI_COMM_MESSAGE_MAX_LENGTH);
        msg.data_length = written;
        const char * cutoff_index = test_data + strlen (test_data) - written;
        memset (&rx_data, 0, sizeof (rx_data));
        err_code |= m_channel.send (&msg);

        while ( ( (m_has_sent < 2) && (m_has_received < 2))
                && (!timeout))
        {
            if (TEST_TIMEOUT_MS < ri_rtc_millis())
            {
                timeout = true;
            }
        }

        if ( (RD_SUCCESS != err_code)
                || memcmp (&rx_data.data, cutoff_index, rx_data.data_length)
                || timeout
                || (m_has_received != 2)
                || (m_has_sent != 2))
        {
            status = true;
        }

        (void) ri_rtc_uninit();
    }
    else
    {
        status = true;
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    (void) ri_uart_uninit (&m_channel);
    return status;
}

bool ri_uart_rx_short_test (const rd_test_print_fp printfp, const ri_gpio_id_t input,
                            const ri_gpio_id_t output)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"rx_short\":");
    err_code |= uart_init_test (input, output);
    const char test_data[] = "Lorem ipsum dolor sit amet\n";

    if (RD_SUCCESS == err_code)
    {
        status |= uart_run_test (test_data);
    }
    else
    {
        status = true;
    }

    if (status)
    {
        printfp ("\"fail\"\r\n");
    }
    else
    {
        printfp ("\"pass\"\r\n");
    }

    (void) ri_uart_uninit (&m_channel);
    return status;
}

bool ri_communication_uart_run_integration_test (const rd_test_print_fp printfp,
        const ri_gpio_id_t input, const ri_gpio_id_t output)
{
    rd_status_t status = false;
    printfp ("\"uart\":{\r\n");
    status |= ri_uart_init_test (printfp);
    status |= ri_uart_tx_test (printfp, input, output);
    status |= ri_uart_rx_test (printfp, input, output);
    status |= ri_uart_rx_short_test (printfp, input, output);
    printfp ("},\r\n");
    return status;
}
#endif