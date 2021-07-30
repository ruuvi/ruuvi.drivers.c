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

#define TMP117_CC_RETRIES_MAX    (5U)
#define TMP117_CC_RETRY_DELAY_MS (10U)

static uint8_t  m_address;
static uint16_t ms_per_sample;
static uint16_t ms_per_cc;
static float    m_temperature;
static uint64_t m_timestamp;
static const char m_sensor_name[] = "TMP117";
static bool m_continuous = false;

static inline bool param_is_valid (const uint8_t param)
{
    return ( (RD_SENSOR_CFG_DEFAULT   == param)
             || (RD_SENSOR_CFG_MIN       == param)
             || (RD_SENSOR_CFG_MAX       == param)
             || (RD_SENSOR_CFG_NO_CHANGE == param));
}

static rd_status_t tmp117_soft_reset (void)
{
    uint16_t reset = TMP117_MASK_RESET & 0xFFFF;
    rd_status_t err_code = ri_i2c_tmp117_write (m_address,
                           TMP117_REG_CONFIGURATION,
                           reset);
    ri_delay_ms (TMP117_CC_RESET_DELAY_MS);
    return err_code;
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

static inline rd_status_t os_1_set (uint16_t * const reg_val)
{
    rd_status_t err_code = RD_SUCCESS;
    *reg_val |= TMP117_VALUE_OS_1;

    if (TMP117_OS_1_TSAMPLE_MS > ms_per_cc)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    ms_per_sample = TMP117_OS_1_TSAMPLE_MS;
    return err_code;
}

static inline rd_status_t os_8_set (uint16_t * const reg_val)
{
    rd_status_t err_code = RD_SUCCESS;
    *reg_val |= TMP117_VALUE_OS_8;

    if (TMP117_OS_8_TSAMPLE_MS > ms_per_cc)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    ms_per_sample = TMP117_OS_8_TSAMPLE_MS;
    return err_code;
}

static inline rd_status_t os_32_set (uint16_t * const reg_val)
{
    rd_status_t err_code = RD_SUCCESS;
    *reg_val |= TMP117_VALUE_OS_32;

    if (TMP117_OS_32_TSAMPLE_MS > ms_per_cc)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    ms_per_sample = TMP117_OS_32_TSAMPLE_MS;
    return err_code;
}

static inline rd_status_t os_64_set (uint16_t * const reg_val)
{
    rd_status_t err_code = RD_SUCCESS;
    *reg_val |= TMP117_VALUE_OS_64;

    if (TMP117_OS_64_TSAMPLE_MS > ms_per_cc)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    ms_per_sample = TMP117_OS_64_TSAMPLE_MS;
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
            reg_val |= os_1_set (&reg_val);
            break;

        case TMP117_VALUE_OS_8:
            reg_val |= os_8_set (&reg_val);
            break;

        case TMP117_VALUE_OS_32:
            reg_val |= os_32_set (&reg_val);
            break;

        case TMP117_VALUE_OS_64:
            reg_val |= os_64_set (&reg_val);
            break;

        default:
            err_code |= RD_ERROR_INVALID_PARAM;
    }

    err_code |= ri_i2c_tmp117_write (m_address, TMP117_REG_CONFIGURATION,
                                     reg_val);
    return err_code;
}

static rd_status_t
tmp117_cc_check (uint16_t * const reg, const uint16_t ts, const uint16_t reg_val)
{
    rd_status_t err_code = RD_SUCCESS;

    if (ts < ms_per_sample)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        *reg |= reg_val;
        ms_per_cc = ts;
    }

    return err_code;
}

static rd_status_t tmp117_samplerate_set (const uint16_t num_os)
{
    uint16_t reg_val;
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                    &reg_val);
    reg_val &= ~TMP117_MASK_CC;

    switch (num_os)
    {
        case TMP117_VALUE_CC_16_MS:
            err_code |= tmp117_cc_check (&reg_val, ms_per_sample, TMP117_VALUE_CC_16_MS);
            break;

        case TMP117_VALUE_CC_125_MS:
            err_code |= tmp117_cc_check (&reg_val, ms_per_sample, TMP117_VALUE_CC_125_MS);
            break;

        case TMP117_VALUE_CC_250_MS:
            err_code |= tmp117_cc_check (&reg_val, ms_per_sample, TMP117_VALUE_CC_250_MS);
            break;

        case TMP117_VALUE_CC_500_MS:
            err_code |= tmp117_cc_check (&reg_val, ms_per_sample, TMP117_VALUE_CC_500_MS);
            break;

        case TMP117_VALUE_CC_1000_MS:
            err_code |= tmp117_cc_check (&reg_val, ms_per_sample, TMP117_VALUE_CC_1000_MS);
            break;

        case TMP117_VALUE_CC_4000_MS:
            err_code |= tmp117_cc_check (&reg_val, ms_per_sample, TMP117_VALUE_CC_4000_MS);
            break;

        case TMP117_VALUE_CC_8000_MS:
            err_code |= tmp117_cc_check (&reg_val, ms_per_sample, TMP117_VALUE_CC_8000_MS);
            break;

        case TMP117_VALUE_CC_16000_MS:
            err_code |= tmp117_cc_check (&reg_val, ms_per_sample, TMP117_VALUE_CC_16000_MS);
            break;

        default:
            err_code |= RD_ERROR_INVALID_PARAM;
            break;
    }

    err_code |= ri_i2c_tmp117_write (m_address, TMP117_REG_CONFIGURATION, reg_val);
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
    uint16_t reg_val = 0;
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
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

static rd_status_t tmp117_read (float * const temperature)
{
    uint16_t reg_val;
    rd_status_t err_code;
    err_code = ri_i2c_tmp117_read (m_address, TMP117_REG_TEMP_RESULT, &reg_val);
    int32_t dec_temperature;

    if (reg_val > 0x7FFFU)
    {
        dec_temperature = (int32_t) reg_val - 0xFFFF;
    }
    else
    {
        dec_temperature = reg_val;
    }

    *temperature = (0.0078125F * dec_temperature);

    if ( (TMP117_VALUE_TEMP_NA == reg_val) || (RD_SUCCESS != err_code))
    {
        *temperature = NAN;
    }

    return err_code;
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
            ms_per_sample = TMP117_OS_8_TSAMPLE_MS; //!< default OS setting
            m_continuous = false;
        }
    }

    return err_code;
}

rd_status_t ri_tmp117_uninit (rd_sensor_t * sensor, rd_bus_t bus, uint8_t handle)
{
    UNUSED_VARIABLE (bus);
    UNUSED_VARIABLE (handle);
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == sensor)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        tmp117_sleep();
        rd_sensor_uninitialize (sensor);
        m_timestamp = RD_UINT64_INVALID;
        m_temperature = NAN;
        m_address = 0;
        m_continuous = false;
    }

    return err_code;
}


rd_status_t ri_tmp117_samplerate_set (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == samplerate)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (m_continuous)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (RD_SENSOR_CFG_NO_CHANGE == *samplerate)
    {
        err_code |= ri_tmp117_samplerate_get (samplerate);
    }
    else if ( (RD_SENSOR_CFG_DEFAULT == *samplerate)
              || (1 >= *samplerate))
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
    else
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }

    return  err_code;
}

rd_status_t ri_tmp117_samplerate_get (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;
    uint16_t reg_val = 0;

    if (NULL == samplerate)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        err_code |= ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
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
                err_code |=  RD_ERROR_INTERNAL;
        }
    }

    return err_code;
}

rd_status_t ri_tmp117_resolution_set (uint8_t * resolution)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == resolution)
    {
        err_code |=  RD_ERROR_NULL;
    }
    else if (m_continuous)
    {
        err_code |=  RD_ERROR_INVALID_STATE;
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
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == resolution)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        *resolution = RD_SENSOR_CFG_DEFAULT;
    }

    return err_code;
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
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == dsp) || (NULL == parameter))
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (m_continuous)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        if (RD_SENSOR_CFG_NO_CHANGE == * dsp)
        {
            err_code |= ri_tmp117_dsp_get (dsp, parameter);
        }
        else if ( (RD_SENSOR_DSP_LAST == *dsp)
                  || (RD_SENSOR_CFG_DEFAULT == *dsp))
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
            else if (RD_SENSOR_CFG_MIN == *parameter)
            {
                *parameter = 8;
                err_code |= tmp117_oversampling_set (TMP117_VALUE_OS_8);
            }
            else if (RD_SENSOR_CFG_MAX == *parameter)
            {
                *parameter = 64;
                err_code |= tmp117_oversampling_set (TMP117_VALUE_OS_64);
            }
            else
            {
                err_code |= RD_ERROR_NOT_SUPPORTED;
            }
        }
        else
        {
            err_code |= RD_ERROR_NOT_SUPPORTED;
        }
    }

    return err_code;
}

rd_status_t ri_tmp117_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == dsp) || (NULL == parameter))
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        uint16_t reg_val = 0;
        err_code |= ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION,
                                        &reg_val);
        reg_val &= TMP117_MASK_OS;

        switch (reg_val)
        {
            case TMP117_VALUE_OS_1:
                *dsp = RD_SENSOR_DSP_LAST;
                *parameter = 1;
                ms_per_sample = TMP117_OS_1_TSAMPLE_MS;
                break;

            case TMP117_VALUE_OS_8:
                *dsp = RD_SENSOR_DSP_OS;
                *parameter = 8;
                ms_per_sample = TMP117_OS_8_TSAMPLE_MS;
                break;

            case TMP117_VALUE_OS_32:
                *dsp = RD_SENSOR_DSP_OS;
                *parameter = 32;
                ms_per_sample = TMP117_OS_32_TSAMPLE_MS;
                break;

            case TMP117_VALUE_OS_64:
                *dsp = RD_SENSOR_DSP_OS;
                *parameter = 64;
                ms_per_sample = TMP117_OS_64_TSAMPLE_MS;
                break;

            default:
                err_code |= RD_ERROR_INTERNAL;
        }
    }

    return err_code;
}

static rd_status_t tmp117_poll_drdy (bool * const drdy)
{
    rd_status_t err_code = RD_SUCCESS;
    uint16_t cfg = 0;
    err_code |= ri_i2c_tmp117_read (m_address, TMP117_REG_CONFIGURATION, &cfg);
    cfg &= TMP117_MASK_DRDY;
    *drdy = (cfg != 0);
    return err_code;
}

static rd_status_t tmp117_wait_for_sample (const uint16_t initial_delay_ms)
{
    rd_status_t err_code = RD_SUCCESS;
    bool drdy = false;
    uint8_t retries = 0;
    ri_delay_ms (initial_delay_ms);

    while ( (RD_SUCCESS == err_code) && (!drdy) && (retries <= TMP117_CC_RETRIES_MAX))
    {
        err_code |= tmp117_poll_drdy (&drdy);

        if (!drdy)
        {
            ri_delay_ms (TMP117_CC_RETRY_DELAY_MS);
            retries++;
        }
    }

    if (retries > TMP117_CC_RETRIES_MAX)
    {
        err_code |= RD_ERROR_TIMEOUT;
    }

    return err_code;
}

static rd_status_t __attribute__ ( (nonnull))
tmp117_take_single_sample (uint8_t * const mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (m_continuous)
    {
        err_code |= RD_ERROR_INVALID_STATE;
        *mode = RD_SENSOR_CFG_CONTINUOUS;
    }
    else
    {
        err_code |= tmp117_sample ();

        if (RD_SUCCESS == err_code)
        {
            err_code |= tmp117_wait_for_sample (ms_per_sample);
        }

        if (RD_SUCCESS == err_code)
        {
            err_code |= tmp117_read (&m_temperature);
        }

        *mode = RD_SENSOR_CFG_SLEEP;
    }

    return err_code;
}

rd_status_t ri_tmp117_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        switch (*mode)
        {
            case RD_SENSOR_CFG_CONTINUOUS:
                err_code |= tmp117_continuous();
                m_continuous = true;
                break;

            case RD_SENSOR_CFG_SINGLE:
                err_code |= tmp117_take_single_sample (mode);
                break;

            case RD_SENSOR_CFG_SLEEP:
                err_code |= tmp117_sleep();
                m_continuous = false;
                break;

            default:
                err_code |= RD_ERROR_INVALID_PARAM;
                break;
        }
    }

    return err_code;
}

rd_status_t ri_tmp117_mode_get (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (m_continuous)
    {
        *mode = RD_SENSOR_CFG_CONTINUOUS;
    }
    else
    {
        *mode = RD_SENSOR_CFG_SLEEP;
    }

    return err_code;
}

rd_status_t ri_tmp117_data_get (rd_sensor_data_t * const data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == data)
    {
        err_code |= RD_ERROR_NULL;
    }

    if (m_continuous)
    {
        err_code |= tmp117_read (&m_temperature);
        m_timestamp = rd_sensor_timestamp_get();
    }

    if ( (RD_SUCCESS == err_code) && (RD_UINT64_INVALID != m_timestamp)
            && !isnan (m_temperature))
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
