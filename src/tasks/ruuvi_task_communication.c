#include "ruuvi_driver_enabled_modules.h"
#if RT_COMMUNICATION_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_task_communication.h"

#include <stdio.h>
#include <inttypes.h>

#define MAC_BYTES     (6U)  //!< Number of bytes in MAC address
#define ID_BYTES      (8U)  //!< Number of bytes in device ID
#define CHAR_PER_BYTE (3U)  //!< Char per byte in hexstr, including trailing NULL.
#define NULL_LEN      (1U) //!< Length of trailing NULL.
#define BITS_PER_BYTE (8U)  //!< Bits per byte.

/**
 * @brief convert U64 to hex string.
 *
 * Converts U64 into hex string, with option to leave most significant bytes out.
 * Adds ':' as delimiter.
 * e.g. u64_to_hexstr(0xAABBCCDDEEFF, str, 6) -> "AA:BB:CC:DD:EE:FF\0"
 *
 * @param[in] value Value to convert.
 * @param[out] str String to print to.
 * @param[in] Length of string to print to, at least 3 * bytes.
 * @param[in] bytes Number of bytes to write.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_LENGTH if string doesn't fit into given buffer.
 */
static rd_status_t u64_to_hexstr (const uint64_t value, char * const str,
                                  const size_t str_len, const uint8_t bytes)
{
    rd_status_t status = RD_SUCCESS;
    uint8_t bit_offset = BITS_PER_BYTE * (bytes - 1U);
    size_t written = 0;
    uint8_t num_delimiters = bytes - 1U;

    for (size_t ii = 0; (ii < bytes) && (str_len > (written + NULL_LEN)); ii++)
    {
        uint8_t byte = (uint8_t) (value >> bit_offset);
        written += snprintf (str + written, str_len - written, "%02X", byte);

        if ( (ii < num_delimiters) && (str_len > written))
        {
            written += snprintf (str + written, str_len - written, ":");
        }

        bit_offset -= BITS_PER_BYTE;
    }

    if ( (written + NULL_LEN) < CHAR_PER_BYTE * bytes
            || (str_len < written + NULL_LEN))
    {
        status |= RD_ERROR_INVALID_LENGTH;
    }

    return status;
}

rd_status_t rt_com_get_mac_str (char * const mac_str, const size_t mac_len)
{
    rd_status_t status = RD_SUCCESS;

    if (NULL == mac_str)
    {
        status = RD_ERROR_NULL;
    }
    else if (!ri_radio_is_init())
    {
        status |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        uint64_t mac;
        status |= ri_radio_address_get (&mac);
        status |= u64_to_hexstr (mac, mac_str, mac_len, MAC_BYTES);
    }

    return status;
}

rd_status_t rt_com_get_id_str (char * const id_str, const size_t id_len)
{
    rd_status_t status = RD_SUCCESS;

    if (NULL == id_str)
    {
        status = RD_ERROR_NULL;
    }
    else
    {
        uint64_t id;
        status |= ri_comm_id_get (&id);
        status |= u64_to_hexstr (id, id_str, id_len, ID_BYTES);
    }

    return status;
}

#endif
