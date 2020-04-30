#include "ruuvi_driver_enabled_modules.h"
#if RT_COMMUNICATION_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_task_communication.h"

#include <stdio.h>
#include <inttypes.h>

#define MAC_BYTES (6U) //!< Number of bytes in MAC address
#define ID_BYTES (8U)  //!< Number of bytes in device ID

rd_status_t rt_com_get_mac_str (char * const mac_str, const size_t mac_len)
{
    rd_status_t status = RD_SUCCESS;

    if(NULL == mac_str)
    {
        status = RD_ERROR_NULL;
    }
    else if(!ri_radio_is_init())
    {
        status |= RD_ERROR_INVALID_STATE;
    }
    else if(RT_COMM_MAC_STRLEN > mac_len)
    {
        status = RD_ERROR_INVALID_LENGTH;
    }
    else
    {
        uint64_t mac;
        uint8_t mac_buffer[MAC_BYTES];
        status |= ri_radio_address_get (&mac);
        mac_buffer[0U] = (mac >> 40U) & 0xFFU;
        mac_buffer[1U] = (mac >> 32U) & 0xFFU;
        mac_buffer[2U] = (mac >> 24U) & 0xFFU;
        mac_buffer[3U] = (mac >> 16U) & 0xFFU;
        mac_buffer[4U] = (mac >> 8U) & 0xFFU;
        mac_buffer[5U] = (mac >> 0U) & 0xFFU;
        snprintf(mac_str, mac_len, "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac_buffer[0], mac_buffer[1], mac_buffer[2],
                 mac_buffer[3], mac_buffer[4], mac_buffer[5]);
    }

    return status;
}

rd_status_t rt_com_get_id_str (char * const id_str, const size_t id_len)
{
    rd_status_t status = RD_SUCCESS;

    if(NULL == id_str)
    {
        status = RD_ERROR_NULL;
    }
    else if(RT_COMM_ID_STRLEN > id_len)
    {
        status = RD_ERROR_INVALID_LENGTH;
    }
    else
    {
        uint64_t id;
        uint8_t id_buffer[ID_BYTES];
        status |= ri_comm_id_get (&id);
        id_buffer[0U] = (id >> 56U) & 0xFFU;
        id_buffer[1U] = (id >> 48U) & 0xFFU;
        id_buffer[2U] = (id >> 40U) & 0xFFU;
        id_buffer[3U] = (id >> 32U) & 0xFFU;
        id_buffer[4U] = (id >> 24U) & 0xFFU;
        id_buffer[5U] = (id >> 16U) & 0xFFU;
        id_buffer[6U] = (id >> 8U) & 0xFFU;
        id_buffer[7U] = (id >> 0U) & 0xFFU;
        snprintf(id_str, id_len, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                 id_buffer[0], id_buffer[1], id_buffer[2], id_buffer[3],
                 id_buffer[4], id_buffer[5], id_buffer[6], id_buffer[7]);
    }

    return status;
}

#endif