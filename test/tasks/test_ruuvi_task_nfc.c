#include "unity.h"

#include "ruuvi_driver_enabled_modules.h"

#include "ruuvi_driver_error.h"
#include "ruuvi_task_nfc.h"
#include "ruuvi_interface_communication.h"

#include "mock_ruuvi_interface_communication_nfc.h"
#include "mock_ruuvi_interface_log.h"

#include <string.h>

static uint32_t send_count = 0;
static uint32_t read_count = 0;
static bool m_con_cb;
static bool m_discon_cb;
static bool m_tx_cb;
static bool m_rx_cb;
static const char m_name[] = "Ceedling";

static ri_comm_dis_init_t m_dis;
static ri_comm_channel_t m_mock_nfc;

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
    p_channel->on_evt = rt_nfc_isr;
    return RD_SUCCESS;
}

static ri_comm_channel_t m_mock_nfc;

void setUp (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_log_Ignore();
    ri_log_hex_Ignore();
    ri_error_to_string_IgnoreAndReturn (RD_SUCCESS);
    mock_init (&m_mock_nfc);
    strcpy (m_dis.fw_version, "CeedlingFW");
    strcpy (m_dis.model, "Ceedlingmodel");
    strcpy (m_dis.hw_version, "Ceedlinghw_version");
    strcpy (m_dis.manufacturer, "Ceedlingmanufacturer");
    strcpy (m_dis.deviceid, "Ceedlingdeviceid");
    strcpy (m_dis.deviceaddr, "Ceedlingdeviceaddr");
}

void tearDown (void)
{
    memset (&m_mock_nfc, 0, sizeof (ri_comm_channel_t));
    send_count = 0;
    read_count = 0;
    m_con_cb = false;
    m_discon_cb = false;
    m_tx_cb = false;
    m_rx_cb = false;
    memset (&m_dis, 0, sizeof (m_dis));
    ri_nfc_uninit_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rt_nfc_uninit();
    rt_nfc_set_on_received_isr (NULL);
    rt_nfc_set_on_sent_isr (NULL);
    rt_nfc_set_on_connected_isr (NULL);
    rt_nfc_set_on_disconn_isr (NULL);
    TEST_ASSERT (!rt_nfc_is_init());
}

/**
 * @brief Initialize NFC. Must be called as a first function in rt_nfc.
 *
 * After calling this function underlying software stack is ready to setup NFC services.
 *
 * @param[in] Full name of device to be advertised in scan responses. Maximum 11 chars + trailing NULL. Must not be NULL, 0-length string is valid.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL if name is NULL (use 0-length string instead)
 * @retval RD_ERROR_INVALID_LENGTH if name is longer than @ref SCAN_RSP_NAME_MAX_LEN
 * @retval RD_ERROR_INVALID_STATE if NFC is already initialized or advertisements are not initialized.
 *
 */
void test_rt_nfc_init_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_nfc_fw_version_set_IgnoreAndReturn (RD_SUCCESS);
    ri_nfc_address_set_IgnoreAndReturn (RD_SUCCESS);
    ri_nfc_id_set_IgnoreAndReturn (RD_SUCCESS);
    ri_nfc_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_nfc_init_ReturnArrayThruPtr_channel (&m_mock_nfc, 1);
    err_code |= rt_nfc_init (&m_dis);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (rt_nfc_is_init());
}

void test_rt_nfc_init_twice (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_nfc_fw_version_set_IgnoreAndReturn (RD_SUCCESS);
    ri_nfc_address_set_IgnoreAndReturn (RD_SUCCESS);
    ri_nfc_id_set_IgnoreAndReturn (RD_SUCCESS);
    ri_nfc_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_nfc_init_ReturnArrayThruPtr_channel (&m_mock_nfc, 1);
    err_code |= rt_nfc_init (&m_dis);
    err_code |= rt_nfc_init (&m_dis);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_nfc_init_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= rt_nfc_init (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_sw_set_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_nfc_fw_version_set_ExpectAndReturn ( (uint8_t *) "SW: CeedlingFW",
                                            strlen ("SW: CeedlingFW"), RD_SUCCESS);
    err_code |= sw_set ("CeedlingFW");
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_sw_set_max_len (void)
{
    rd_status_t err_code = RD_SUCCESS;
    char version_string[RI_COMM_DIS_STRLEN] = { 0 };
    char expected_string[RI_COMM_DIS_STRLEN] = "SW: ";

    for (size_t ii = 0; ii < RI_COMM_DIS_STRLEN - sizeof ("SW: "); ii++)
    {
        version_string[ii] = 'A';
        expected_string[ii + strlen ("SW: ")] = 'A';
    }

    ri_nfc_fw_version_set_ExpectAndReturn ( (uint8_t *) expected_string,
                                            strlen (expected_string), RD_SUCCESS);
    err_code |= sw_set (version_string);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_sw_set_too_long (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t version_string[RI_COMM_DIS_STRLEN] = {0};

    for (size_t ii = 0; ii < RI_COMM_DIS_STRLEN; ii++)
    {
        version_string[ii] = 'A';
    }

    err_code |= sw_set ( (char *) version_string);
    TEST_ASSERT (RD_ERROR_INVALID_LENGTH == err_code);
}

void test_sw_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= sw_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_mac_set_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_nfc_address_set_ExpectAndReturn ( (uint8_t *) "MAC: AA:BB:CC:DD:EE:FF",
                                         strlen ("MAC: AA:BB:CC:DD:EE:FF"), RD_SUCCESS);
    err_code |= mac_set ("AA:BB:CC:DD:EE:FF");
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_mac_set_max_len (void)
{
    rd_status_t err_code = RD_SUCCESS;
    char version_string[RI_COMM_DIS_STRLEN] = {0};
    char expected_string[RI_COMM_DIS_STRLEN] = "MAC: ";

    for (size_t ii = 0; ii < RI_COMM_DIS_STRLEN - sizeof ("MAC: "); ii++)
    {
        version_string[ii] = 'A';
        expected_string[ii + strlen ("MAC: ")] = 'A';
    }

    ri_nfc_address_set_ExpectAndReturn ( (uint8_t *) expected_string,
                                         strlen (expected_string), RD_SUCCESS);
    err_code |= mac_set (version_string);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_mac_set_too_long (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t version_string[RI_COMM_DIS_STRLEN] = {0};

    for (size_t ii = 0; ii < RI_COMM_DIS_STRLEN; ii++)
    {
        version_string[ii] = 'A';
    }

    err_code |= mac_set ( (char *) version_string);
    TEST_ASSERT (RD_ERROR_INVALID_LENGTH == err_code);
}

void test_mac_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= mac_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_id_set_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_nfc_id_set_ExpectAndReturn ( (uint8_t *) "ID: AA:BB:CC:DD:EE:FF:11:22",
                                    strlen ("ID: AA:BB:CC:DD:EE:FF:11:22"), RD_SUCCESS);
    err_code |= id_set ("AA:BB:CC:DD:EE:FF:11:22");
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_id_set_max_len (void)
{
    rd_status_t err_code = RD_SUCCESS;
    char version_string[RI_COMM_DIS_STRLEN] = { 0 };;
    char expected_string[RI_COMM_DIS_STRLEN] = "ID: ";

    for (size_t ii = 0; ii < RI_COMM_DIS_STRLEN - sizeof ("ID: "); ii++)
    {
        version_string[ii] = 'A';
        expected_string[ii + strlen ("ID: ")] = 'A';
    }

    ri_nfc_id_set_ExpectAndReturn ( (uint8_t *) expected_string,
                                    strlen (expected_string), RD_SUCCESS);
    err_code |= id_set (version_string);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_id_set_too_long (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t version_string[RI_COMM_DIS_STRLEN] = { 0 };;

    for (size_t ii = 0; ii < RI_COMM_DIS_STRLEN; ii++)
    {
        version_string[ii] = 'A';
    }

    err_code |= id_set ( (char *) version_string);
    TEST_ASSERT (RD_ERROR_INVALID_LENGTH == err_code);
}

void test_id_set_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= id_set (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

/**
 * @brief Send given message via NFC
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
void test_rt_nfc_send_asynchronous_ok()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg = { 0 };
    msg.data_length = 11;
    test_rt_nfc_init_ok();
    err_code = rt_nfc_send (&msg);
    // init sends once.
    TEST_ASSERT (2 == send_count);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_nfc_send_asynchronous_null()
{
    rd_status_t err_code = RD_SUCCESS;
    ri_comm_message_t msg = { 0 };
    msg.data_length = 11;
    test_rt_nfc_init_ok();
    err_code = rt_nfc_send (NULL);
    // init sends once.
    TEST_ASSERT (1 == send_count);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_nfc_callbacks_ok()
{
    test_rt_nfc_init_ok();
    rt_nfc_set_on_received_isr (on_rx_isr);
    rt_nfc_set_on_sent_isr (on_tx_isr);
    rt_nfc_set_on_connected_isr (on_con_isr);
    rt_nfc_set_on_disconn_isr (on_discon_isr);
    rt_nfc_isr (RI_COMM_CONNECTED, NULL, 0);
    rt_nfc_isr (RI_COMM_DISCONNECTED, NULL, 0);
    rt_nfc_isr (RI_COMM_SENT, NULL, 0);
    rt_nfc_isr (RI_COMM_RECEIVED, NULL, 0);
    TEST_ASSERT (m_rx_cb);
    TEST_ASSERT (m_tx_cb);
    TEST_ASSERT (m_con_cb);
    TEST_ASSERT (m_discon_cb);
}

void test_rt_nfc_callbacks_null()
{
    test_rt_nfc_init_ok();
    rt_nfc_isr (RI_COMM_CONNECTED, NULL, 0);
    rt_nfc_isr (RI_COMM_DISCONNECTED, NULL, 0);
    rt_nfc_isr (RI_COMM_SENT, NULL, 0);
    rt_nfc_isr (RI_COMM_RECEIVED, NULL, 0);
    TEST_ASSERT_FALSE (m_rx_cb);
    TEST_ASSERT_FALSE (m_tx_cb);
    TEST_ASSERT_FALSE (m_con_cb);
    TEST_ASSERT_FALSE (m_discon_cb);
}
