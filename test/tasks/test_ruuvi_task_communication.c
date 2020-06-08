#include "unity.h"

#include "ruuvi_task_communication.h"
#include "mock_ruuvi_interface_communication_radio.h"
#include "mock_ruuvi_interface_communication.h"

#include <string.h>

void setUp (void)
{
}

void tearDown (void)
{
}


/**
 * @brief Get MAC address of the device from radio driver and write it to given string.
 *
 * The MAC address (or Bluetooth Address) is presented as most significant byte first,
 * if your Bluetooth scanner shows "AA:BB:CC:DD:EE:FF" the mac_str will have
 * "AA:BB:CC:DD:EE:FF".
 *
 * @param[out] mac_str 18-character array.
 * @param[in]  mac_len Length of mac_str, must be at least 18.
 * @retval RD_SUCCESS if mac_str was written.
 * @retval RD_ERROR_NULL if mac_str was NULL.
 * @retval RD_ERROR_INVALID_STATE if radio is not initialized.
 */
void test_rt_com_get_mac_str_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_radio_is_init_ExpectAndReturn (true);
    uint64_t mac = 0xAABBCCDDEEFF;
    char mac_str[18];
    ri_radio_address_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_radio_address_get_ReturnArrayThruPtr_address (&mac, 1);
    err_code = rt_com_get_mac_str (mac_str, sizeof (mac_str));
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (!strcmp (mac_str, "AA:BB:CC:DD:EE:FF"));
}

void test_rt_com_get_mac_str_radio_not_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_radio_is_init_ExpectAndReturn (false);
    uint64_t mac = 0xAABBCCDDEEFF;
    char mac_str[18];
    err_code = rt_com_get_mac_str (mac_str, sizeof (mac_str));
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}


void test_rt_com_get_mac_str_too_short (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_radio_is_init_ExpectAndReturn (true);
    uint64_t mac = 0xAABBCCDDEEFF;
    char mac_str[17];
    ri_radio_address_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_radio_address_get_ReturnArrayThruPtr_address (&mac, 1);
    err_code = rt_com_get_mac_str (mac_str, sizeof (mac_str));
    TEST_ASSERT (RD_ERROR_INVALID_LENGTH == err_code);
}

void test_rt_com_get_mac_str_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_com_get_mac_str (NULL, 18);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

/**
 * @brief Get Unique ID of the device and write it to given string.
 *
 * The ID will remain constant even if MAC is changed. The ID must remain same across
 * reboots and firmware updates.
 *
 * @param[out] id_str 24-character array
 * @param[in]  id_len Length of id_str, must be at least 24.
 * @retval RD_SUCCESS if id buffer was written
 * @retval RD_ERROR_NULL if id_buffer was NULL
 */
void test_rt_com_get_id_str_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint64_t id = 0xAABBCCDDEEFF0011;
    char id_str[24];
    ri_comm_id_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_comm_id_get_ReturnArrayThruPtr_id (&id, 1);
    err_code = rt_com_get_id_str (id_str, sizeof (id_str));
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (!strcmp (id_str, "AA:BB:CC:DD:EE:FF:00:11"));
}

void test_rt_com_get_id_radio_fail (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint64_t id = 0xAABBCCDDEEFF0011;
    char id_str[24];
    ri_comm_id_get_ExpectAnyArgsAndReturn (RD_ERROR_INVALID_STATE);
    ri_comm_id_get_ReturnArrayThruPtr_id (&id, 1);
    err_code = rt_com_get_id_str (id_str, sizeof (id_str));
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_com_get_id_str_too_short (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint64_t id = 0xAABBCCDDEEFF0011;
    char id_str[23];
    ri_comm_id_get_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_comm_id_get_ReturnArrayThruPtr_id (&id, 1);
    err_code = rt_com_get_id_str (id_str, sizeof (id_str));
    TEST_ASSERT (RD_ERROR_INVALID_LENGTH == err_code);
}

void test_rt_com_get_id_str_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_com_get_id_str (NULL, 18);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}
