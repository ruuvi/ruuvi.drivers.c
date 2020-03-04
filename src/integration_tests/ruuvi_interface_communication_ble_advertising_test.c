#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS && 0
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_communication_ble_advertising_test.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_yield.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/**
 * @addtogroup BLE
 * @{
 */
/**
 * @file ruuvi_interface_communication_ble_advertising_test.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Integration test BLE advertising implementation.
 */

static ri_communication_t m_channel;
static bool m_has_connected;
static bool m_has_disconnected;
static bool m_has_sent;
static bool m_has_received;
static ri_communication_message_t rx_data;

static rd_status_t ble_isr (ri_communication_evt_t evt,
                            void * p_data, size_t data_len)
{
    switch (evt)
    {
        // Note: This gets called only after the NFC notifications have been registered.
        case RI_COMMUNICATION_CONNECTED:
            m_has_connected = true;
            break;

        case RI_COMMUNICATION_DISCONNECTED:
            m_has_disconnected = true;
            break;

        case RI_COMMUNICATION_SENT:
            m_has_sent = true;
            break;

        case RI_COMMUNICATION_RECEIVED:
            m_has_received = true;
            break;

        default:
            break;
    }

    return RD_SUCCESS;
}

/*
 * Test BLE radio initialization.
 *
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_INVALID_STATE if BLE is already initialized
 */
static bool ri_ble_init_test (const rd_test_print_fp printfp)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"init\":");
    err_code |= ri_nfc_init (&m_channel);

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_ble_init (&m_channel);

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
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"tx_rx\":");
    err_code |= ri_nfc_init (&m_channel);
    const char test_data[] = "Lorem Ipsum";
    ri_communication_message_t msg = {0};
    snprintf (msg.data, RI_COMMUNICATION_MESSAGE_MAX_LENGTH, "%s", test_data);
    msg.data_length = strlen (msg.data);

    if (RD_SUCCESS == err_code)
    {
        m_channel.on_evt = nfc_isr;
        err_code |= m_channel.send (&msg);

        while (! (m_has_connected
                  && m_has_disconnected
                  && m_has_sent
                  && m_has_received))
        {
            ri_yield();
        }

        if ( (RD_SUCCESS != err_code)
                || memcmp (&rx_data, &msg, sizeof (ri_communication_message_t)))
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
        printfp ("\"fail\"\r\n");
    }
    else
    {
        printfp ("\"pass\"\r\n");
    }

    ri_nfc_uninit (&m_channel);
    return status;
}

bool ri_communication_nfc_run_integration_test (const rd_test_print_fp printfp)
{
    rd_status_t status = false;
    printfp ("\"nfc\":{\r\n");
    status |= ri_nfc_init_test (printfp);
    status |= ri_nfc_rx_test (printfp);
    printfp ("},\r\n");
    return status;
}
#endif