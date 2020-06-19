#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_communication_ble_advertising_test.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#include "ruuvi_interface_communication_radio_test.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_yield.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define TEST_STRING "Ave mundi!"
#define EXTENDED_STRING "Lorem ipsum dolor sit amet, consectetur adipiscing elit."\
              " Donec nisl "\
               "ligula, lacinia malesuada pellentesque molestie, venenatis "\
               "non neque. Cras eget ligula eget nunc pharetra tincidunt. "\
               "Etiam volutpat."

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

static ri_comm_channel_t m_channel;
static volatile bool m_has_connected;
static volatile bool m_has_disconnected;
static volatile bool m_has_sent;
static volatile bool m_has_received;
static volatile bool m_timeout;

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
            break;

        case RI_COMM_TIMEOUT:
            m_timeout = true;
            break;

        default:
            break;
    }

    return RD_SUCCESS;
}


/**
 * @brief Initialize Advertising module and scanning module.
 *
 * @param[out] channel Interface used for communicating through advertisements.
 * @param[in]  adv_channels Physical channels used by radio. One or more.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if radio is not already initialized.
 *
 * @note Modulation used on the advertisement depends on how radio was initialized.
 */

static bool ri_adv_init_null_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_adv_init (NULL);
    return (RD_ERROR_NULL != err_code);
}

static bool ri_adv_init_twice_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_adv_init (&m_channel);
    err_code |= ri_adv_init (&m_channel);
    return (RD_ERROR_INVALID_STATE != err_code);
}

static bool ri_adv_init_norad_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_adv_init (&m_channel);
    return (RD_ERROR_INVALID_STATE != err_code);
}

static bool ri_adv_init_test (const rd_test_print_fp printfp,
                              const ri_radio_modulation_t modulation)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"init\":");
    status |= ri_adv_init_norad_test();
    err_code |= ri_radio_init (modulation);
    err_code |= ri_adv_init (&m_channel);
    err_code |= ri_adv_uninit (&m_channel);

    if (RD_SUCCESS == err_code)
    {
        status |= ri_adv_init_null_test();
        status |= ri_adv_init_twice_test();
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

    ri_adv_uninit (&m_channel);
    ri_radio_uninit();
    return status;
}

/**
 * @brief Setter for broadcast advertisement interval.
 *
 * @param[in] ms Milliseconds, random delay of 0 - 10 ms will be added to the interval on
 *               every TX to avoid collisions. min 100 ms, max 10 000 ms.
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_INVALID_PARAM if the parameter is outside allowed range.
 */

static bool ri_adv_interval_short_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_adv_tx_interval_set (RI_TEST_ADV_TOO_FAST);
    return (RD_ERROR_INVALID_PARAM != err_code);
}

static bool ri_adv_interval_long_test (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_adv_tx_interval_set (RI_TEST_ADV_TOO_SLOW);
    return (RD_ERROR_INVALID_PARAM != err_code);
}

static bool ri_adv_tx_test (ri_comm_message_t * const msg)
{
    rd_status_t err_code = RD_SUCCESS;
    uint32_t interval = 0;
    bool status = false;
    err_code |= ri_adv_tx_interval_set (RI_TEST_ADV_FAST);

    if (RD_SUCCESS == err_code)
    {
        m_has_sent = false;
        m_channel.send (msg);
        uint64_t start = ri_rtc_millis();

        while (!m_has_sent
                && (ri_rtc_millis() - start) < (RI_TEST_ADV_FAST * msg->repeat_count))
        {
            ri_yield();
        }

        uint64_t end = ri_rtc_millis();

        if ( (RI_TEST_ADV_FAST > (end - start))
                || (RI_TEST_ADV_FAST  * (msg->repeat_count - 1) + (2 * RI_ADV_RND_DELAY)) < (end - start))
        {
            status = true;
        }
    }
    else
    {
        status = true;
    }

    err_code |= ri_adv_tx_interval_get (NULL);
    status |= (RD_ERROR_NULL != err_code);
    err_code = ri_adv_tx_interval_get (&interval);
    status |= (RD_SUCCESS != err_code);
    status |= (RI_TEST_ADV_FAST != interval);
    return status;
}

static bool ri_adv_interval_test (const rd_test_print_fp printfp,
                                  const ri_radio_modulation_t modulation)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg;
    msg.repeat_count = 2;
    snprintf ( (char *) & (msg.data), sizeof (msg.data), TEST_STRING);
    msg.data_length = strlen (TEST_STRING);
    printfp ("\"interval\":");
    err_code |= ri_rtc_init();
    err_code |= ri_radio_init (modulation);
    err_code |= ri_adv_init (&m_channel);
    m_channel.on_evt = &ble_isr;
    status |= ri_adv_interval_short_test();
    status |= ri_adv_interval_long_test();
    status |= ri_adv_tx_test (&msg);

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    (void) ri_rtc_uninit();
    (void) ri_adv_uninit (&m_channel);
    (void) ri_radio_uninit();
    return status;
}

static bool ri_adv_extended_test (const rd_test_print_fp printfp,
                                  const ri_radio_modulation_t modulation)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg;
    msg.repeat_count = 2;
    snprintf ( (char *) & (msg.data), sizeof (msg.data), EXTENDED_STRING);
    msg.data_length = strlen (EXTENDED_STRING);
    printfp ("\"extended\":");
    err_code |= ri_rtc_init();
    err_code |= ri_radio_init (modulation);
    err_code |= ri_adv_init (&m_channel);
    m_channel.on_evt = &ble_isr;
    status |= ri_adv_tx_test (&msg);

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    (void) ri_rtc_uninit();
    (void) ri_adv_uninit (&m_channel);
    (void) ri_radio_uninit();
    return status;
}

/**
 * @brief Set radio TX power.
 *
 * @param[in,out] dbm Radio power. Supported values are board-dependent.
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_NULL if dbm is NULL.
 * @retval RD_ERROR_INVALID_STATE  if radio is not initialized.
 * @retval RD_ERROR_INVALID_PARAM if given power level is not supported.
 */
static bool ri_adv_pwr_test_noinit (void)
{
    int8_t pwr = 0;
    rd_status_t err_code = ri_adv_tx_power_set (&pwr);
    return (RD_ERROR_INVALID_STATE != err_code);
}

static bool ri_adv_pwr_test_null (void)
{
    rd_status_t err_code = ri_adv_tx_power_set (NULL);
    return (RD_ERROR_NULL != err_code);
}

static bool ri_adv_power_test (const rd_test_print_fp printfp,
                               const ri_radio_modulation_t modulation)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    printfp ("\"power\":");
    int8_t pwr = -127;
    int8_t original = pwr;
    status |= ri_adv_pwr_test_noinit();
    err_code |= ri_radio_init (modulation);
    err_code |= ri_adv_init (&m_channel);
    status |= ri_adv_pwr_test_null();
    err_code = ri_adv_tx_power_set (&pwr);
    // pwr should be updated.
    status |= (original == pwr);
    original = pwr;
    pwr = 126;
    // Check invalid param.
    err_code = ri_adv_tx_power_set (&pwr);
    status |= (RD_ERROR_INVALID_PARAM != err_code);
    // Check that power was not modified with invalid param.
    err_code = ri_adv_tx_power_get (&pwr);
    status |= (original != pwr);

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    (void) ri_adv_uninit (&m_channel);
    (void) ri_radio_uninit();
    return status;
}

/**
 *  @brief setup scan window interval and window size.
 *
 *  The scan window interval must be larger or equivalent to window size.
 *  Example: Interval 1000 ms, window size 100 ms.
 *  The scanning will scan 100 ms at channel 37, wait 900 ms, scan 100 ms at channel 38,
 *  wait 900 ms, scan 100 ms at channel 39, wait 900 ms and start again at channel 37.
 *
 *  Scan must be triggered via @ref ri_adv_scan_start and will stop after all channels and phys
 *  are scanned or @ref ri_adv_scan_stop is called.
 *
 *  When incoming packet is detected, communication channel .on_evt -handler is called with
 *  on_evt(RI_COMMUNICATION_RECEIVED, void* p_data, size_t data_len) where p_data points
 *  to @ref ri_adv_scan_t and data_len is size of scan data.
 *  To get associated ri_communication_msg_t, schedule a call to .receive in the ISR.
 *
 *  @param[in] window_interval_ms Interval of the scanning.
 *                                One scan interval will scan one phy / channel.
 *  @param[in] window_size_ms     Window size within interval.
 *
 *  @return RD_SUCCESS  on success.
 *  @return RD_ERROR_INVALID_STATE if scan is ongoing.
 *  @return RD_ERROR_INVALID_PARAM if window is larger than interval or values are otherwise invalid.
 */
static bool ri_adv_rx_interval_test (const rd_test_print_fp printfp,
                                     const ri_radio_modulation_t modulation)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    uint64_t test_start = 0;
    uint64_t test_end = 0;
    printfp ("\"scan\":");
    err_code |= ri_rtc_init();
    err_code |= ri_radio_init (modulation);
    err_code |= ri_adv_init (&m_channel);
    m_channel.on_evt = ble_isr;
    m_timeout = false;
    m_has_received;
    test_start = ri_rtc_millis();
    err_code |= ri_adv_scan_start (RI_TEST_ADV_SCAN_INTERVAL,
                                   RI_TEST_ADV_SCAN_WINDOW);

    while (!m_timeout
            && ( (RI_TEST_ADV_SCAN_CH_NUM + 1) * RI_TEST_ADV_SCAN_INTERVAL) > (ri_rtc_millis() -
                    test_start))
    {
        // Sleep - woken up on event
        // ri_yield();
        // Prevent loop being optimized away
        __asm__ ("");
    }

    test_end = ri_rtc_millis();
    const uint32_t test_min_ms = (RI_TEST_ADV_SCAN_CH_NUM - 1U) * RI_TEST_ADV_SCAN_INTERVAL;
    const uint32_t test_max_ms = (RI_TEST_ADV_SCAN_CH_NUM * RI_TEST_ADV_SCAN_INTERVAL) + 5U;
    const uint32_t test_time_ms = (test_end - test_start);

    // Fail if:
    //   * Scanned less than (number of channels - 1) * interval + 1 scan window
    //        - can timeout immediately after last scan window, not waiting for interval.
    //   * Scanned more than number if intervals + 1 scan window. Must finish asap, but
    //     allow a little margin.
    //   * Error code was returned.
    //   * No advertisements were seen.
    //   * Advertising didn't timeout at all.
    if ( (test_min_ms > test_time_ms)
            || (test_max_ms < test_time_ms)
            || (RD_SUCCESS != err_code)
            || (!m_has_received)
            || (!m_timeout))
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

    (void) ri_rtc_uninit();
    (void) ri_adv_uninit (&m_channel);
    (void) ri_radio_uninit();
    return status;
}

/**
 * @brief Configure advertising data with a scan response.
 * The scan response must be separately enabled by setting advertisement type as
 * scannable.
 *
 * @param[in] name Name to advertise, NULL-terminated string.
 *                 Max 27 characters if not advertising NUS, 10 characters if NUS is
 *                 advertised. NULL won't be included in advertisement.
 * @param[in] advertise_nus True to include Nordic UART Service UUID in scan response.
 * @retval @ref RD_SUCCESS on success
 * @retval @ref RD_ERROR_NULL if name is NULL
 * @retval @ref RD_ERROR_DATA_SIZE if name will be cut. However the abbreviated name will
 *              be set.
 */
//rd_status_t ri_adv_scan_response_setup (const char * const name,
//                                        const bool advertise_nus);

/**
 * @brief Configure the type of advertisement.
 *
 * Advertisement can be connectable, scannable, both or neither.
 * It is possible to setup scannable advertisement before setting up scan response,
 * in this case scan response will be 0-length and empty until scan resonse if configured.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if advertisements are not initialized.
 */
//rd_status_t ri_adv_type_set (ri_adv_type_t type);

/**
 * @brief Start scanning for BLE advertisements.
 *
 * After calling this function, the communication channel structure .on_evt will
 * get called with scan reports:
 * on_evt(RI_COMMUNICATION_RECEIVED, void* p_data, size_t data_len) where p_data points
 * to @ref ri_adv_scan_t and data_len is size of scan data.
 *
 * The modulation is determined by how radio was initialized, uninitialize radio and
 * re-initialize with a different modulation if needed.
 * Channels depend on advertisement initialization values, uninitialize and re-initialize
 * radio if needed.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if advertising is not initialized.
 */
rd_status_t ri_adv_scan_stop (void);

bool ri_communication_ble_advertising_run_integration_test (const rd_test_print_fp
        printfp,
        const ri_radio_modulation_t modulation)
{
    bool status = false;
    printfp ("\"ble_adv_");
    print_modulation (printfp, modulation);
    printfp ("\":{\r\n");
    status |= ri_adv_init_test (printfp, modulation);
    status |= ri_adv_interval_test (printfp, modulation);
    status |= ri_adv_extended_test (printfp, modulation);
    status |= ri_adv_power_test (printfp, modulation);
    status |= ri_adv_rx_interval_test (printfp, modulation);
    printfp ("},\r\n");
    return status;
}

#endif
