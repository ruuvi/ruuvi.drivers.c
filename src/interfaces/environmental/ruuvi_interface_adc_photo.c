#include "ruuvi_driver_enabled_modules.h"

#if (RI_ADC_PHOTO_ENABLED || DOXYGEN)

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_adc_photo.h"
#include "ruuvi_interface_adc_mcu.h"
#include "ruuvi_interface_yield.h"

/**
 * @addtogroup ADC_PHOTO
 */
/*@{*/

/**
 * @file ruuvi_interface_adc_photo.c
 * @author Oleg Protasevich
 * @date 2020-06-05
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * ADC_PHOTO sensor driver.
 *
 */

#ifdef RI_ADC_PHOTO_CHANNEL
#define ADC_PHOTO_USE_CHANNEL RI_ADC_PHOTO_CHANNEL
#else
#define ADC_PHOTO_USE_CHANNEL 1
#endif

#ifdef RI_ADC_PHOTO_DIVIDER
#define ADC_PHOTO_USE_DIVIDER RI_ADC_PHOTO_DIVIDER
#else
#define ADC_PHOTO_USE_DIVIDER 1.00f
#endif

#ifdef RI_ADC_PHOTO_VDD
#define ADC_PHOTO_USE_VDD RI_ADC_PHOTO_VDD
#else
#define ADC_PHOTO_USE_VDD 3.60f
#endif

#define ADC_PHOTO_DATA_COUNTER      1
#define ADC_PHOTO_DEFAULT_BITFIELD  0
#define ADC_PHOTO_ENABLE_BYTE       1
#define ADC_PHOTO_VOLTS_TO_LUX      1333.00f

static ri_adc_pins_config_t adc_photo_pins_config =
{
    .p_pin.channel = RI_ADC_GND,
#ifdef RI_ADC_ADV_CONFIG
    .p_pin.resistor = RI_ADC_RESISTOR_DISABLED,
#endif
};

static ri_adc_channel_config_t adc_photo_config =
{
    .mode = RI_ADC_MODE_SINGLE,
    .vref = RI_ADC_VREF_EXTERNAL,
#ifdef RI_ADC_ADV_CONFIG
    .gain = RI_ADC_GAIN1_6,
    .acqtime = RI_ADC_ACQTIME_10US,
#endif
};

static ri_adc_get_data_t adc_photo_options =
{
    .vdd = ADC_PHOTO_USE_VDD,
    .divider = ADC_PHOTO_USE_DIVIDER,
};

static uint64_t m_tsample;           //!< Timestamp of sample.
static bool m_autorefresh;           //!< Flag to refresh data on data_get.
float m_luminosity;
static bool m_is_init;               //!< Flag, is sensor init.
static const char m_sensor_name[] = "PHOTO"; //!< Human-readable name of the sensor.


static float volts_to_lux (const float * const data)
{
    float result;
    result = (float) (ADC_PHOTO_VOLTS_TO_LUX) * (*data);
    return result;
}

static rd_status_t get_data (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float volts;
    m_tsample = rd_sensor_timestamp_get();

    if (RD_SUCCESS == ri_adc_get_data_absolute (ADC_PHOTO_USE_CHANNEL,
            &adc_photo_options,
            &volts))
    {
        m_luminosity = volts_to_lux (&volts);
    }
    else
    {
        err_code = RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t ri_adc_photo_samplerate_set (uint8_t * samplerate)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_photo_mode_get (&mode);
    return err_code | validate_default_input_set (samplerate, mode);
}

rd_status_t ri_adc_photo_resolution_set (uint8_t * resolution)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_photo_mode_get (&mode);
    return err_code | validate_default_input_set (resolution, mode);
}

rd_status_t ri_adc_photo_scale_set (uint8_t * scale)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_photo_mode_get (&mode);
    return err_code | validate_default_input_set (scale, mode);
}

rd_status_t ri_adc_photo_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_photo_mode_get (&mode);
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

rd_status_t ri_adc_photo_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    uint8_t mode;
    rd_status_t err_code = ri_adc_photo_mode_get (&mode);
    err_code |= validate_default_input_get (dsp);
    err_code |= validate_default_input_get (parameter);
    return err_code;
}

rd_status_t ri_adc_photo_init (rd_sensor_t *
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
            adc_photo_pins_config.p_pin.channel = handle;
            err_code |= ri_adc_configure (ADC_PHOTO_USE_CHANNEL,
                                          &adc_photo_pins_config,
                                          &adc_photo_config);

            if (RD_SUCCESS == err_code)
            {
                environmental_sensor->init              = ri_adc_photo_init;
                environmental_sensor->uninit            = ri_adc_photo_uninit;
                environmental_sensor->samplerate_set    = ri_adc_photo_samplerate_set;
                environmental_sensor->samplerate_get    = validate_default_input_get;
                environmental_sensor->resolution_set    = ri_adc_photo_resolution_set;
                environmental_sensor->resolution_get    = validate_default_input_get;
                environmental_sensor->scale_set         = ri_adc_photo_scale_set;
                environmental_sensor->scale_get         = validate_default_input_get;
                environmental_sensor->dsp_set           = ri_adc_photo_dsp_set;
                environmental_sensor->dsp_get           = ri_adc_photo_dsp_get;
                environmental_sensor->mode_set          = ri_adc_photo_mode_set;
                environmental_sensor->mode_get          = ri_adc_photo_mode_get;
                environmental_sensor->data_get          = ri_adc_photo_data_get;
                environmental_sensor->configuration_set = rd_sensor_configuration_set;
                environmental_sensor->configuration_get = rd_sensor_configuration_get;
                environmental_sensor->provides.datas.luminosity = ADC_PHOTO_ENABLE_BYTE;
                m_tsample = RD_UINT64_INVALID;
                m_is_init = true;
            }
        }
    }

    return err_code;
}

rd_status_t ri_adc_photo_uninit (rd_sensor_t * sensor,
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
        err_code |= ri_adc_stop (ADC_PHOTO_USE_CHANNEL);
    }

    return err_code;
}

rd_status_t ri_adc_photo_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        if ( (RD_SENSOR_CFG_SLEEP == (*mode)) ||
                (RD_SENSOR_CFG_DEFAULT == (*mode)))
        {
            m_autorefresh = false;
            (*mode) = RD_SENSOR_CFG_SLEEP;
        }
        else if (RD_SENSOR_CFG_SINGLE == *mode)
        {
            uint8_t current_mode;
            ri_adc_photo_mode_get (&current_mode);

            if (RD_SENSOR_CFG_CONTINUOUS == current_mode)
            {
                (*mode) = RD_SENSOR_CFG_CONTINUOUS;
                err_code |= RD_ERROR_INVALID_STATE;
            }
            else
            {
                m_autorefresh = false;
                (*mode) = RD_SENSOR_CFG_SLEEP;
                err_code |= get_data();
            }
        }
        else if (RD_SENSOR_CFG_CONTINUOUS == (*mode))
        {
            m_autorefresh = true;
        }
        else
        {
            err_code |= RD_ERROR_INVALID_PARAM;
        }
    }

    return err_code;
}

rd_status_t ri_adc_photo_mode_get (uint8_t * mode)
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

rd_status_t ri_adc_photo_data_get (rd_sensor_data_t * const
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
            rd_sensor_data_fields_t env_fields = {.bitfield = ADC_PHOTO_DEFAULT_BITFIELD};
            float env_values;
            env_values = m_luminosity;
            env_fields.datas.luminosity = ADC_PHOTO_DATA_COUNTER;
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