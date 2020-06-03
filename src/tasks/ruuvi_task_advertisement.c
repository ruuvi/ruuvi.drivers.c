/**
 * @addtogroup advertisement_tasks
 */
/** @{ */
/**
 * @file ruuvi_task_advertisement.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-06-01
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Advertise data and GATT connection if available.
 */

#include "ruuvi_driver_enabled_modules.h"
#if RT_ADV_ENABLED

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_task_advertisement.h"
#include "ruuvi_task_gatt.h"

// https://github.com/arm-embedded/gcc-arm-none-eabi.debian/blob/master/src/libiberty/strnlen.c
// Not included when compiled with std=c99.
static size_t safe_strlen (const char * s, size_t maxlen)
{
    size_t i;

    for (i = 0; i < maxlen; ++i)
        if (s[i] == '\0')
        { break; }

    return i;
}

static ri_comm_channel_t m_channel;
static bool m_is_init;

rd_status_t rt_adv_init (rt_adv_init_t * const adv_init_settings)
{
    rd_status_t err_code = RD_SUCCESS;

    if (m_is_init)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        err_code |= ri_adv_init (&m_channel);
        err_code |= ri_adv_tx_interval_set (adv_init_settings->adv_interval_ms);
        err_code |= ri_adv_tx_power_set (& (adv_init_settings->adv_pwr_dbm));
        err_code |= ri_adv_type_set (NONCONNECTABLE_NONSCANNABLE);
        err_code |= ri_adv_manufacturer_id_set (adv_init_settings->manufacturer_id);

        if (RD_SUCCESS == err_code)
        {
            m_is_init = true;
        }
    }

    return err_code;
}

rd_status_t rt_adv_uninit (void)
{
    m_is_init = false;
    return ri_adv_uninit (&m_channel);
}

rd_status_t rt_adv_stop (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!m_is_init)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        err_code |= ri_adv_stop();
    }

    return err_code;
}

rd_status_t rt_adv_send_data (ri_comm_message_t * const msg)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == msg)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (!rt_adv_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (24 < msg->data_length)
    {
        err_code |= RD_ERROR_DATA_SIZE;
    }
    else
    {
        err_code |= m_channel.send (msg);
    }

    return err_code;
}

rd_status_t rt_adv_connectability_set (const bool enable, const char * const device_name)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!rt_adv_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (!enable)
    {
        err_code |= ri_adv_type_set (NONCONNECTABLE_NONSCANNABLE);
    }
    else if (NULL == device_name)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (SCAN_RSP_NAME_MAX_LEN < safe_strlen (device_name, (SCAN_RSP_NAME_MAX_LEN + 1)))
    {
        err_code |= RD_ERROR_INVALID_LENGTH;
    }
    else
    {
        err_code |= ri_adv_type_set (CONNECTABLE_SCANNABLE);
        err_code |= ri_adv_scan_response_setup (device_name, rt_gatt_is_nus_enabled());
    }

    return err_code;
}

inline bool rt_adv_is_init (void)
{
    return ( (NULL != m_channel.send) && (true == m_is_init));
}

rd_status_t rt_adv_scan_start (const ri_comm_evt_handler_fp_t on_evt)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!rt_adv_is_init())
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        m_channel.on_evt = on_evt;
        err_code |= ri_adv_scan_start (RT_ADV_SCAN_INTERVAL_MS, RT_ADV_SCAN_WINDOW_MS);
    }

    return err_code;
}

rd_status_t rt_adv_scan_stop (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!m_is_init)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        err_code |= ri_adv_scan_stop();
    }

    return err_code;
}

#endif
/** @} */
