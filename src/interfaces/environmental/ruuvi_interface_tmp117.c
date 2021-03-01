#include "ruuvi_driver_enabled_modules.h"

#if (RI_TMP117_ENABLED || DOXYGEN)

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_tmp117.h"
#include "ruuvi_interface_i2c_tmp117.h"
#include "ruuvi_interface_yield.h"


/**
 * @addtogroup TMP117
 */
/** @{ */

/**
 * @file ruuvi_interface_tmp117.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2021-01-25
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * TMP117 temperature sensor driver.
 *
 */


static inline bool param_is_valid (const uint8_t param)
{
    return ( (RD_SENSOR_CFG_DEFAULT   == param)
             || (RD_SENSOR_CFG_MIN       == param)
             || (RD_SENSOR_CFG_MAX       == param)
             || (RD_SENSOR_CFG_NO_CHANGE == param));
}

static uint8_t  m_address;
static uint16_t ms_per_sample;
static uint16_t ms_per_cc;
static float    m_temperature;
static uint64_t m_timestamp;
static const char m_sensor_name[] = "TMP117";
static bool m_continuous = false;

static rd_status_t tmp117_soft_reset (void)
{
    uint16_t reset = TMP117_MASK_RESET & 0xFFFF;
    return ri_i2c_tmp117_write (m_address, TMP117_REG_CONFIGURATION, reset);
}

static rd_status_t tmp117_validate_id (void)
{
    uint16_t id;
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_i2c_tmp117_read (m_address, TMP117_REG_DEVICE_ID, &id);
    id &= TMP117_MASK_ID;

    if (TMP117_VALUE_ID != id)
    {
        err_code |= RD_ERROR_NOT_FOUND;
    }

    return err_code;
}

static rd_status_t tmp117_oversampling_set (const uint8_t num_os)
{
    uint16_t reg_val;
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                    &reg_val);
    reg_val &= ~TMP117_MASK_OS;

    switch (num_os)
    {
        case TMP117_VALUE_OS_1:
            reg_val |= TMP117_VALUE_OS_1;

            if (16 > ms_per_cc)
            {
                err_code |= RD_ERROR_INVALID_STATE;
            }

            ms_per_sample = 16;
            break;

        case TMP117_VALUE_OS_8:
            reg_val |= TMP117_VALUE_OS_8;

            if (125 > ms_per_cc)
            {
                err_code |= RD_ERROR_INVALID_STATE;
            }

            ms_per_sample = 125;
            break;

        case TMP117_VALUE_OS_32:
            reg_val |= TMP117_VALUE_OS_32;

            if (500 > ms_per_cc)
            {
                err_code |= RD_ERROR_INVALID_STATE;
            }

            ms_per_sample = 500;
            break;

        case TMP117_VALUE_OS_64:
            reg_val |= TMP117_VALUE_OS_64;

            if (1000 > ms_per_cc)
            {
                err_code |= RD_ERROR_INVALID_STATE;
            }

            ms_per_sample = 1000;
            break;

        default:
            err_code | RD_ERROR_INVALID_PARAM;
    }

    err_code |= ri_i2c_tmp117_write (m_address, TMP117_REG_CONFIGURATION,
                                     reg_val);
    return err_code;
}

static rd_status_t tmp117_samplerate_set (const uint16_t num_os)
{
    uint16_t reg_val;
    rd_status_t err_code;
    err_code = ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                   &reg_val);
    reg_val &= ~TMP117_MASK_CC;

    switch (num_os)
    {
        case TMP117_VALUE_CC_16_MS:
            if (16 < ms_per_sample) { return RD_ERROR_INVALID_STATE; }

            reg_val |= TMP117_VALUE_CC_16_MS;
            ms_per_cc = 16;
            break;

        case TMP117_VALUE_CC_125_MS:
            if (125 < ms_per_sample) { return RD_ERROR_INVALID_STATE; }

            reg_val |= TMP117_VALUE_CC_125_MS;
            ms_per_cc = 125;
            break;

        case TMP117_VALUE_CC_250_MS:
            if (250 < ms_per_sample) { return RD_ERROR_INVALID_STATE; }

            reg_val |= TMP117_VALUE_CC_250_MS;
            ms_per_cc = 250;
            break;

        case TMP117_VALUE_CC_500_MS:
            if (500 < ms_per_sample) { return RD_ERROR_INVALID_STATE; }

            reg_val |= TMP117_VALUE_CC_500_MS;
            ms_per_cc = 500;
            break;

        case TMP117_VALUE_CC_1000_MS:
            if (1000 < ms_per_sample) { return RD_ERROR_INVALID_STATE; }

            reg_val |= TMP117_VALUE_CC_1000_MS;
            ms_per_cc = 1000;
            break;

        case TMP117_VALUE_CC_4000_MS:
            if (4000 < ms_per_sample) { return RD_ERROR_INVALID_STATE; }

            reg_val |= TMP117_VALUE_CC_4000_MS;
            ms_per_cc = 4000;
            break;

        case TMP117_VALUE_CC_8000_MS:
            if (8000 < ms_per_sample) { return RD_ERROR_INVALID_STATE; }

            reg_val |= TMP117_VALUE_CC_8000_MS;
            ms_per_cc = 8000;
            break;

        case TMP117_VALUE_CC_16000_MS:
            if (16000 < ms_per_sample) { return RD_ERROR_INVALID_STATE; }

            reg_val |= TMP117_VALUE_CC_16000_MS;
            ms_per_cc = 16000;
            break;

        default:
            return RD_ERROR_INVALID_PARAM;
    }

    err_code |= ri_i2c_tmp117_write (m_address, TMP117_REG_CONFIGURATION,
                                     reg_val);
    return err_code;
}

static rd_status_t tmp117_sleep (void)
{
    uint16_t reg_val;
    rd_status_t err_code;
    err_code = ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                   &reg_val);
    reg_val &= ~TMP117_MASK_MODE;
    reg_val |= TMP117_VALUE_MODE_SLEEP;
    err_code |= ri_i2c_tmp117_write (m_address, TMP117_REG_CONFIGURATION,
                                     reg_val);
    return  err_code;
}

static rd_status_t tmp117_sample (void)
{
    uint16_t reg_val;
    rd_status_t err_code;
    err_code = ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                   &reg_val);
    reg_val &= ~TMP117_MASK_MODE;
    reg_val |= TMP117_VALUE_MODE_SINGLE;
    err_code |= ri_i2c_tmp117_write (m_address, TMP117_REG_CONFIGURATION,
                                     reg_val);
    m_timestamp = rd_sensor_timestamp_get();
    return  err_code;
}

static rd_status_t tmp117_continuous (void)
{
    uint16_t reg_val;
    rd_status_t err_code;
    err_code = ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                   &reg_val);
    reg_val &= ~TMP117_MASK_MODE;
    reg_val |= TMP117_VALUE_MODE_CONT;
    err_code |= ri_i2c_tmp117_write (m_address, TMP117_REG_CONFIGURATION,
                                     reg_val);
    return  err_code;
}

static float tmp117_read (void)
{
    uint16_t reg_val;
    rd_status_t err_code;
    err_code = ri_i2c_tmp117_read (m_address, TMP117_REG_TEMP_RESULT, &reg_val);
    int16_t dec_temperature = (reg_val > 32767) ? reg_val - 65535 : reg_val;
    float temperature = (0.0078125 * dec_temperature);

    if (TMP117_VALUE_TEMP_NA == reg_val || RD_SUCCESS != err_code) { temperature = NAN; }

    return temperature;
}

rd_status_t
ri_tmp117_init (rd_sensor_t * environmental_sensor, rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == environmental_sensor)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (rd_sensor_is_init (environmental_sensor))
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        rd_sensor_initialize (environmental_sensor);
        environmental_sensor->name = m_sensor_name;
        err_code = RD_SUCCESS;
        m_address = handle;
        size_t retries = 0;

        if (RD_BUS_I2C == bus)
        {
            err_code |= tmp117_validate_id();
        }
        else
        {
            err_code |=  RD_ERROR_INVALID_PARAM;
        }

        if (RD_SUCCESS == err_code)
        {
            err_code |= tmp117_soft_reset();
            environmental_sensor->init              = ri_tmp117_init;
            environmental_sensor->uninit            = ri_tmp117_uninit;
            environmental_sensor->samplerate_set    = ri_tmp117_samplerate_set;
            environmental_sensor->samplerate_get    = ri_tmp117_samplerate_get;
            environmental_sensor->resolution_set    = ri_tmp117_resolution_set;
            environmental_sensor->resolution_get    = ri_tmp117_resolution_get;
            environmental_sensor->scale_set         = ri_tmp117_scale_set;
            environmental_sensor->scale_get         = ri_tmp117_scale_get;
            environmental_sensor->dsp_set           = ri_tmp117_dsp_set;
            environmental_sensor->dsp_get           = ri_tmp117_dsp_get;
            environmental_sensor->mode_set          = ri_tmp117_mode_set;
            environmental_sensor->mode_get          = ri_tmp117_mode_get;
            environmental_sensor->data_get          = ri_tmp117_data_get;
            environmental_sensor->configuration_set = rd_sensor_configuration_set;
            environmental_sensor->configuration_get = rd_sensor_configuration_get;
            environmental_sensor->provides.datas.temperature_c = 1;
            m_timestamp = RD_UINT64_INVALID;
            m_temperature = NAN;
            ms_per_cc = 1000;
            ms_per_sample = 16;
            m_continuous = false;
        }
    }

    return err_code;
}

rd_status_t ri_tmp117_uninit (rd_sensor_t * sensor,
                              rd_bus_t bus, uint8_t handle)
{
    if (NULL == sensor) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;
    tmp117_sleep();
    rd_sensor_uninitialize (sensor);
    m_timestamp = RD_UINT64_INVALID;
    m_temperature = NAN;
    m_address = 0;
    m_continuous = false;
    return err_code;
}


rd_status_t ri_tmp117_samplerate_set (uint8_t * samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    if (m_continuous) { return RD_ERROR_INVALID_STATE; }

    if (RD_SENSOR_CFG_NO_CHANGE == *samplerate)
    {
        return ri_tmp117_samplerate_get (samplerate);
    }

    rd_status_t err_code = RD_SUCCESS;

    if (RD_SENSOR_CFG_DEFAULT == *samplerate ||
            1 >= *samplerate)
    {
        *samplerate = 1;
        err_code |= tmp117_samplerate_set (TMP117_VALUE_CC_1000_MS);
    }
    else if (2 >= *samplerate)
    {
        *samplerate = 2;
        err_code |= tmp117_samplerate_set (TMP117_VALUE_CC_500_MS);
    }
    else if (4 >= *samplerate)
    {
        *samplerate = 4;
        err_code |= tmp117_samplerate_set (TMP117_VALUE_CC_250_MS);
    }
    else if (8 >= *samplerate)
    {
        *samplerate = 8;
        err_code |= tmp117_samplerate_set (TMP117_VALUE_CC_125_MS);
    }
    else if (64 >= *samplerate ||
             RD_SENSOR_CFG_MAX == *samplerate)
    {
        *samplerate = 64;
        err_code |= tmp117_samplerate_set (TMP117_VALUE_CC_16_MS);
    }
    else if (RD_SENSOR_CFG_CUSTOM_1 == *samplerate)
    {
        err_code |= tmp117_samplerate_set (TMP117_VALUE_CC_4000_MS);
    }
    else if (RD_SENSOR_CFG_CUSTOM_2 == *samplerate)
    {
        err_code |= tmp117_samplerate_set (TMP117_VALUE_CC_8000_MS);
    }
    else if (RD_SENSOR_CFG_CUSTOM_3 == *samplerate ||
             RD_SENSOR_CFG_MIN == *samplerate)
    {
        *samplerate = RD_SENSOR_CFG_CUSTOM_3;
        err_code |= tmp117_samplerate_set (TMP117_VALUE_CC_16000_MS);
    }
    else { err_code |= RD_ERROR_NOT_SUPPORTED; }

    return  err_code;
}

rd_status_t ri_tmp117_samplerate_get (uint8_t * samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;
    uint16_t reg_val;
    err_code = ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                   &reg_val);
    reg_val &= TMP117_MASK_CC;

    switch (reg_val)
    {
        case TMP117_VALUE_CC_16_MS:
            *samplerate = 64;
            break;

        case TMP117_VALUE_CC_125_MS:
            *samplerate = 8;
            break;

        case TMP117_VALUE_CC_250_MS:
            *samplerate = 4;
            break;

        case TMP117_VALUE_CC_500_MS:
            *samplerate = 2;
            break;

        case TMP117_VALUE_CC_1000_MS:
            *samplerate = 1;
            break;

        case TMP117_VALUE_CC_4000_MS:
            *samplerate = RD_SENSOR_CFG_CUSTOM_1;
            break;

        case TMP117_VALUE_CC_8000_MS:
            *samplerate = RD_SENSOR_CFG_CUSTOM_2;
            break;

        case TMP117_VALUE_CC_16000_MS:
            *samplerate = RD_SENSOR_CFG_CUSTOM_3;
            break;

        default:
            return RD_ERROR_INTERNAL;
    }

    return err_code;
}

rd_status_t ri_tmp117_resolution_set (uint8_t * resolution)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == resolution)
    {
        return RD_ERROR_NULL;
    }
    else if (m_continuous)
    {
        return RD_ERROR_INVALID_STATE;
    }
    else if (! param_is_valid (*resolution))
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }
    else
    {
        *resolution = RD_SENSOR_CFG_DEFAULT;
    }

    return err_code;
}

rd_status_t ri_tmp117_resolution_get (uint8_t * resolution)
{
    if (NULL == resolution) { return RD_ERROR_NULL; }

    *resolution = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_tmp117_scale_set (uint8_t * scale)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == scale)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (m_continuous)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (!param_is_valid (*scale))
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }
    else
    {
        *scale = RD_SENSOR_CFG_DEFAULT;
    }

    return err_code;
}

rd_status_t ri_tmp117_scale_get (uint8_t * scale)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == scale)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        *scale = RD_SENSOR_CFG_DEFAULT;
    }

    return err_code;
}

rd_status_t ri_tmp117_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter) { return RD_ERROR_NULL; }

    if (m_continuous) { return RD_ERROR_INVALID_STATE; }

    if (RD_SENSOR_CFG_NO_CHANGE == * dsp)
    {
        return ri_tmp117_dsp_get (dsp, parameter);
    }

    rd_status_t err_code = RD_SUCCESS;

    if (RD_SENSOR_DSP_LAST == *dsp ||
            RD_SENSOR_CFG_DEFAULT == *dsp)
    {
        err_code |= tmp117_oversampling_set (TMP117_VALUE_OS_1);
        *parameter = 1;
    }
    else if (RD_SENSOR_DSP_OS == *dsp)
    {
        if (1 >= *parameter)
        {
            *parameter = 1;
            err_code |= tmp117_oversampling_set (TMP117_VALUE_OS_1);
        }
        else if (8 >= *parameter)
        {
            *parameter = 8;
            err_code |= tmp117_oversampling_set (TMP117_VALUE_OS_8);
        }
        else if (32 >= *parameter)
        {
            *parameter = 32;
            err_code |= tmp117_oversampling_set (TMP117_VALUE_OS_32);
        }
        else if (64 >= *parameter)
        {
            *parameter = 64;
            err_code |= tmp117_oversampling_set (TMP117_VALUE_OS_64);
        }
        else { err_code |= RD_ERROR_NOT_SUPPORTED; }
    }
    else { err_code |= RD_ERROR_NOT_SUPPORTED; }

    return err_code;
}

rd_status_t ri_tmp117_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter) { return RD_ERROR_NULL; }

    uint16_t reg_val;
    rd_status_t err_code;
    err_code = ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                   &reg_val);
    reg_val &= TMP117_MASK_OS;

    switch (reg_val)
    {
        case TMP117_VALUE_OS_1:
            *dsp = RD_SENSOR_DSP_LAST;
            *parameter = 1;
            break;

        case TMP117_VALUE_OS_8:
            *dsp = RD_SENSOR_DSP_OS;
            *parameter = 8;
            break;

        case TMP117_VALUE_OS_32:
            *dsp = RD_SENSOR_DSP_OS;
            *parameter = 32;
            break;

        case TMP117_VALUE_OS_64:
            *dsp = RD_SENSOR_DSP_OS;
            *parameter = 64;
            break;
    }

    return err_code;
}


rd_status_t ri_tmp117_mode_set (uint8_t * mode)
{
    if (NULL == mode) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;

    switch (*mode)
    {
        case RD_SENSOR_CFG_CONTINUOUS:
            err_code |= tmp117_continuous();
            m_continuous = true;
            break;

        case RD_SENSOR_CFG_SINGLE:
            if (m_continuous)
            {
                *mode = RD_SENSOR_CFG_CONTINUOUS;
                return RD_ERROR_INVALID_STATE;
            }

            err_code |= tmp117_sample();
            ri_delay_ms (ms_per_sample);
            *mode = RD_SENSOR_CFG_SLEEP;
            break;

        case RD_SENSOR_CFG_SLEEP:
            err_code |= tmp117_sleep();
            m_continuous = false;
            break;

        default:
            err_code |= RD_ERROR_INVALID_PARAM;
    }

    return err_code;
}

rd_status_t ri_tmp117_mode_get (uint8_t * mode)
{
    if (NULL == mode) { return RD_ERROR_NULL; }

    *mode = m_continuous ? RD_SENSOR_CFG_CONTINUOUS : RD_SENSOR_CFG_SLEEP;
    return RD_SUCCESS;
}

rd_status_t ri_tmp117_data_get (rd_sensor_data_t * const
                                data)
{
    if (NULL == data) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;

    if (m_continuous)
    {
        m_temperature = tmp117_read();
        m_timestamp = rd_sensor_timestamp_get();
    }

    if (RD_SUCCESS == err_code && RD_UINT64_INVALID != m_timestamp)
    {
        rd_sensor_data_fields_t env_fields = {.bitfield = 0};
        env_fields.datas.temperature_c = 1;
        rd_sensor_data_set (data,
                            env_fields,
                            m_temperature);
        data->timestamp_ms = m_timestamp;
    }

    return err_code;
}

/** @} */
#endif
