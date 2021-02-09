/**
 * Ruuvi Firmware 3.x GATT tasks.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_atomic.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#include "ruuvi_interface_communication_ble_gatt.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_scheduler.h"
#include "ruuvi_task_advertisement.h"
#include "ruuvi_task_communication.h"
#include "ruuvi_task_gatt.h"
#if RT_GATT_ENABLED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef TASK_GATT_LOG_LEVEL
#define TASK_GATT_LOG_LEVEL RI_LOG_LEVEL_INFO
#endif

static inline void LOGD (const char * const msg)
{
    ri_log (RI_LOG_LEVEL_DEBUG, msg);
}

static inline void LOGDHEX (const uint8_t * const msg, const size_t len)
{
    ri_log_hex (RI_LOG_LEVEL_DEBUG, msg, len);
}

static ri_comm_channel_t m_channel;   //!< API for sending data.
static bool m_is_init;
static bool m_nus_is_init;
static bool m_dis_is_init;
static bool m_dfu_is_init;
static bool m_nus_is_connected;
static char m_name[SCAN_RSP_NAME_MAX_LEN + 1] = { 0 };

static ri_comm_cb_t m_on_connected;    //!< Callback for connection established
static ri_comm_cb_t m_on_disconnected; //!< Callback for connection lost
static ri_comm_cb_t m_on_received;     //!< Callback for data received
static ri_comm_cb_t m_on_sent;         //!< Callback for data sent

// https://github.com/arm-embedded/gcc-arm-none-eabi.debian/blob/master/src/libiberty/strnlen.c
// Not included when compiled with std=c99.
static inline size_t safe_strlen (const char * s, size_t maxlen)
{
    size_t i;

    for (i = 0; (i < maxlen) && ('\0' != s[i]); ++i);

    return i;
}

#ifdef CEEDLING
void rt_gatt_mock_state_reset()
{
    m_on_connected = NULL;
    m_on_disconnected = NULL;
    m_on_received = NULL;
    m_on_sent = NULL;
    m_is_init = false;
    m_nus_is_init = false;
    m_dfu_is_init = false;
    m_dis_is_init = false;
    m_nus_is_connected = false;
    memset (&m_channel, 0, sizeof (ri_comm_channel_t));
    memset (m_name, 0, sizeof (m_name));
}
#endif

/**
 * @brief Event handler for NUS events
 *
 * This function is called in interrupt context, which allows for real-time processing
 * such as feeding softdevice data buffers during connection event.
 * Care must be taken to not call any function which requires external peripherals,
 * such as sensors in this context.
 *
 * If sensors must be read / configured as a response to GATT event, schedule
 * the action and send the results back during next connection event by buffering
 * the response with rt_gatt_send.
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
rd_status_t rt_gatt_on_nus_isr (ri_comm_evt_t evt,
                                void * p_data, size_t data_len)
{
    switch (evt)
    {
        // Note: This gets called only after the NUS notifications have been registered.
        case RI_COMM_CONNECTED:
            m_nus_is_connected = true;
            (NULL != m_on_connected) ? m_on_connected (p_data, data_len) : false;
            break;

        case RI_COMM_DISCONNECTED:
            m_nus_is_connected = false;
            (NULL != m_on_disconnected) ? m_on_disconnected (p_data, data_len) : false;
            break;

        case RI_COMM_SENT:
            (NULL != m_on_sent) ? m_on_sent (p_data, data_len) : false;
            break;

        case RI_COMM_RECEIVED:
            LOGD ("<<<;");
            LOGDHEX (p_data, data_len);
            LOGD (";\r\n");
            (NULL != m_on_received) ? m_on_received (p_data, data_len) : false;
            break;

        default:
            break;
    }

    return RD_SUCCESS;
}

rd_status_t rt_gatt_dis_init (const ri_comm_dis_init_t * const p_dis)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == p_dis)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (rt_gatt_is_init() && (!m_dis_is_init))
    {
        err_code |= ri_gatt_dis_init (p_dis);
        m_dis_is_init = (RD_SUCCESS == err_code);
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t rt_gatt_nus_init (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (rt_gatt_is_init() && (!m_nus_is_init))
    {
        err_code |= ri_gatt_nus_init (&m_channel);

        if (RD_SUCCESS == err_code)
        {
            m_channel.on_evt = rt_gatt_on_nus_isr;
            m_nus_is_init = true;
        }
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t rt_gatt_dfu_init (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (rt_gatt_is_init() && (!m_dfu_is_init))
    {
        err_code |= ri_gatt_dfu_init();
        m_dfu_is_init = (RD_SUCCESS == err_code);
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t rt_gatt_init (const char * const name)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == name)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (rt_adv_is_init() && (!rt_gatt_is_init()))
    {
        const size_t name_length = safe_strlen (name, sizeof (m_name));

        if (sizeof (m_name) > name_length)
        {
            err_code |= ri_gatt_init();
            memcpy (m_name, name, name_length);
            m_name[name_length] = '\0';
        }
        else
        {
            err_code |= RD_ERROR_INVALID_LENGTH;
        }
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    if (RD_SUCCESS == err_code)
    {
        m_is_init = true;
    }

    return err_code;
}

rd_status_t rt_gatt_uninit (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (rt_adv_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        ri_radio_modulation_t modulation = RI_RADIO_BLE_1MBPS;
        err_code |= ri_radio_get_modulation (&modulation);
        rt_gatt_set_on_received_isr (NULL);
        rt_gatt_set_on_sent_isr (NULL);
        rt_gatt_set_on_connected_isr (NULL);
        rt_gatt_set_on_disconn_isr (NULL);
        err_code |= ri_radio_uninit();
        err_code |= ri_gatt_uninit();
        memset (&m_channel, 0, sizeof (m_channel));
        err_code |= ri_radio_init (modulation);
        m_is_init = false;
        m_dis_is_init = false;
        m_nus_is_init = false;
        m_dfu_is_init = false;
    }

    return err_code;
}

rd_status_t rt_gatt_adv_enable (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (rt_gatt_is_init())
    {
        err_code |= rt_adv_connectability_set (true, m_name);
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t rt_gatt_adv_disable (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (rt_gatt_is_init())
    {
        err_code |= rt_adv_connectability_set (false, NULL);
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

bool rt_gatt_is_init (void)
{
    return m_is_init;
}

/**
 * @brief check if NUS is connected, i.e. central has registered to TX notifications.
 *
 * @return true if NUS is connected, false otherwise.
 */
bool rt_gatt_nus_is_connected (void)
{
    return m_nus_is_connected && (NULL != m_channel.send);
}

rd_status_t rt_gatt_send_asynchronous (ri_comm_message_t
                                       * const p_msg)
{
    rd_status_t err_code = RD_SUCCESS;

    // State, input check
    if (NULL == p_msg)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (!rt_gatt_nus_is_connected())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        // Try to put data to SD
        err_code |= m_channel.send (p_msg);

        // If success, return. Else put data to ringbuffer
        if (RD_SUCCESS == err_code)
        {
            LOGD (">>>;");
            LOGDHEX (p_msg->data, p_msg->data_length);
            LOGD (";\r\n");
        }
        else if (RD_ERROR_RESOURCES == err_code)
        {
            err_code = RD_ERROR_NO_MEM;
        }
        // If the error code is something else than buffer full, return error.
        else
        {
            RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        }
    }

    return err_code;
}

void rt_gatt_set_on_connected_isr (const ri_comm_cb_t cb)
{
    m_on_connected = cb;
}


void rt_gatt_set_on_disconn_isr (const ri_comm_cb_t cb)
{
    m_on_disconnected = cb;
}

void rt_gatt_set_on_received_isr (const ri_comm_cb_t cb)
{
    m_on_received = cb;
}

void rt_gatt_set_on_sent_isr (const ri_comm_cb_t cb)
{
    m_on_sent = cb;
}

bool rt_gatt_is_nus_enabled (void)
{
    return m_nus_is_init;
}

#else
rd_status_t rt_gatt_send_asynchronous (ri_comm_message_t
                                       * const p_msg)
{
    return RD_ERROR_NOT_ENABLED;
}

rd_status_t rt_gatt_dfu_init (void)
{
    return RD_ERROR_NOT_ENABLED;
}

rd_status_t rt_gatt_dis_init (const ri_comm_dis_init_t * const dis)
{
    return RD_ERROR_NOT_ENABLED;
}

rd_status_t rt_gatt_nus_init()
{
    return RD_ERROR_NOT_ENABLED;
}

rd_status_t rt_gatt_init (const char * const name)
{
    return RD_ERROR_NOT_ENABLED;
}

rd_status_t rt_gatt_adv_enable()
{
    return RD_ERROR_NOT_ENABLED;
}

rd_status_t rt_gatt_adv_disable()
{
    return RD_ERROR_NOT_ENABLED;
}

bool rt_gatt_is_init()
{
    return false;
}

void rt_gatt_set_on_connected_isr (const ri_comm_cb_t cb)
{
    // No implementation needed
}


void rt_gatt_set_on_disconn_isr (const ri_comm_cb_t cb)
{
    // No implementation needed
}

void rt_gatt_set_on_received_isr (const ri_comm_cb_t cb)
{
    // No implementation needed
}

void rt_gatt_set_on_sent_isr (const ri_comm_cb_t cb)
{
    // No implementation needed
}

bool rt_gatt_is_nus_enabled (void)
{
    return false;
}

#endif
