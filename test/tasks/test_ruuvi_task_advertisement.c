/**
 * @file test_task_advertisement.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-11-18
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "unity.h"

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_task_advertisement.h"
#include "ruuvi_driver_error.h"

#include "mock_ruuvi_task_gatt.h"
#include "mock_ruuvi_interface_communication_ble_advertising.h"
#include "mock_ruuvi_interface_log.h"

#include <stdio.h>
#include <string.h>

#define ADV_INTERVAL_MS (1000U)
#define ADV_PWR_DBM     (0)
#define ADV_MANU_ID     (0xFFFFU)
#define SEND_COUNT_MAX (10U)

static uint32_t send_count = 0;
static uint32_t read_count = 0;
static bool m_con_cb;
static bool m_discon_cb;
static bool m_tx_cb;
static bool m_rx_cb;
static const char m_name[] = "Ceedling";



rd_status_t mock_send (ri_comm_message_t * const p_msg)
{
    rd_status_t err_code = RD_SUCCESS;
    send_count++;
    return err_code;
}

rd_status_t mock_read (ri_comm_message_t * const p_msg)
{
    read_count++;
    return RD_SUCCESS;
}

rd_status_t mock_uninit (ri_comm_channel_t * const p_channel)
{
    memset (p_channel, 0, sizeof (ri_comm_channel_t));
    return RD_SUCCESS;
}

rd_status_t mock_init (ri_comm_channel_t * const p_channel)
{
    p_channel->send   = mock_send;
    p_channel->read   = mock_read;
    p_channel->uninit = mock_uninit;
    p_channel->init   = mock_init;
    p_channel->on_evt = NULL;
    return RD_SUCCESS;
}

rd_status_t on_scan_isr (const ri_comm_evt_t evt, void * p_data, size_t data_len)
{
    // No action needed.
    return RD_SUCCESS;
}

static ri_comm_channel_t m_mock_channel;


void setUp (void)
{
    rd_status_t err_code = RD_SUCCESS;
    mock_init (&m_mock_channel);
    ri_adv_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_adv_init_ReturnArrayThruPtr_channel (&m_mock_channel, 1);
    int8_t power = ADV_PWR_DBM;
    ri_adv_tx_interval_set_ExpectAndReturn (ADV_INTERVAL_MS, RD_SUCCESS);
    ri_adv_tx_power_set_ExpectWithArrayAndReturn (&power, sizeof (power), RD_SUCCESS);
    ri_adv_type_set_ExpectAndReturn (NONCONNECTABLE_NONSCANNABLE, RD_SUCCESS);
    ri_adv_manufacturer_id_set_ExpectAndReturn (ADV_MANU_ID, RD_SUCCESS);
    rt_adv_init_t init;
    init.adv_interval_ms = ADV_INTERVAL_MS;
    init.adv_pwr_dbm = ADV_PWR_DBM;
    init.manufacturer_id = ADV_MANU_ID;
    err_code = rt_adv_init (&init);
    send_count = 0;
    read_count = 0;
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (rt_adv_is_init());
}

void tearDown (void)
{
    rd_status_t err_code = RD_SUCCESS;
    mock_uninit (&m_mock_channel);
    ri_adv_uninit_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_uninit_ReturnArrayThruPtr_channel (
        &m_mock_channel, 1);
    err_code = rt_adv_uninit();
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (!rt_adv_is_init());
}

/**
 * @brief Initializes data advertising.
 *
 * The function setups advertisement interval, advertisement power, advertisement type,
 * manufacturer ID for manufacturer specific data according to constants in
 * application_config.h and ruuvi_boards.h
 * After calling this function advertisement data can be queued into advertisement buffer.
 * You should queue at least one message into buffer before starting advertising.
 *
 * @retval @c RD_SUCCESS on success.
 * @retval @c RD_ERROR_INVALID_STATE if advertising is already initialized.
 * @®etval @c RD_ERROR_INVALID_PARAM if configuration constant is invalid.
 */
void test_rt_adv_init_ok (void)
{
    tearDown();
    mock_init (&m_mock_channel);
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_init_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_init_ReturnArrayThruPtr_channel (
        &m_mock_channel, 1);
    int8_t power = ADV_PWR_DBM;
    ri_adv_tx_interval_set_ExpectAndReturn (
        ADV_INTERVAL_MS, RD_SUCCESS);
    ri_adv_tx_power_set_ExpectWithArrayAndReturn (
        &power, sizeof (power), RD_SUCCESS);
    ri_adv_type_set_ExpectAndReturn (
        NONCONNECTABLE_NONSCANNABLE, RD_SUCCESS);
    ri_adv_manufacturer_id_set_ExpectAndReturn (
        ADV_MANU_ID, RD_SUCCESS);
    rt_adv_init_t init;
    init.adv_interval_ms = ADV_INTERVAL_MS;
    init.adv_pwr_dbm = ADV_PWR_DBM;
    init.manufacturer_id = ADV_MANU_ID;
    err_code = rt_adv_init (&init);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (rt_adv_is_init());
}

void test_rt_adv_init_invalid_interval (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_init_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_init_ReturnArrayThruPtr_channel (
        &m_mock_channel, 1);
    ri_adv_tx_interval_set_ExpectAndReturn (
        ADV_INTERVAL_MS, RD_ERROR_INVALID_PARAM);
    int8_t power = ADV_PWR_DBM;
    ri_adv_tx_power_set_ExpectWithArrayAndReturn (
        &power, sizeof (power), RD_SUCCESS);
    ri_adv_type_set_ExpectAndReturn (
        NONCONNECTABLE_NONSCANNABLE, RD_SUCCESS);
    ri_adv_manufacturer_id_set_ExpectAndReturn (
        ADV_MANU_ID, RD_SUCCESS);
    rt_adv_init_t init;
    init.adv_interval_ms = ADV_INTERVAL_MS;
    init.adv_pwr_dbm = ADV_PWR_DBM;
    init.manufacturer_id = ADV_MANU_ID;
    err_code = rt_adv_init (&init);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
    TEST_ASSERT (!rt_adv_is_init());
}

void test_rt_adv_init_invalid_power (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_init_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_init_ReturnArrayThruPtr_channel (
        &m_mock_channel, 1);
    int8_t power = ADV_PWR_DBM;
    ri_adv_tx_interval_set_ExpectAndReturn (
        ADV_INTERVAL_MS, RD_SUCCESS);
    ri_adv_tx_power_set_ExpectWithArrayAndReturn (
        &power, sizeof (power), RD_ERROR_INVALID_PARAM);
    ri_adv_type_set_ExpectAndReturn (
        NONCONNECTABLE_NONSCANNABLE, RD_SUCCESS);
    ri_adv_manufacturer_id_set_ExpectAndReturn (
        ADV_MANU_ID, RD_SUCCESS);
    rt_adv_init_t init;
    init.adv_interval_ms = ADV_INTERVAL_MS;
    init.adv_pwr_dbm = ADV_PWR_DBM;
    init.manufacturer_id = ADV_MANU_ID;
    err_code = rt_adv_init (&init);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
    TEST_ASSERT (!rt_adv_is_init());
}

void test_rt_adv_init_invalid_type (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_init_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_init_ReturnArrayThruPtr_channel (
        &m_mock_channel, 1);
    int8_t power = ADV_PWR_DBM;
    ri_adv_tx_interval_set_ExpectAndReturn (
        ADV_INTERVAL_MS, RD_SUCCESS);
    ri_adv_tx_power_set_ExpectWithArrayAndReturn (
        &power, sizeof (power), RD_SUCCESS);
    ri_adv_type_t type = NONCONNECTABLE_NONSCANNABLE;
    ri_adv_type_set_ExpectAndReturn (type,
                                     RD_ERROR_INVALID_PARAM);
    ri_adv_manufacturer_id_set_ExpectAndReturn (
        ADV_MANU_ID, RD_SUCCESS);
    rt_adv_init_t init;
    init.adv_interval_ms = ADV_INTERVAL_MS;
    init.adv_pwr_dbm = ADV_PWR_DBM;
    init.manufacturer_id = ADV_MANU_ID;
    err_code = rt_adv_init (&init);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
    TEST_ASSERT (!rt_adv_is_init());
}

void test_rt_adv_init_twice (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rt_adv_init_t init;
    init.adv_interval_ms = ADV_INTERVAL_MS;
    init.adv_pwr_dbm = ADV_PWR_DBM;
    init.manufacturer_id = ADV_MANU_ID;
    err_code = rt_adv_init (&init);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
    TEST_ASSERT (rt_adv_is_init());
}

/**
 * @brief Uninitializes data advertising.
 *
 * Can be called even if advertising was not initialized.
 * Does not uninitialize timers even if they were initialized for advertisement module.
 * Clears previous advertisement data if there was any.
 *
 * @retval @c RD_SUCCESS on success
 * @retval error code from stack on error
 */
void test_rt_adv_uninit (void)
{
    rd_status_t err_code = RD_SUCCESS;
    mock_uninit (&m_mock_channel);
    ri_adv_uninit_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_uninit_ReturnArrayThruPtr_channel (
        &m_mock_channel, 1);
    err_code = rt_adv_uninit();
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (!rt_adv_is_init());
}

/**
 * @brief Stops advertising.
 *
 * @retval RD_SUCCESS on success
 * @retval error code from stack on error
 */
void test_rt_adv_stop_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_stop_ExpectAndReturn (RD_SUCCESS);
    err_code = rt_adv_stop();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

/** @brief Send given message as a BLE advertisement.
 *
 *  This function configures the primary advertisement packet with the flags and manufacturer specific data.
 *  Payload of the msg will be sent as the manufacturer specific data payload.
 *  Manufacturer ID is defined by ADV_MANU_ID in ruuvi_boards.h.
 *
 *  If the device is connectable, call @ref rt_adv_connectability to setup the
 *  scan response and flags to advertise connectability.
 *
 *  @param[in] msg message to be sent as manufacturer specific data payload
 *  @retval    RD_ERROR_NULL if msg is NULL
 *  @retval    RD_ERROR_INVALID_STATE if advertising isn't initialized or started.
 *  @retval    RD_ERROR_DATA_SIZE if payload size is larger than 24 bytes
 *  @retval    error code from stack on other error.
 */
void test_rt_adv_send_data_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_adv_send_data (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_adv_send_data_not_init (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t message;
    message.data_length = 10;
    err_code = rt_adv_send_data (&message);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_adv_send_data_excess_size_25 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t message;
    message.data_length = 25;
    err_code = rt_adv_send_data (&message);
    TEST_ASSERT (RD_ERROR_DATA_SIZE == err_code);
}

/** @brief Start advertising BLE GATT connection
 *
 *  This function configures the primary advertisement to be SCANNABLE_CONNECTABLE and
 *  sets up a scan response which has given device name (max 10 characters + NULL)
 *  and UUID of Nordic UART Service.
 *
 *  Be sure to configure the GATT
 *
 *  @param[in] enable true to enable connectability, false to disable.
 *  @param[in] name NULL-terminated string representing device name, max 10 Chars + NULL.
 *  @retval    RD_SUCCESS if operation was finished as expected.
 *  @retval    RD_ERROR_NULL if name is NULL and trying to enable the scan response
 *  @retval    RD_ERROR_INVALID_STATE if advertising isn't initialized or started.
 *  @retval    RD_ERROR_INVALID_LENGTH if name size exceeds 10 bytes + NULL
 *  @retval    error code from stack on other error.
 */
void test_rt_adv_connectability_set_nus_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    const char name[] = "Ceedling";
    ri_adv_type_set_ExpectAndReturn (CONNECTABLE_SCANNABLE, RD_SUCCESS);
    rt_gatt_is_nus_enabled_ExpectAndReturn (true);
    ri_adv_scan_response_setup_ExpectAndReturn (name, true, RD_SUCCESS);
    err_code = rt_adv_connectability_set (true, name);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adv_connectability_set_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    const char name[] = "Ceedling";
    ri_adv_type_set_ExpectAndReturn (CONNECTABLE_SCANNABLE, RD_SUCCESS);
    rt_gatt_is_nus_enabled_ExpectAndReturn (false);
    ri_adv_scan_response_setup_ExpectAndReturn (name, false, RD_SUCCESS);
    err_code = rt_adv_connectability_set (true, name);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adv_connectability_set_not_init (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_adv_connectability_set (true, "Ceedling");
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_adv_connectability_set_error_long_name (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_adv_connectability_set (true, "Ceedling spawnling");
    TEST_ASSERT (RD_ERROR_INVALID_LENGTH == err_code);
}

void test_rt_adv_connectability_set_error_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_adv_connectability_set (true, NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_adv_connectability_set_off (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_type_set_ExpectAndReturn (
        NONCONNECTABLE_NONSCANNABLE, RD_SUCCESS);
    err_code = rt_adv_connectability_set (false, NULL);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

/** @brief Start scanning BLE advertisements.
 *
 * This is non-blocking, you'll need to handle incoming events.
 *
 * PHY to be scanned is determined by radio initialization.
 * If you have selected 2 MBit / s PHY, primary advertisements
 * are scanned at 1 Mbit / s and secondary extended advertisement
 * can be scanned at at 2 MBit / s.
 *
 * Scan is done at 7000 ms window and interval, this consumes
 * a lot of power and may collapse a coin cell battery.
 *
 * Events are:
 *   - on_evt(RI_COMM_RECEIVED, scan, sizeof(ri_adv_scan_t));
 *   - on_evt(RI_COMM_TIMEOUT, NULL, 0);
 *
 *  @param[in] on_evt Event handler for scan results.
 *  @retval    RD_SUCCESS Scanning was started.
 *  @retval    RD_ERROR_INVALID_STATE Advertising isn't initialized.
 *  @return    error code from stack on other error.
 *
 * @note Scanning is stopped on timeout, you can restart the scan on event handler.
 * @warning Event handler is called in interrupt context.
 */
void test_rt_adv_scan_start_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_scan_start_ExpectAndReturn (RT_ADV_SCAN_INTERVAL_MS, RT_ADV_SCAN_WINDOW_MS,
                                       RD_SUCCESS);
    err_code |= rt_adv_scan_start (on_scan_isr);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adv_scan_start_not_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    tearDown();
    err_code |= rt_adv_scan_start (on_scan_isr);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Abort scanning.
 *
 * After calling this function scanning is immediately stopped.
 *
 */
void test_rt_adv_scan_stop_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_scan_stop_ExpectAndReturn (RD_SUCCESS);
    err_code |= rt_adv_scan_stop ();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adv_scan_stop_not_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    tearDown();
    err_code |= rt_adv_scan_stop ();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}
