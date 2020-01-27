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
#include "mock_ruuvi_interface_communication_ble4_advertising.h"
#include "mock_ruuvi_interface_log.h"

#define ADV_INTERVAL_MS (1000U)
#define ADV_PWR_DBM     (0)
#define ADV_MANU_ID     (0xFFFFU)

static ri_communication_t m_adv_channel =
{
    .send = ri_adv_send,
    .read = ri_adv_receive,
    .init = ri_adv_init,
    .uninit = ri_adv_uninit
};

static ri_communication_t m_uninit_channel =
{
    .send = NULL,
    .read = NULL,
    .init = NULL,
    .uninit = NULL
};


void setUp (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_init_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_init_ReturnArrayThruPtr_channel (
        &m_adv_channel, 1);
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
    err_code = rt_adv_init(&init);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (rt_adv_is_init());
}

void tearDown (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_uninit_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_uninit_ReturnArrayThruPtr_channel (
        &m_uninit_channel, 1);
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
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_init_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_init_ReturnArrayThruPtr_channel (
        &m_adv_channel, 1);
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
    err_code = rt_adv_init(&init);
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
        &m_adv_channel, 1);
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
    err_code = rt_adv_init(&init);
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
        &m_adv_channel, 1);
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
    err_code = rt_adv_init(&init);
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
        &m_adv_channel, 1);
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
    err_code = rt_adv_init(&init);
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
    err_code = rt_adv_init(&init);
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
    ri_adv_uninit_ExpectAnyArgsAndReturn (
        RD_SUCCESS);
    ri_adv_uninit_ReturnArrayThruPtr_channel (
        &m_uninit_channel, 1);
    err_code = rt_adv_uninit();
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (!rt_adv_is_init());
}

/**
 * @brief Starts advertising.
 *
 * Before this function is called, you must initialize advertising and should
 * set some data into advertisement buffer. Otherwise empty advertisement packets are sent.
 * It might be desirable to send empty advertisement payloads as GATT connection
 * advertisements piggyback on data advertisements.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if advertising is not initialized.
 * returns error code from stack on error
 *
 */
void test_rt_adv_start_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adv_start_ExpectAndReturn (
        RD_SUCCESS);
    err_code = rt_adv_start();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adv_start_not_init (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_adv_start();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
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
    ri_adv_stop_ExpectAndReturn (
        RD_SUCCESS);
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

void test_rt_adv_send_data_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_communication_message_t message;
    message.data_length = 10;
    ri_adv_send_ExpectWithArrayAndReturn (&message,
            sizeof (message), RD_SUCCESS);
    err_code = rt_adv_send_data (&message);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

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
    ri_communication_message_t message;
    message.data_length = 10;
    err_code = rt_adv_send_data (&message);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_adv_send_data_excess_size_25 (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_communication_message_t message;
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
    rt_gatt_is_nus_enabled_ExpectAndReturn(true);
    ri_adv_scan_response_setup_ExpectAndReturn (name, true, RD_SUCCESS);
    err_code = rt_adv_connectability_set (true, name);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adv_connectability_set_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    const char name[] = "Ceedling";
    ri_adv_type_set_ExpectAndReturn (CONNECTABLE_SCANNABLE, RD_SUCCESS);
    rt_gatt_is_nus_enabled_ExpectAndReturn(false);
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
