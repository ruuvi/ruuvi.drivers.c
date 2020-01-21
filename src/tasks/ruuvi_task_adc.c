
/**
 * @addtogroup adc_tasks
 */
/*@{*/
/**
 * @file rt_adc.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-11-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */
#include "ruuvi_driver_enabled_modules.h"
#if RT_ADC_ENABLED

#include "ruuvi_task_adc.h"

#include <stdbool.h>
#include <string.h>

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_adc_mcu.h"
#include "ruuvi_interface_atomic.h"

static ri_atomic_t m_is_init;
static bool m_is_configured;
static bool m_vdd_prepared;
static bool m_vdd_sampled;
static float m_vdd;
static rd_sensor_t m_adc; //!< ADC control instance

rd_status_t rt_adc_init (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!ri_atomic_flag (&m_is_init, true))
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t rt_adc_uninit (void)
{
    rd_status_t err_code = RD_SUCCESS;
    m_is_configured = false;
    m_vdd_prepared = false;
    m_vdd_sampled = false;
    err_code |= ri_adc_mcu_uninit (&m_adc, RD_BUS_NONE, 0);

    if (!ri_atomic_flag (&m_is_init, false))
    {
        err_code |= RD_ERROR_FATAL;
    }

    return err_code;
}

/**
 * @brief Check if ADC is initialized.
 *
 * @retval true if ADC is initialized.
 * @retval false if ADC is not initialized.
 */
inline bool rt_adc_is_init (void)
{
    return (0 != m_is_init);
}

rd_status_t rt_adc_configure_se (rd_sensor_configuration_t * const
                                 config, const uint8_t handle, const rt_adc_mode_t mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!rt_adc_is_init() || m_is_configured)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        // TODO @ojousima: Support ratiometric
        if (ABSOLUTE == mode)
        {
            err_code |= ri_adc_mcu_init (&m_adc, RD_BUS_NONE, handle);
            err_code |= m_adc.configuration_set (&m_adc, config);
        }
        else
        {
            err_code |= RD_ERROR_NOT_IMPLEMENTED;
        }
    }

    if (RD_SUCCESS == err_code)
    {
        m_is_configured = true;
    }

    return err_code;
}

rd_status_t rt_adc_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!rt_adc_is_init() || !m_is_configured || (NULL == m_adc.mode_set))
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        uint8_t mode = RD_SENSOR_CFG_SINGLE;
        err_code |= m_adc.mode_set (&mode);
    }

    return err_code;
}

rd_status_t rt_adc_voltage_get (rd_sensor_data_t * const data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!rt_adc_is_init() || !m_is_configured)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        err_code |= m_adc.data_get (data);
    }

    return err_code;
}

rd_status_t rt_adc_ratio_get (rd_sensor_data_t * const data)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t rt_adc_vdd_prepare (rd_sensor_configuration_t * const vdd_adc_configuration)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= rt_adc_init();
    err_code |= rt_adc_configure_se (vdd_adc_configuration, RI_ADC_AINVDD,
                                     ABSOLUTE);
    m_vdd_prepared = (RD_SUCCESS == err_code);
    return (RD_SUCCESS == err_code) ? RD_SUCCESS : RD_ERROR_BUSY;
}

rd_status_t rt_adc_vdd_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!m_vdd_prepared)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        rd_sensor_data_t battery;
        memset (&battery, 0, sizeof (rd_sensor_data_t));
        float battery_values;
        battery.data = &battery_values;
        battery.fields.datas.voltage_v = 1;
        err_code |= rt_adc_sample();
        err_code |= rt_adc_voltage_get (&battery);
        m_vdd = rd_sensor_data_parse (&battery, battery.fields);
        err_code |= rt_adc_uninit();
        m_vdd_prepared = false;
        m_vdd_sampled = true;
    }

    return err_code;
}

rd_status_t rt_adc_vdd_get (float * const battery)
{
    rd_status_t err_code = RD_SUCCESS;

    if (true == m_vdd_sampled)
    {
        *battery = m_vdd;
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}
#endif
/*@}*/