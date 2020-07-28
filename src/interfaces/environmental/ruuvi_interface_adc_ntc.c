#include "ruuvi_driver_enabled_modules.h"

#if (RI_ADC_NTC_ENABLED || DOXYGEN)

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_adc_ntc.h"
#include "ruuvi_interface_adc_mcu.h"
#include "ruuvi_interface_yield.h"

/**
 * @addtogroup ADC_NTC
 */
/*@{*/

/**
 * @file ruuvi_interface_adc_ntc.c
 * @author Oleg Protasevich
 * @date 2020-06-05
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * ADC_NTC sensor driver.
 *
 */

#ifdef RI_ADC_NTC_CHANNEL
#define ADC_NTC_USE_CHANNEL RI_ADC_NTC_CHANNEL
#else
#define ADC_NTC_USE_CHANNEL 1
#endif

#ifdef RI_ADC_NTC_DIVIDER
#define ADC_NTC_USE_DIVIDER RI_ADC_NTC_DIVIDER
#else
#define ADC_NTC_USE_DIVIDER 1.00f
#endif

#ifdef RI_ADC_NTC_VDD
#define ADC_NTC_USE_VDD RI_ADC_NTC_VDD
#else
#define ADC_NTC_USE_VDD 3.60f
#endif

#ifdef RI_ADC_NTC_BALANCE
#define ADC_NTC_BALANCE RI_ADC_NTC_BALANCE
#else
#define ADC_NTC_BALANCE 10000.00f
#endif

#ifdef RI_ADC_NTC_DEFAULT_RES
#define ADC_NTC_DEFAULT_RES RI_ADC_NTC_DEFAULT_RES
#else
#define ADC_NTC_DEFAULT_RES 10000.00f
#endif

#ifdef RI_ADC_NTC_DEFAULT_TEMP
#define ADC_NTC_DEFAULT_TEMP RI_ADC_NTC_DEFAULT_TEMP
#else
#define ADC_NTC_DEFAULT_TEMP 25.00f
#endif

#ifdef RI_ADC_NTC_DEFAULT_BETA
#define ADC_NTC_DEFAULT_BETA RI_ADC_NTC_DEFAULT_BETA
#else
#define ADC_NTC_DEFAULT_BETA  3974.0f
#endif

#define ADC_NTC_DATA_COUNTER      1
#define ADC_NTC_DEFAULT_BITFIELD  0
#define ADC_NTC_ENABLE_BYTE       1
#define ADC_K_TO_C_CONST          273.15f
#define ADC_NTC_DEFAULT_TEMP_K    (ADC_NTC_DEFAULT_TEMP + ADC_K_TO_C_CONST)

static ri_adc_pins_config_t adc_ntc_pins_config =
{
    .p_pin.channel = RI_ADC_GND,
#ifdef RI_ADC_ADV_CONFIG
    .p_pin.resistor = RI_ADC_RESISTOR_DISABLED,
#endif
};

static ri_adc_channel_config_t adc_ntc_config =
{
    .mode = RI_ADC_MODE_SINGLE,
    .vref = RI_ADC_VREF_EXTERNAL,
#ifdef RI_ADC_ADV_CONFIG
    .gain = RI_ADC_GAIN1_6,
    .acqtime = RI_ADC_ACQTIME_10US,
#endif
};

static ri_adc_get_data_t adc_ntc_options =
{
    .vdd = ADC_NTC_USE_VDD,
    .divider = ADC_NTC_USE_DIVIDER,
};

static uint64_t m_tsample;           //!< Timestamp of sample.
static bool m_autorefresh;           //!< Flag to refresh data on data_get.
float m_temperture;                  //!< Last measured temperature
static bool m_is_init;               //!< Flag, is sensor init.
static const char m_sensor_name[] = "NTC"; //!< Human-readable name of the sensor.

/**
 * @brief convert measured voltage ratio to temperature
 *
 * @param[in] ratio Measured voltage ratio in NTC divider. 0.0 ... 1.0
 * @return temperature in celcius.
 * @retval RD_FLOAT_INVALID if ratio < 0.0 or ratio > 1.0
 */
static float ratio_to_temperature (const float * const ratio)
{
    float result = RD_FLOAT_INVALID;
    float Rt;
    float Rb = (float)   ADC_NTC_BALANCE;        //!< Fixed divider resistance
    float beta = (float) ADC_NTC_DEFAULT_BETA;   //!< Beta of NTC
    float T0 = (float)   ADC_NTC_DEFAULT_TEMP_K; //!< Calibration temp of NTC.
    float R0 = (float)   RI_ADC_NTC_DEFAULT_RES; //!< Calibration resistance of NTC.

    if ( (NULL != ratio) && (*ratio >= 0.0F) && (*ratio <= 1.0F))
    {
        // Calculate NTC resistance.
        Rt = (Rb * *ratio) / (1.0F - *ratio);
        // 1/T = 1/T0 + 1/B * ln(R/R0)
        result = 1.0F / ( (1 / T0) + ( (1 / beta) * log (Rt / R0)));
        // Convert K->C
        result -= ADC_K_TO_C_CONST;
    }

    return result;
}

static rd_status_t get_data (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float ratio;
    m_tsample = rd_sensor_timestamp_get();

    if (RD_SUCCESS == ri_adc_get_data_ratio (ADC_NTC_USE_CHANNEL,
            &adc_ntc_options,
            &ratio))
    {
        //m_temperture = volts_to_temperature (&volts);
        m_temperture = ratio_to_temperature (&ratio);
    }
    else
    {
        err_code = RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t ri_adc_ntc_samplerate_set (uint8_t * samplerate)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_ntc_mode_get (&mode);
    return err_code | validate_default_input_set (samplerate, mode);
}
rd_status_t ri_adc_ntc_resolution_set (uint8_t * resolution)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_ntc_mode_get (&mode);
    return err_code | validate_default_input_set (resolution, mode);
}

rd_status_t ri_adc_ntc_scale_set (uint8_t * scale)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_ntc_mode_get (&mode);
    return err_code | validate_default_input_set (scale, mode);
}

rd_status_t ri_adc_ntc_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_ntc_mode_get (&mode);
    err_code |= validate_default_input_set (parameter, mode);

    if (RD_SUCCESS == err_code)
    {
        if (NULL == dsp)
        {
            err_code = RD_ERROR_NULL;
        }
        else
        {
            if (RD_SENSOR_DSP_LAST != (*dsp))
            {
                err_code = RD_ERROR_NOT_SUPPORTED;
            }
        }
    }

    return RD_SUCCESS;
}

rd_status_t ri_adc_ntc_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    return ( (validate_default_input_get (dsp)) |
             validate_default_input_get (parameter));
}

rd_status_t ri_adc_ntc_init (rd_sensor_t *
                             environmental_sensor, rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == environmental_sensor)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        if (m_is_init)
        {
            err_code = RD_ERROR_INVALID_STATE;
        }
        else
        {
            rd_sensor_initialize (environmental_sensor);
            environmental_sensor->name = m_sensor_name;
            err_code |= ri_adc_init (NULL);
            adc_ntc_pins_config.p_pin.channel = handle;
            err_code |= ri_adc_configure (ADC_NTC_USE_CHANNEL,
                                          &adc_ntc_pins_config,
                                          &adc_ntc_config);

            if (RD_SUCCESS == err_code)
            {
                environmental_sensor->init              = ri_adc_ntc_init;
                environmental_sensor->uninit            = ri_adc_ntc_uninit;
                environmental_sensor->samplerate_set    = ri_adc_ntc_samplerate_set;
                environmental_sensor->samplerate_get    = validate_default_input_get;
                environmental_sensor->resolution_set    = ri_adc_ntc_resolution_set;
                environmental_sensor->resolution_get    = validate_default_input_get;
                environmental_sensor->scale_set         = ri_adc_ntc_scale_set;
                environmental_sensor->scale_get         = validate_default_input_get;
                environmental_sensor->dsp_set           = ri_adc_ntc_dsp_set;
                environmental_sensor->dsp_get           = ri_adc_ntc_dsp_get;
                environmental_sensor->mode_set          = ri_adc_ntc_mode_set;
                environmental_sensor->mode_get          = ri_adc_ntc_mode_get;
                environmental_sensor->data_get          = ri_adc_ntc_data_get;
                environmental_sensor->configuration_set = rd_sensor_configuration_set;
                environmental_sensor->configuration_get = rd_sensor_configuration_get;
                environmental_sensor->provides.datas.temperature_c = ADC_NTC_ENABLE_BYTE;
                m_tsample = RD_UINT64_INVALID;
                m_is_init = true;
            }
        }
    }

    return err_code;
}

rd_status_t ri_adc_ntc_uninit (rd_sensor_t * sensor,
                               rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == sensor)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        rd_sensor_uninitialize (sensor);
        m_tsample = RD_UINT64_INVALID;
        m_is_init = false;
        m_autorefresh = false;
        err_code |= ri_adc_stop (ADC_NTC_USE_CHANNEL);
    }

    return err_code;
}

rd_status_t ri_adc_ntc_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_ERROR_INVALID_STATE;

    if (NULL == mode)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        if ( (RD_SENSOR_CFG_SLEEP == (*mode)) ||
                (RD_SENSOR_CFG_DEFAULT == (*mode)))
        {
            m_autorefresh = false;
            (*mode) = RD_SENSOR_CFG_SLEEP;
            err_code = RD_SUCCESS;
        }

        if (RD_SENSOR_CFG_CONTINUOUS == (*mode))
        {
            m_autorefresh = true;
            err_code = RD_SUCCESS;
        }

        if (RD_SENSOR_CFG_SINGLE == *mode)
        {
            uint8_t current_mode;
            ri_adc_ntc_mode_get (&current_mode);

            if (RD_SENSOR_CFG_CONTINUOUS == current_mode)
            {
                (*mode) = RD_SENSOR_CFG_CONTINUOUS;
                err_code = RD_ERROR_INVALID_STATE;
            }
            else
            {
                m_autorefresh = false;
                (*mode) = RD_SENSOR_CFG_SLEEP;
                err_code = get_data();
            }
        }
    }

    return err_code;
}

rd_status_t ri_adc_ntc_mode_get (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        if (m_autorefresh)
        {
            (*mode) = RD_SENSOR_CFG_CONTINUOUS;
        }

        if (!m_autorefresh)
        {
            (*mode) = RD_SENSOR_CFG_SLEEP;
        }
    }

    return err_code;
}

rd_status_t ri_adc_ntc_data_get (rd_sensor_data_t * const
                                 p_data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == p_data)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        if (m_autorefresh)
        {
            err_code = get_data();
        }

        if ( (RD_SUCCESS == err_code) &&
                (RD_UINT64_INVALID != m_tsample))
        {
            rd_sensor_data_t d_environmental;
            rd_sensor_data_fields_t env_fields = {.bitfield = ADC_NTC_DEFAULT_BITFIELD};
            float env_values;
            env_values = m_temperture;
            env_fields.datas.temperature_c = ADC_NTC_DATA_COUNTER;
            d_environmental.data = &env_values;
            d_environmental.valid  = env_fields;
            d_environmental.fields = env_fields;
            rd_sensor_data_populate (p_data,
                                     &d_environmental,
                                     p_data->fields);
            p_data->timestamp_ms = m_tsample;
        }
    }

    return err_code;
}

#endif