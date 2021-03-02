#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_communication_ble_gatt_test.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#include "ruuvi_interface_communication_ble_gatt.h"
#include "ruuvi_interface_communication_radio_test.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_flash.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_timer.h"
#include "ruuvi_interface_yield.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/**
 * @addtogroup VLE
 * @{
 */
/**
 * @file ruuvi_interface_communication_ble_gatt_test.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-06-18
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Integration test BLE GATT implementation.
 */
#define GATT_TX_TEST_TIMEOUT_MS (10000U)

static ri_comm_channel_t m_channel;
static ri_comm_channel_t m_adv;     //!< Required to establish GATT connection
static volatile bool m_has_connected;
static volatile bool m_has_disconnected;
static volatile bool m_has_sent;
static volatile bool m_has_received;
static ri_comm_message_t rx_data;

static rd_status_t ble_isr (ri_comm_evt_t evt,
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

/**
 * @brief Initialize GATT module.
 *
 * @param[out] channel Interface used for communicating through advertisements.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if radio is not already initialized.
 *
 * @note Modulation used on the gatt depends on how radio was initialized.
 */
static bool ri_gatt_init_null_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_gatt_init ();
    err_code |= ri_gatt_nus_init (NULL);
    return (RD_ERROR_NULL != err_code);
}

static bool ri_gatt_init_twice_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_gatt_init ();
    err_code |= ri_gatt_init ();
    return (RD_ERROR_INVALID_STATE != err_code);
}

static bool ri_gatt_init_norad_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_gatt_init ();
    return (RD_ERROR_INVALID_STATE != err_code);
}

static bool ri_gatt_init_test (const rd_test_print_fp printfp,
                               const ri_radio_modulation_t modulation)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"init\":");
    status |= ri_gatt_init_norad_test();
    err_code |= ri_timer_init();
    err_code |= ri_flash_init();
    err_code |= ri_radio_init (modulation);
    err_code |= ri_gatt_init ();
    // nRF SDK requires complete radio re-init.
    err_code |= ri_radio_uninit ();
    err_code |= ri_gatt_uninit ();
    err_code |= ri_radio_init (modulation);

    if (RD_SUCCESS == err_code)
    {
        status |= ri_gatt_init_null_test();
        status |= ri_gatt_init_twice_test();
    }
    else
    {
        status |= true;
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    ri_gatt_nus_uninit (&m_channel);
    ri_radio_uninit();
    ri_gatt_uninit ();
    ri_flash_uninit();
    ri_timer_uninit();
    return status;
}

/* Send data slowfly for 10 s to verify power consumption when little data is sent */
static bool tx_slow_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    const char test_data[] = "Test slow tx: ";
    ri_comm_message_t msg = {0};
    uint16_t msg_index = 0U;

    for (uint8_t ii = 0U; ii < 10U; ii++)
    {
        snprintf ( (char *) & msg.data, RI_COMM_MESSAGE_MAX_LENGTH, "%s%d", test_data,
                   msg_index++);
        msg.data_length = strlen (test_data);
        msg.repeat_count = 1U;
        err_code |= m_channel.send (&msg);
        ri_delay_ms (1000U);
    }

    return (RD_SUCCESS != err_code);
}

/* Check throughput, return throughput- */
static float tx_throughput_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    const char test_data[] = "msg: "; // 5 chars
    ri_comm_message_t msg = {0};
    uint64_t test_start = ri_rtc_millis();

    for (uint16_t ii = 0U; ii < TEST_GATT_PACKET_NUM;)
    {
        err_code = RD_SUCCESS;

        while (RD_SUCCESS == err_code)
        {
            // 5 + 5 + 3 + 5 + 1 + NULL = 20 bytes
            snprintf ( (char *) & msg.data, RI_COMM_MESSAGE_MAX_LENGTH, "%s%05d / %05d.", test_data,
                       ii, TEST_GATT_PACKET_NUM);
            msg.data_length = TEST_GATT_PACKET_LEN;
            msg.repeat_count = 1U;
            err_code |= m_channel.send (&msg);

            if (RD_SUCCESS == err_code)
            {
                ii++;
            }
        }

        ri_yield();
    }

    uint64_t test_end = ri_rtc_millis();
    return ( (float) (TEST_GATT_PACKET_NUM * TEST_GATT_PACKET_LEN)) / ( ( (float) (
                test_end - test_start)) / 1000.0F);
}

bool ri_gatt_tx_test (const rd_test_print_fp printfp,
                      const ri_radio_modulation_t modulation)
{
    bool status = false;
    m_has_connected = false;
    uint64_t start_time = 0;
    bool timeout = false;
    rd_status_t err_code = RD_SUCCESS;
    float throughput = 0;
    char throughput_string[100];
    printfp ("\"tx\":");
    // RTC + timer required for low-power yield.
    err_code |= ri_rtc_init();
    err_code |= ri_timer_init();
    err_code |= ri_yield_low_power_enable (true);
    err_code |= ri_flash_init();
    err_code |= ri_radio_init (modulation);
    // Advertise NUS to tester.
    err_code |= ri_adv_init (&m_adv);
    err_code |= ri_adv_scan_response_setup ("RuuviTest", true);
    err_code |= ri_adv_type_set (CONNECTABLE_SCANNABLE);
    err_code |= ri_adv_tx_interval_set (100U);
    err_code |= ri_gatt_init ();
    err_code |= ri_gatt_nus_init (&m_channel);
    const char test_data[] = "Lorem Ipsum";
    ri_comm_message_t msg = {0};
    snprintf ( (char *) & msg.data, RI_COMM_MESSAGE_MAX_LENGTH, "%s", test_data);
    msg.data_length = strlen (test_data);
    msg.repeat_count = 200U; // 200 * 100ms => 20 s to connect.
    err_code |=  m_adv.send (&msg);

    if (RD_SUCCESS == err_code)
    {
        m_channel.on_evt = ble_isr;
        start_time = ri_rtc_millis();

        while (! (m_has_connected))
        {
            if ( (start_time + GATT_TX_TEST_TIMEOUT_MS) < ri_rtc_millis())
            {
                timeout = true;
                break;
            }
        }

        if (false == timeout)
        {
            status |= tx_slow_test ();
            throughput = tx_throughput_test();

            if (0 == throughput)
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
        printfp ("\"timeout\",\r\n");
    }
    else
    {
        if (status)
        {
            printfp ("\"fail\",\r\n");
        }
        else
        {
            printfp ("\"pass\",\r\n");
        }
    }

    snprintf (throughput_string, sizeof (throughput_string),
              "\"throughput\":\"%.3f bps\"\r\n",
              throughput);
    printfp (throughput_string);
    ri_gatt_nus_uninit (&m_channel);
    ri_adv_uninit (&m_adv);
    ri_delay_ms (2000); //!< Wait for disconnect packet to go through.
    ri_radio_uninit();
    ri_gatt_uninit (); // GATT can only be uninit after radio.
    err_code |= ri_flash_uninit();
    ri_yield_low_power_enable (false);
    ri_timer_uninit();
    ri_rtc_uninit();
    return status;
}

bool ri_communication_ble_gatt_run_integration_test (const rd_test_print_fp printfp,
        const ri_radio_modulation_t modulation)
{
    rd_status_t status = false;
    printfp ("\"ble_gatt_");
    print_modulation (printfp, modulation);
    printfp ("\":{\r\n");
    status |= ri_gatt_init_test (printfp, modulation);
    status |= ri_gatt_tx_test (printfp, modulation);
    printfp ("},\r\n");
    return status;
}
#endif