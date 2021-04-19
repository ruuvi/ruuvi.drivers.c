/**
 * @addtogroup nfc_tasks
 */
/*@{*/
/**
 * @file ruuvi_task_nfc.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-21
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.

 * When NFC reader is in range return 4 UTF-textfields with content
 * @code
 * SW: version
 * MAC: AA:BB:CC:DD:EE:FF
 * ID: 00:11:22:33:44:55:66:77
 * Data:
 * @endcode
 */
#include "ruuvi_driver_enabled_modules.h"
#if RT_NFC_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_atomic.h"
#include "ruuvi_interface_communication_nfc.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_scheduler.h"
#include "ruuvi_interface_watchdog.h"
#include "ruuvi_task_nfc.h"
#include <stdio.h>
#include <string.h>

static ri_comm_channel_t m_channel;   //!< Handle for NFC comms.
static ri_comm_cb_t m_on_connected;    //!< Callback for connection established.
static ri_comm_cb_t m_on_disconnected; //!< Callback for connection lost.
static ri_comm_cb_t m_on_received;     //!< Callback for data received.
static ri_comm_cb_t m_on_sent;         //!< Callback for data sent.
static bool m_nfc_is_connected;        //!< True while NFC reader is present.
static bool m_nfc_is_initialized;      //!< True while NFC is initialized.

/**
 * @brief Event handler for NFC events
 *
 * This function is called in interrupt context, which allows for real-time processing
 * such as feeding softdevice data buffers during connection event.
 * Care must be taken to not call any function which requires external peripherals,
 * such as sensors in this context.
 *
 * If sensors must be read / configured as a response to NFC event, schedule
 * the action and send the results back during next connection event by buffering
 * the response with rt_nfc_send.
 *
 * @param evt Event type
 * @param p_data pointer to event data, if event is
 *               @c RI_COMMUNICATION_RECEIVED received data, NULL otherwise.
 * @param data_len number of bytes in received data, 0 if p_data is NULL.
 *
 */
#ifndef CEEDLING
static
#endif
rd_status_t rt_nfc_isr (ri_comm_evt_t evt, void * p_data, size_t data_len)
{
    switch (evt)
    {
        // Note: This gets called only after the NFC notifications have been registered.
        case RI_COMM_CONNECTED:
            m_nfc_is_connected = true;
            (NULL != m_on_connected) ? m_on_connected (p_data, data_len) : false;
            break;

        case RI_COMM_DISCONNECTED:
            m_nfc_is_connected = false;
            (NULL != m_on_disconnected) ? m_on_disconnected (p_data, data_len) : false;
            break;

        case RI_COMM_SENT:
            (NULL != m_on_sent) ? m_on_sent (p_data, data_len) : false;
            break;

        case RI_COMM_RECEIVED:
            (NULL != m_on_received) ? m_on_received (p_data, data_len) : false;
            break;

        default:
            break;
    }

    return RD_SUCCESS;
}

#ifndef CEEDLING
static
#endif
rd_status_t sw_set (const char * const sw)
{
    rd_status_t err_code = RD_SUCCESS;
    int written = 0;

    if (NULL == sw)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        uint8_t fw_string[RI_COMM_DIS_STRLEN] = { 0 };
        written = snprintf ( (char *) fw_string,
                             RI_COMM_DIS_STRLEN,
                             "SW: %s",
                             sw);

        if ( (0 > written)
                || (RI_COMM_DIS_STRLEN <= written))
        {
            err_code |= RD_ERROR_INVALID_LENGTH;
        }
        else
        {
            err_code |= ri_nfc_fw_version_set (fw_string,
                                               strlen ( (char *) fw_string));
        }
    }

    return err_code;
}

#ifndef CEEDLING
static
#endif
rd_status_t mac_set (const char * const mac)
{
    rd_status_t err_code = RD_SUCCESS;
    int written = 0;

    if (NULL == mac)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        uint8_t name[RI_COMM_DIS_STRLEN] = { 0 };
        written = snprintf ( (char *) name,
                             RI_COMM_DIS_STRLEN,
                             "MAC: %s",
                             mac);

        if ( (0 > written)
                || (RI_COMM_DIS_STRLEN <= written))
        {
            err_code |= RD_ERROR_INVALID_LENGTH;
        }
        else
        {
            err_code |= ri_nfc_address_set (name, strlen ( (char *) name));
        }
    }

    return err_code;
}

#ifndef CEEDLING
static
#endif
rd_status_t id_set (const char * const id)
{
    rd_status_t err_code = RD_SUCCESS;
    int written = 0;

    if (NULL == id)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        uint8_t id_string[RI_COMM_DIS_STRLEN] = { 0 };
        written = snprintf ( (char *) id_string,
                             RI_COMM_DIS_STRLEN,
                             "ID: %s",
                             id);

        if ( (0 > written)
                || (RI_COMM_DIS_STRLEN <= written))
        {
            err_code |= RD_ERROR_INVALID_LENGTH;
        }
        else
        {
            err_code |= ri_nfc_id_set (id_string,
                                       strlen ( (char *) id_string));
        }
    }

    return err_code;
}

rd_status_t rt_nfc_init (ri_comm_dis_init_t * const init_data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == init_data)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (rt_nfc_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        err_code |= sw_set (init_data->fw_version);
        err_code |= mac_set (init_data->deviceaddr);
        err_code |= id_set (init_data->deviceid);
        err_code |= ri_nfc_init (&m_channel);
        ri_comm_message_t msg;
        memcpy (&msg.data, "Data:", sizeof ("Data:"));
        msg.data_length = 6;
        m_channel.on_evt = rt_nfc_isr;
        err_code |= m_channel.send (&msg);
    }

    m_nfc_is_initialized = (RD_SUCCESS == err_code);
    return err_code;
}

bool rt_nfc_is_init (void)
{
    return m_nfc_is_initialized;
}

/**
 * @brief Uninitializes NFC.
 *
 * After uninitialization device is no longer readable with NFC reader.
 *
 * @return RD_SUCCESS on success.
 * @return error code from stack on error.
 */
rd_status_t rt_nfc_uninit (void)
{
    m_nfc_is_initialized = false;
    return ri_nfc_uninit (&m_channel);
}

rd_status_t rt_nfc_send (ri_comm_message_t * message)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == message)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        err_code |= m_channel.send (message);
    }

    return err_code;
}

void rt_nfc_set_on_connected_isr (const ri_comm_cb_t cb)
{
    m_on_connected = cb;
}


void rt_nfc_set_on_disconn_isr (const ri_comm_cb_t cb)
{
    m_on_disconnected = cb;
}

void rt_nfc_set_on_received_isr (const ri_comm_cb_t cb)
{
    m_on_received = cb;
}

void rt_nfc_set_on_sent_isr (const ri_comm_cb_t cb)
{
    m_on_sent = cb;
}

bool rt_nfc_is_connected (void)
{
    return m_nfc_is_connected;
}

#else
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include <stdbool.h>
rd_status_t rt_nfc_init (ri_comm_dis_init_t * const init_data)
{
    return RD_ERROR_NOT_ENABLED;
}

rd_status_t rt_nfc_send (ri_comm_message_t * message)
{
    return RD_ERROR_NOT_ENABLED;
}

void rt_nfc_set_on_connected_isr (const ri_comm_cb_t cb)
{
    // No implementation needed
}


void rt_nfc_set_on_disconn_isr (const ri_comm_cb_t cb)
{
    // No implementation needed
}

void rt_nfc_set_on_received_isr (const ri_comm_cb_t cb)
{
    // No implementation needed
}

void rt_nfc_set_on_sent_isr (const ri_comm_cb_t cb)
{
    // No implementation needed
}

bool rt_nfc_is_connected (void)
{
    return false;
}
#endif

/*@}*/
