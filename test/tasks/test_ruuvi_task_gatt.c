#include "unity.h"

#include "ruuvi_driver_enabled_modules.h"

#include "ruuvi_driver_error.h"
#include "ruuvi_task_gatt.h"
#include "ruuvi_interface_communication.h"

#include "mock_ruuvi_task_advertisement.h"
#include "mock_ruuvi_interface_atomic.h"
#include "mock_ruuvi_interface_communication_ble_advertising.h"
#include "mock_ruuvi_interface_communication_ble_gatt.h"
#include "mock_ruuvi_interface_communication_radio.h"
#include "mock_ruuvi_interface_log.h"

#include <string.h>

static uint32_t send_count = 0;
static uint32_t read_count = 0;
static bool m_con_cb;
static bool m_discon_cb;
static bool m_tx_cb;
static bool m_rx_cb;
static const char m_name[] = "Ceedling";

#define SEND_COUNT_MAX (10U)

rd_status_t mock_send (ri_comm_message_t * const p_msg)
{
    rd_status_t err_code = RD_SUCCESS;
    static bool extra_error = false;

    if (send_count < SEND_COUNT_MAX)
    {
        send_count++;
    }
    else
    {
        if (!extra_error)
        {
            err_code |= RD_ERROR_RESOURCES;
            extra_error = true;
        }
        else
        {
            err_code |= RD_ERROR_INTERNAL;
        }
    }

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

void on_con_isr (void * p_data, size_t data_len)
{
    m_con_cb = true;
}

void on_discon_isr (void * p_data, size_t data_len)
{
    m_discon_cb = true;
}

void on_rx_isr (void * p_data, size_t data_len)
{
    m_rx_cb = true;
}

void on_tx_isr (void * p_data, size_t data_len)
{
    m_tx_cb = true;
}

rd_status_t mock_init (ri_comm_channel_t * const p_channel)
{
    p_channel->send   = mock_send;
    p_channel->read   = mock_read;
    p_channel->uninit = mock_uninit;
    p_channel->init   = mock_init;
    p_channel->on_evt = rt_gatt_on_nus_isr;
    return RD_SUCCESS;
}

static ri_comm_channel_t m_mock_gatt;

void setUp (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_log_Ignore();
    ri_log_hex_Ignore();
    ri_error_to_string_IgnoreAndReturn (RD_SUCCESS);
    rt_adv_is_init_ExpectAndReturn (true);
    ri_gatt_init_ExpectAndReturn (RD_SUCCESS);
    err_code |= rt_gatt_init (m_name);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (rt_gatt_is_init());
}

void tearDown (void)
{
    memset (&m_mock_gatt, 0, sizeof (ri_comm_channel_t));
    send_count = 0;
    read_count = 0;
    m_con_cb = false;
    m_discon_cb = false;
    m_tx_cb = false;
    m_rx_cb = false;
    rt_gatt_mock_state_reset();
    TEST_ASSERT (!rt_gatt_is_init());
}





/**
 * @brief Initialize Device Firmware Update service
 *
 * GATT must be initialized before calling this function, and once initialized the DFU
 * service cannot be uninitialized.
 *
 * Call will return successfully even if the device doesn't have useable bootloader, however
 * program will reboot if user tries to enter bootloader in that case.
 *
 * To use the DFU service advertisement module must send connectable (and preferably scannable) advertisements.
 *
 * @retval RD_SUCCESS GATT was initialized successfully
 * @retval RD_ERROR_INVALID_STATE DFU was already initialized or GATT is not initialized
 */
void test_rt_gatt_dfu_init_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_gatt_dfu_init_ExpectAndReturn (RD_SUCCESS);
    err_code = rt_gatt_dfu_init();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_gatt_dfu_init_no_gatt (void)
{
    rd_status_t err_code = RD_SUCCESS;
    tearDown();
    err_code = rt_gatt_dfu_init();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_gatt_dfu_init_twice (void)
{
    rd_status_t err_code = RD_SUCCESS;
    test_rt_gatt_dfu_init_ok();
    err_code = rt_gatt_dfu_init();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Initialize Device Information Update service
 *
 * GATT must be initialized before calling this function, and once initialized the DIS
 * service cannot be uninitialized.
 *
 * DIS service lets user read basic information, such as firmware version and hardware model over GATT in a standard format.
 *
 * To use the DIS service advertisement module must send connectable (and preferably scannable) advertisements.
 *
 * @param[in] dis structure containing data to be copied into DIS, can be freed after call finishes.
 *
 * @retval RD_SUCCESS GATT was initialized successfully
 * @retval RD_ERROR_NULL if given NULL as the information.
 * @retval RD_ERROR_INVALID_STATE DIS was already initialized or GATT is not initialized
 */
void test_rt_gatt_dis_init_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_dis_init_t dis = {0};
    ri_gatt_dis_init_ExpectAndReturn (&dis, RD_SUCCESS);
    err_code = rt_gatt_dis_init (&dis);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_gatt_dis_init_twice (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_dis_init_t dis = {0};
    test_rt_gatt_dis_init_ok();
    err_code = rt_gatt_dis_init (&dis);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_gatt_dis_init_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_gatt_dis_init (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_gatt_dis_init_no_gatt (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_dis_init_t dis = {0};
    err_code = rt_gatt_dis_init (&dis);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Initialize Nordic UART Service
 *
 * GATT must be initialized before calling this function, and once initialized the NUS
 * service cannot be uninitialized.
 *
 * NUS service lets user do bidirectional communication with the application.
 *
 * To use the NUS service advertisement module must send connectable (and preferably scannable) advertisements.
 *
 *
 * @retval RD_SUCCESS GATT was initialized successfully
 * @retval RD_ERROR_NULL if given NULL as the information.
 * @retval RD_ERROR_INVALID_STATE DIS was already initialized or GATT is not initialized
 *
 * @note To actually use the data in application, user must setup at least data received callback with @ref rt_gatt_set_on_received_isr
 */
void test_rt_gatt_nus_init_ok()
{
    rd_status_t err_code = RD_SUCCESS;
    mock_init (&m_mock_gatt);
    ri_gatt_nus_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_gatt_nus_init_ReturnArrayThruPtr_channel (&m_mock_gatt, 1);
    err_code = rt_gatt_nus_init ();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

/**
 * @brief Initialize GATT. Must be called as a first function in rt_gatt.
 *
 * After calling this function underlying software stack is ready to setup GATT services.
 *
 * @param[in] Full name of device to be advertised in scan responses. Maximum 11 chars + trailing NULL. Must not be NULL, 0-length string is valid.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL if name is NULL (use 0-length string instead)
 * @retval RD_ERROR_INVALID_LENGTH if name is longer than @ref SCAN_RSP_NAME_MAX_LEN
 * @retval RD_ERROR_INVALID_STATE if GATT is already initialized or advertisements are not initialized.
 *
 */
void test_rt_gatt_init_ok (void)
{
    // no implementation needed, done ins setup/teardown
}

void test_rt_gatt_init_twice (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rt_adv_is_init_ExpectAndReturn (true);
    err_code |= rt_gatt_init (m_name);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_gatt_init_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= rt_gatt_init (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_gatt_init_too_long_name (void)
{
    rd_status_t err_code = RD_SUCCESS;
    tearDown();
    // Scan response has space for extra null already, so add + 2
    char toolong[SCAN_RSP_NAME_MAX_LEN + 2] = {0};

    for (size_t ii = 0; ii < sizeof (toolong); ii++)
    {
        toolong[ii] = 'A';
    }

    rt_adv_is_init_ExpectAndReturn (true);
    err_code |= rt_gatt_init (toolong);
    TEST_ASSERT (RD_ERROR_INVALID_LENGTH == err_code);
    TEST_ASSERT (!rt_gatt_is_init());
}

void test_rt_gatt_init_max_len_name (void)
{
    rd_status_t err_code = RD_SUCCESS;
    tearDown();
    // Scan response has space for extra null already, so add + 2
    char maxlen[SCAN_RSP_NAME_MAX_LEN + 1] = {0};

    for (size_t ii = 0; ii < sizeof (maxlen); ii++)
    {
        maxlen[ii] = 'A';
    }

    maxlen[sizeof (maxlen) - 1] = '\0';
    rt_adv_is_init_ExpectAndReturn (true);
    ri_gatt_init_ExpectAndReturn (RD_SUCCESS);
    err_code |= rt_gatt_init (maxlen);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (rt_gatt_is_init());
}

/**
 * @brief Start advertising GATT connection to devices.
 *
 * Calling this function is not enough to let users to connect, you must also update advertised data
 * to add the scan response to data being advertised. This makes sure that advertised data stays valid.
 * This function has no effect if called while already enabled.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if GATT is not initialized.
 */
void test_rt_gatt_adv_enable_ok_no_nus()
{
    rd_status_t err_code = RD_SUCCESS;
    rt_adv_connectability_set_ExpectAndReturn (true, m_name, RD_SUCCESS);
    err_code |= rt_gatt_adv_enable ();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_gatt_adv_enable_ok_with_nus()
{
    rd_status_t err_code = RD_SUCCESS;
    test_rt_gatt_nus_init_ok();
    rt_adv_connectability_set_ExpectAndReturn (true, m_name, RD_SUCCESS);
    err_code |= rt_gatt_adv_enable ();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_gatt_adv_enable_gatt_not_init()
{
    rd_status_t err_code = RD_SUCCESS;
    tearDown();
    err_code |= rt_gatt_adv_enable ();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Stop advertising GATT connection to devices.
 *
 * Calling this function is not enough to stop advertising connection, you must also update advertised data
 * to remove the scan response from data being advertised. This makes sure that advertised data stays valid.
 * This function has not effect if called while already disabled
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if GATT is not initialized.
 */
void test_rt_gatt_adv_disable_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rt_adv_connectability_set_ExpectAndReturn (false, NULL, RD_SUCCESS);
    err_code |= rt_gatt_adv_disable();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_gatt_adv_disable_not_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    tearDown();
    err_code |= rt_gatt_adv_disable();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Send given message via NUS
 *
 * This function queues a message to be sent and returns immediately.
 * There is no guarantee on when the data is actually sent, and
 * there is no acknowledgement or callback after the data has been sent.
 *
 * @return RD_SUCCESS if data was placed in send buffer.
 * @retval RD_ERROR_NULL if NULL was tried to send.
 * @return RD_ERROR_INVALID_STATE if NUS is not connected.
 * @retval RD_ERROR_NO_MEM if transmit buffer is full.
 * @return error code from stack on error
 *
 */
void test_rt_gatt_send_asynchronous_ok()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg = { 0 };
    msg.data_length = 11;
    test_rt_gatt_nus_init_ok();
    rt_gatt_on_nus_isr (RI_COMM_CONNECTED, NULL, 0);
    err_code = rt_gatt_send_asynchronous (&msg);
    TEST_ASSERT (1 == send_count);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_gatt_send_asynchronous_null()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg = { 0 };
    msg.data_length = 11;
    test_rt_gatt_nus_init_ok();
    rt_gatt_on_nus_isr (RI_COMM_CONNECTED,
                        NULL, 0);
    err_code = rt_gatt_send_asynchronous (NULL);
    TEST_ASSERT (0 == send_count);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_gatt_send_asynchronous_no_nus()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg = { 0 };
    msg.data_length = 11;
    test_rt_gatt_nus_init_ok();
    err_code = rt_gatt_send_asynchronous (&msg);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
    TEST_ASSERT (0 == send_count);
}

void test_rt_gatt_send_asynchronous_no_mem()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg = { 0 };
    msg.data_length = 11;
    test_rt_gatt_nus_init_ok();
    rt_gatt_on_nus_isr (RI_COMM_CONNECTED,
                        NULL, 0);

    for (uint32_t ii = 0; ii <= SEND_COUNT_MAX; ii++)
    {
        err_code |= rt_gatt_send_asynchronous (&msg);
    }

    TEST_ASSERT (SEND_COUNT_MAX == send_count);
    TEST_ASSERT (RD_ERROR_NO_MEM == err_code);
}

void test_rt_gatt_send_asynchronous_unknown_error()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg = { 0 };
    msg.data_length = 11;
    test_rt_gatt_nus_init_ok();
    rt_gatt_on_nus_isr (RI_COMM_CONNECTED,
                        NULL, 0);

    for (uint32_t ii = 0; ii < SEND_COUNT_MAX; ii++)
    {
        err_code = rt_gatt_send_asynchronous (&msg);
    }

    err_code = rt_gatt_send_asynchronous (&msg);
    TEST_ASSERT (RD_ERROR_INTERNAL == err_code);
}

void test_rt_gatt_callbacks_ok()
{
    test_rt_gatt_nus_init_ok();
    rt_gatt_set_on_received_isr (on_rx_isr);
    rt_gatt_set_on_sent_isr (on_tx_isr);
    rt_gatt_set_on_connected_isr (on_con_isr);
    rt_gatt_set_on_disconn_isr (on_discon_isr);
    rt_gatt_on_nus_isr (RI_COMM_CONNECTED,
                        NULL, 0);
    rt_gatt_on_nus_isr (RI_COMM_DISCONNECTED,
                        NULL, 0);
    rt_gatt_on_nus_isr (RI_COMM_SENT,
                        NULL, 0);
    rt_gatt_on_nus_isr (RI_COMM_RECEIVED,
                        NULL, 0);
    TEST_ASSERT (m_rx_cb);
    TEST_ASSERT (m_tx_cb);
    TEST_ASSERT (m_con_cb);
    TEST_ASSERT (m_discon_cb);
}

void test_rt_gatt_callbacks_null()
{
    test_rt_gatt_nus_init_ok();
    rt_gatt_on_nus_isr (RI_COMM_CONNECTED,
                        NULL, 0);
    rt_gatt_on_nus_isr (RI_COMM_DISCONNECTED,
                        NULL, 0);
    rt_gatt_on_nus_isr (RI_COMM_SENT,
                        NULL, 0);
    rt_gatt_on_nus_isr (RI_COMM_RECEIVED,
                        NULL, 0);
    TEST_ASSERT_FALSE (m_rx_cb);
    TEST_ASSERT_FALSE (m_tx_cb);
    TEST_ASSERT_FALSE (m_con_cb);
    TEST_ASSERT_FALSE (m_discon_cb);
}

/**
 * @brief Uninitialize GATT.
 *
 * After calling this function callbacks, characteristics and services are cleared.
 *
 * @note Nordic SDK requires radio uninitialization to reset GATT service states.
 *       If any other task is using radio, this function will return error.
 *       This function will re-initialize radio after GATT is uninitialized with original
 *       modulation.
 *
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if GATT cannot be uninitialized.
 *
 */
void test_rt_gatt_uninit_ok (void)
{
    ri_radio_modulation_t modulation = RI_RADIO_BLE_2MBPS;
    test_rt_gatt_dis_init_ok();
    test_rt_gatt_adv_enable_ok_with_nus();
    test_rt_gatt_dfu_init_ok();
    rt_adv_is_init_ExpectAndReturn (false);
    ri_radio_get_modulation_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_radio_get_modulation_ReturnThruPtr_p_modulation (&modulation);
    ri_radio_uninit_ExpectAndReturn (RD_SUCCESS);
    ri_gatt_uninit_ExpectAndReturn (RD_SUCCESS);
    ri_radio_init_ExpectAndReturn (modulation, RD_SUCCESS);
    rd_status_t err_code = rt_gatt_uninit();
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (!rt_gatt_is_init());
    setUp();
    test_rt_gatt_dis_init_ok();
    test_rt_gatt_adv_enable_ok_with_nus();
    test_rt_gatt_dfu_init_ok();
}

void test_rt_gatt_uninit_adv_ongoing (void)
{
    ri_radio_modulation_t modulation = RI_RADIO_BLE_1MBPS;
    rt_adv_is_init_ExpectAndReturn (true);
    rd_status_t err_code = rt_gatt_uninit();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}