/**
 * @addtogroup nfc_tasks
 */
/*@{*/
/**
 * @file task_nfc.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-12-24
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.

 * When NFC reader is in range return 4 UTF-textfields with content
 * @code
 * SW: version
 * MAC: AA:BB:CC:DD:EE:FF
 * ID: 00:11:22:33:44:55:66:77
 * DATA:
 * @endcode
 */
#include "ruuvi_driver_enabled_modules.h"
#if RT_NFC_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_nfc.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_interface_scheduler.h"
#include "ruuvi_interface_watchdog.h"
#include "ruuvi_task_nfc.h"
#include <stdio.h>
#include <string.h>

static ri_communication_t channel;

rd_status_t rt_nfc_init (ri_communication_dis_init_t * const init_data)
{
    rd_status_t err_code = RD_SUCCESS;
    int written = 0;
    uint8_t fw_prefix[] = {'S', 'W', ':', ' '};
    uint8_t version_string[RI_COMMUNICATION_DIS_STRLEN] = { 0 };
    memcpy (version_string, fw_prefix, sizeof (fw_prefix));
    written = snprintf ( (char *) (version_string + sizeof (fw_prefix)),
                         RI_COMMUNICATION_DIS_STRLEN - sizeof (fw_prefix),
                         "%s", init_data->fw_version);

    if (! (written > 0 && RI_COMMUNICATION_DIS_STRLEN > written))
    {
        err_code |= RD_ERROR_DATA_SIZE;
    }

    err_code |= ri_nfc_fw_version_set (version_string,
                                       strlen ( (char *) version_string));
    uint8_t name[RI_COMMUNICATION_DIS_STRLEN] = { 0 };
    written = snprintf ( (char *) name,
                         RI_COMMUNICATION_DIS_STRLEN,
                         "MAC: %s",
                         init_data->deviceaddr);

    if (! (written > 0 && RI_COMMUNICATION_DIS_STRLEN > written))
    {
        err_code |= RD_ERROR_DATA_SIZE;
    }

    err_code |= ri_nfc_address_set (name, strlen ( (char *) name));
    uint8_t prefix[] = {'I', 'D', ':', ' '};
    uint8_t id_string[RI_COMMUNICATION_DIS_STRLEN] = { 0 };
    memcpy (id_string, prefix, sizeof (prefix));
    written = snprintf ( (char *) (id_string + sizeof (prefix)),
                         RI_COMMUNICATION_DIS_STRLEN - sizeof (prefix),
                         "%s", init_data->deviceid);

    if (! (written > 0 && RI_COMMUNICATION_DIS_STRLEN > written))
    {
        err_code |= RD_ERROR_DATA_SIZE;
    }

    err_code |= ri_nfc_id_set (id_string,
                               strlen ( (char *) id_string));
    err_code |= ri_nfc_init (&channel);
    ri_communication_message_t msg;
    memcpy (&msg.data, "Data:", sizeof ("Data:"));
    msg.data_length = 6;
    err_code |= channel.send (&msg);
    channel.on_evt = rt_nfc_on_nfc;
    return err_code;
}

rd_status_t rt_nfc_send (ri_communication_message_t * message)
{
    return channel.send (message);
}

rd_status_t rt_nfc_on_nfc (ri_communication_evt_t evt,
                           void * p_data, size_t data_len)
{
    rd_status_t err_code = RD_SUCCESS;

    switch (evt)
    {
        case RI_COMMUNICATION_CONNECTED:
            ri_log (RI_LOG_LEVEL_INFO, "NFC connected \r\n");
            break;

        case RI_COMMUNICATION_DISCONNECTED:
            ri_log (RI_LOG_LEVEL_INFO, "NFC disconnected \r\n");
            break;

        case RI_COMMUNICATION_SENT:
            ri_log (RI_LOG_LEVEL_INFO, "NFC data sent\r\n");
            break;

        case RI_COMMUNICATION_RECEIVED:
            ri_log (RI_LOG_LEVEL_INFO, "NFC data received\r\n");
            break;

        default:
            break;
    }

    return err_code;
}
#else
#include "ruuvi_driver_error.h"
rd_status_t rt_nfc_init (void)
{
    return  RD_SUCCESS;
}
#endif

/*@}*/
