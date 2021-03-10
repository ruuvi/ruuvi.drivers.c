#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_communication_nfc_test.h"
#include "ruuvi_interface_communication_nfc.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_interface_rtc.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/**
 * @addtogroup NFC
 * @{
 */
/**
 * @file ruuvi_interface_communication_nfc_test.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Integration test NFC implementation.
 */
#define NFC_TEST_TIMEOUT_MS (20000U)

static ri_comm_channel_t m_channel;
static bool m_has_connected;
static bool m_has_disconnected;
static bool m_has_sent;
static bool m_has_received;
static ri_comm_message_t rx_data;

static rd_status_t nfc_isr (ri_comm_evt_t evt,
                            void * p_data, size_t data_len)
{
    switch (evt)
    {
        // Note: This gets called only after the NFC notifications have been registered.
        case RI_COMM_CONNECTED:
            m_has_connected = true;
            break;

        case RI_COMM_DISCONNECTED:
            m_has_disconnected = true;
            break;

        case RI_COMM_SENT:
            m_has_sent = true;
            break;

        case RI_COMM_RECEIVED:
            m_has_received = true;
            rx_data.data_length = RI_COMM_MESSAGE_MAX_LENGTH;
            m_channel.read (&rx_data);
            break;

        default:
            break;
    }

    return RD_SUCCESS;
}

/*
 * Initializes NFC hardware.
 *
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_INVALID_STATE if NFC is already initialized
 */
static bool ri_nfc_init_test (const rd_test_print_fp printfp)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"init\":");
    err_code |= ri_nfc_init (&m_channel);

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_nfc_init (&m_channel);

        if (RD_ERROR_INVALID_STATE == err_code)
        {
            err_code = ri_nfc_uninit (&m_channel);
            err_code = ri_nfc_init (&m_channel);

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

    ri_nfc_uninit (&m_channel);
    return status;
}

/**
 * Send data as ascii-encoded binary.
 *
 * Returns RD_SUCCESS if the data was placed in buffer
 * Returns error code from the stack if data could not be placed to the buffer
 */
bool ri_nfc_rx_test (const rd_test_print_fp printfp)
{
    bool status = false;
    uint64_t start_time = 0;
    bool timeout = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"tx_rx\":");
    err_code |= ri_nfc_init (&m_channel);
    const char test_data[] = "Lorem Ipsum";
    ri_comm_message_t msg = {0};
    snprintf ( (char *) & msg.data, RI_COMM_MESSAGE_MAX_LENGTH, "%s", test_data);
    msg.data_length = strlen (test_data);

    if (RD_SUCCESS == err_code)
    {
        m_channel.on_evt = nfc_isr;
        err_code |= m_channel.send (&msg);
        ri_rtc_init();
        start_time = ri_rtc_millis();

        while (! (m_has_connected
                  && m_has_disconnected
                  && m_has_sent
                  && m_has_received))
        {
            if ( (start_time + NFC_TEST_TIMEOUT_MS) < ri_rtc_millis())
            {
                timeout = true;
                break;
            }
        }

        if (false == timeout)
        {
            if ( (RD_SUCCESS != err_code)
                    || memcmp (&rx_data, &msg, sizeof (ri_comm_message_t)))
            {
                status = true;
            }
        }
    }
    else
    {
        status = true;
    }

    if (timeout)
    {
        printfp ("\"timeout\"\r\n");
    }
    else
    {
        if (status)
        {
            printfp ("\"fail\"\r\n");
        }
        else
        {
            printfp ("\"pass\"\r\n");
        }
    }

    ri_rtc_uninit();
    ri_nfc_uninit (&m_channel);
    return status;
}

bool ri_communication_nfc_run_integration_test (const rd_test_print_fp printfp)
{
    rd_status_t status = false;
    printfp ("\"nfc\":{\r\n");
    status |= ri_nfc_init_test (printfp);
    status |= ri_nfc_rx_test (printfp);
    printfp ("}\r\n");
    return status;
}
#endif