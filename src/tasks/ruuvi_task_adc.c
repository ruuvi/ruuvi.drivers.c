
/**
 * @addtogroup adc_tasks
 */
/** @{*/
/**
 * @file ruuvi_task_adc.c
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

#define RD_ADC_USE_DIVIDER      1.00f
#define RD_ADC_USE_VDD          3.30f

#define RD_ADC_DATA_COUNTER     1
#define RD_ADC_DATA_START       0
#define RD_ADC_DATA_VOLTAGE     1

#define RD_ADC_DEFAULT_BITFIELD 0
#define RD_ADC_CLEAN_BYTE       0
#define RD_ADC_INIT_BYTE        0

#define RT_ADC_CH_UNUSED        (0xFFU) //!< Channel not assigned.

static ri_atomic_t m_is_init;
static bool m_is_configured;
static bool m_vdd_prepared;
static bool m_vdd_sampled;
static bool m_ratio;
static float m_vdd;
static uint8_t m_handle; //!< handle of last ADC used.
static uint8_t m_channel[RI_ADC_CH_NUM]; //!< Channel assigment for handles.
static uint8_t m_next_channel; //!< Next channel to be assigned.

/**
 * @brief assign ADC channel for a handle
 *
 * @retval RD_SUCCESS If channel was assigned or handle already has channel.
 * @retval RD_ERROR_RESOURCES If there are no channels available.
 * @retval RD_ERROR_INVALID_PARAM If
 */
static rd_status_t channel_assign (const uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    if (handle > RI_ADC_CH_NUM)
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }
    else if (RT_ADC_CH_UNUSED == m_channel[handle])
    {
        if (ri_adc_mcu_is_valid_ch (m_next_channel))
        {
            m_channel[handle] = m_next_channel++;
        }
        else
        {
            err_code |= RD_ERROR_RESOURCES;
        }
    }
    else
    {
        // No action needed.
    }

    return err_code;
}


static ri_adc_pins_config_t pins_config =
{
    .p_pin.channel = RI_ADC_AINVDD,
#ifdef RI_ADC_ADV_CONFIG
    .p_pin.resistor = RI_ADC_RESISTOR_DISABLED,
#endif
};

static ri_adc_channel_config_t absolute_config =
{
    .mode = RI_ADC_MODE_SINGLE,
    .vref = RI_ADC_VREF_INTERNAL,
#ifdef RI_ADC_ADV_CONFIG
    .gain = RI_ADC_GAIN1_6,
    .acqtime = RI_ADC_ACQTIME_10US,
#endif
};

static ri_adc_get_data_t options =
{
    .vdd = RD_ADC_USE_VDD,
    .divider = RD_ADC_USE_DIVIDER,
};

static rd_status_t rt_adc_mcu_data_get (rd_sensor_data_t * const
                                        p_data)
{
    rd_status_t status = RD_ERROR_INVALID_STATE;

    if (NULL == p_data)
    {
        status = RD_ERROR_NULL;
    }
    else
    {
        p_data->timestamp_ms = RD_UINT64_INVALID;
        rd_sensor_data_t d_adc;
        rd_sensor_data_fields_t adc_fields = {.bitfield = RD_ADC_DEFAULT_BITFIELD};
        float adc_values[RD_ADC_DATA_COUNTER] = {0};

        if (false == m_ratio)
        {
            status = ri_adc_get_data_absolute (m_channel[m_handle],
                                               &options,
                                               &adc_values[RD_ADC_DATA_START]);
        }
        else
        {
            status = ri_adc_get_data_ratio (m_channel[m_handle],
                                            &options,
                                            &adc_values[RD_ADC_DATA_START]);
        }

        if (RD_SUCCESS == status)
        {
            adc_fields.datas.voltage_v = RD_ADC_DATA_COUNTER;
            d_adc.data = adc_values;
            d_adc.valid  = adc_fields;
            d_adc.fields = adc_fields;
            rd_sensor_data_populate (p_data,
                                     &d_adc,
                                     p_data->fields);
            p_data->timestamp_ms = rd_sensor_timestamp_get();
        }
    }

    return RD_SUCCESS;
}

rd_status_t rt_adc_init (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!ri_atomic_flag (&m_is_init, true))
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        for (size_t ii = 0; ii < RI_ADC_CH_NUM; ii++)
        {
            m_channel[ii] = RT_ADC_CH_UNUSED;
        }

        m_next_channel = 0;
        err_code |= ri_adc_init (NULL);
    }

    return err_code;
}

rd_status_t rt_adc_uninit (void)
{
    rd_status_t err_code = RD_SUCCESS;
    m_is_configured = false;
    m_vdd_prepared = false;
    m_vdd_sampled = false;
    m_ratio = false;
    err_code |= ri_adc_stop (m_channel[m_handle]);
    err_code |= ri_adc_uninit (true);

    for (size_t ii = 0; ii < RI_ADC_CH_NUM; ii++)
    {
        m_channel[ii] = RT_ADC_CH_UNUSED;
    }

    m_next_channel = 0;

    if (!ri_atomic_flag (&m_is_init, false))
    {
        err_code |= RD_ERROR_FATAL;
    }

    return err_code;
}

inline bool rt_adc_is_init (void)
{
    return (RD_ADC_INIT_BYTE != m_is_init);
}

rd_status_t rt_adc_configure_se (rd_sensor_configuration_t * const config,
                                 const uint8_t handle, const rt_adc_mode_t mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!rt_adc_is_init() || m_is_configured)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (RI_ADC_GND == handle)
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }
    else
    {
        pins_config.p_pin.channel = handle;

        if (ABSOLUTE == mode)
        {
            m_ratio = false;
        }
        else
        {
            m_ratio = true;
        }

        // Handle is used as channel index, however there is NONE at index 0.
        m_handle = handle;
        err_code |= channel_assign (m_handle);

        if (RD_SUCCESS == err_code)
        {
            err_code |= ri_adc_configure (m_channel[m_handle],
                                          &pins_config,
                                          &absolute_config);
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

    if (!rt_adc_is_init() || !m_is_configured)
    {
        err_code |= RD_ERROR_INVALID_STATE;
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
        if (true == m_ratio)
        {
            err_code |= RD_ERROR_INVALID_STATE;
        }
        else
        {
            err_code |= rt_adc_mcu_data_get (data);
        }
    }

    return err_code;
}

rd_status_t rt_adc_ratio_get (rd_sensor_data_t * const data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (!rt_adc_is_init() || !m_is_configured)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        if (false == m_ratio)
        {
            err_code |= RD_ERROR_INVALID_STATE;
        }
        else
        {
            err_code |= rt_adc_mcu_data_get (data);
        }
    }

    return err_code;
}

rd_status_t rt_adc_vdd_prepare (rd_sensor_configuration_t * const vdd_adc_configuration)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= rt_adc_init();

    if (err_code == RD_SUCCESS)
    {
        err_code |= rt_adc_configure_se (vdd_adc_configuration, RI_ADC_AINVDD,
                                         ABSOLUTE);

        if (err_code == RD_SUCCESS)
        {
            m_vdd_prepared = true;
        }
        else
        {
            err_code |= rt_adc_uninit();
        }
    }

    return err_code;
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
        memset (&battery, RD_ADC_CLEAN_BYTE, sizeof (rd_sensor_data_t));
        float battery_values;
        battery.data = &battery_values;
        battery.fields.datas.voltage_v = RD_ADC_DATA_VOLTAGE;
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

rd_status_t rt_adc_absolute_sample (rd_sensor_configuration_t * const configuration,
                                    const uint8_t handle, float * const sample)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == configuration) || (NULL == sample)) { return  RD_ERROR_NULL; }

    rd_sensor_data_t d_adc;
    memset (&d_adc, RD_ADC_CLEAN_BYTE, sizeof (rd_sensor_data_t));
    float battery_values;
    d_adc.data = &battery_values;
    d_adc.fields.datas.voltage_v = RD_ADC_DATA_VOLTAGE;
    err_code |= rt_adc_init();
    err_code |= rt_adc_configure_se (configuration, handle, ABSOLUTE);
    m_vdd_prepared = (RD_SUCCESS == err_code);

    if (m_vdd_prepared)
    {
        err_code |= rt_adc_sample();
        err_code |= rt_adc_voltage_get (&d_adc);
    }

    *sample = rd_sensor_data_parse (&d_adc, d_adc.fields);
    return err_code;
}

rd_status_t rt_adc_ratiometric_sample (rd_sensor_configuration_t * const configuration,
                                       const uint8_t handle, float * const sample)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == configuration) || (NULL == sample)) { return  RD_ERROR_NULL; }

    rd_sensor_data_t d_adc;
    memset (&d_adc, RD_ADC_CLEAN_BYTE, sizeof (rd_sensor_data_t));
    float rate_values;
    d_adc.data = &rate_values;
    d_adc.fields.datas.voltage_ratio = RD_ADC_DATA_VOLTAGE;
    err_code |= rt_adc_init();
    err_code |= rt_adc_configure_se (configuration, handle, RATIOMETRIC);
    m_vdd_prepared = (RD_SUCCESS == err_code);

    if (m_vdd_prepared)
    {
        err_code |= rt_adc_sample();
        err_code |= rt_adc_ratio_get (&d_adc);
    }

    *sample = rd_sensor_data_parse (&d_adc, d_adc.fields);
    return err_code;
}
#endif
/** @}*/
