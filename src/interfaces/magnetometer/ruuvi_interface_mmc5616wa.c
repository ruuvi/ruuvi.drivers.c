#include "ruuvi_driver_enabled_modules.h"
#if (RI_MMC5616WA_ENABLED || DOXYGEN)

#include "ruuvi_interface_mmc5616wa.h"

#include "ruuvi_interface_i2c_mmc5616wa.h"
#include "ruuvi_interface_yield.h"

#include <string.h>

static const char m_sensor_name[] = "MMC5616";
static const rd_sensor_data_fields_t m_fields =
{
    .datas.magnetometer_x_g = 1,
    .datas.magnetometer_y_g = 1,
    .datas.magnetometer_z_g = 1,
    .datas.temperature_c = 1
};

#ifndef CEEDLING
static
#endif

ri_mmc5616wa_dev_t dev = {0};

static void ri_mmc5616wa_delay_ms (uint32_t period_ms)
{
    (void) ri_delay_ms (period_ms);
}

static rd_status_t mmc_status_to_rd (const int32_t status)
{
    switch (status)
    {
        case MMC5616WA_OK:
            return RD_SUCCESS;

        case MMC5616WA_E_NULL:
            return RD_ERROR_NULL;

        case MMC5616WA_E_INVALID_PARAM:
            return RD_ERROR_INVALID_PARAM;

        case MMC5616WA_E_TIMEOUT:
            return RD_ERROR_TIMEOUT;

        case MMC5616WA_E_NOT_FOUND:
            return RD_ERROR_NOT_FOUND;

        case MMC5616WA_E_IO:
        default:
            return RD_ERROR_INTERNAL;
    }
}

static rd_status_t dev_ctx_init (const rd_bus_t bus, const uint8_t handle)
{
    if (RD_BUS_I2C != bus)
    {
        return RD_ERROR_NOT_SUPPORTED;
    }

    dev.handle = handle;
    dev.ctx.write_reg = &ri_i2c_mmc5616wa_write;
    dev.ctx.read_reg = &ri_i2c_mmc5616wa_read;
    dev.ctx.delay_ms = &ri_mmc5616wa_delay_ms;
    dev.ctx.handle = &dev.handle;
    dev.mode = RD_SENSOR_CFG_SLEEP;
    dev.tsample = RD_UINT64_INVALID;
    mmc5616wa_default_config (&dev.config);
    dev.config.odr = RI_MMC5616WA_DEFAULT_SAMPLERATE;
    mmc5616wa_default_fifo_config (&dev.fifo_config);
    return RD_SUCCESS;
}

static bool default_param_is_valid (const uint8_t parameter)
{
    return ( (RD_SENSOR_CFG_DEFAULT == parameter)
             || (RD_SENSOR_CFG_MIN == parameter)
             || (RD_SENSOR_CFG_MAX == parameter)
             || (RD_SENSOR_CFG_NO_CHANGE == parameter));
}

static rd_status_t default_param_set (uint8_t * const parameter,
                                      const uint8_t value,
                                      const uint8_t mode)
{
    if (NULL == parameter)
    {
        return RD_ERROR_NULL;
    }

    if (RD_SENSOR_CFG_SLEEP != mode)
    {
        return RD_ERROR_INVALID_STATE;
    }

    if ( (*parameter == value) || default_param_is_valid (*parameter))
    {
        *parameter = value;
        return RD_SUCCESS;
    }

    *parameter = RD_SENSOR_ERR_NOT_SUPPORTED;
    return RD_ERROR_NOT_SUPPORTED;
}

static void sensor_fps_setup (rd_sensor_t * const p_sensor)
{
    p_sensor->init = &ri_mmc5616wa_init;
    p_sensor->uninit = &ri_mmc5616wa_uninit;
    p_sensor->samplerate_set = &ri_mmc5616wa_samplerate_set;
    p_sensor->samplerate_get = &ri_mmc5616wa_samplerate_get;
    p_sensor->resolution_set = &ri_mmc5616wa_resolution_set;
    p_sensor->resolution_get = &ri_mmc5616wa_resolution_get;
    p_sensor->scale_set = &ri_mmc5616wa_scale_set;
    p_sensor->scale_get = &ri_mmc5616wa_scale_get;
    p_sensor->dsp_set = &ri_mmc5616wa_dsp_set;
    p_sensor->dsp_get = &ri_mmc5616wa_dsp_get;
    p_sensor->mode_set = &ri_mmc5616wa_mode_set;
    p_sensor->mode_get = &ri_mmc5616wa_mode_get;
    p_sensor->data_get = &ri_mmc5616wa_data_get;
    p_sensor->configuration_set = &rd_sensor_configuration_set;
    p_sensor->configuration_get = &rd_sensor_configuration_get;
    p_sensor->fifo_enable = &ri_mmc5616wa_fifo_use;
    p_sensor->fifo_interrupt_enable = &ri_mmc5616wa_fifo_interrupt_use;
    p_sensor->fifo_read = &ri_mmc5616wa_fifo_read;
}

static rd_status_t sample_take (void)
{
    mmc5616wa_mag_raw_t raw_magnetic = {0};
    uint8_t raw_temperature = 0U;
    rd_status_t err_code = mmc_status_to_rd (
                               mmc5616wa_magnetic_measurement_get (&dev.ctx, &raw_magnetic));

    if (RD_SUCCESS == err_code)
    {
        err_code |= mmc_status_to_rd (
                        mmc5616wa_temperature_measurement_get (&dev.ctx, &raw_temperature));
    }

    if (RD_SUCCESS == err_code)
    {
        dev.values[0] = mmc5616wa_magnetic_20bit_to_gauss (raw_magnetic.x);
        dev.values[1] = mmc5616wa_magnetic_20bit_to_gauss (raw_magnetic.y);
        dev.values[2] = mmc5616wa_magnetic_20bit_to_gauss (raw_magnetic.z);
        dev.values[3] = mmc5616wa_temperature_to_celsius (raw_temperature);
        dev.tsample = rd_sensor_timestamp_get();
    }

    return err_code;
}

rd_status_t ri_mmc5616wa_init (rd_sensor_t * p_sensor, rd_bus_t bus,
                               uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
    bool selftest_passed = false;

    if (NULL == p_sensor)
    {
        return RD_ERROR_NULL;
    }

    if (rd_sensor_is_init (p_sensor))
    {
        return RD_ERROR_INVALID_STATE;
    }

    rd_sensor_initialize (p_sensor);
    p_sensor->name = m_sensor_name;
    err_code |= dev_ctx_init (bus, handle);

    if (RD_SUCCESS == err_code)
    {
        err_code |= mmc_status_to_rd (mmc5616wa_init (&dev.ctx));
    }

    if (RD_SUCCESS == err_code)
    {
        err_code |= mmc_status_to_rd (mmc5616wa_self_test (&dev.ctx,
                                  &selftest_passed));

        if (!selftest_passed)
        {
            err_code |= RD_ERROR_SELFTEST;
        }
    }

    if (RD_SUCCESS == err_code)
    {
        sensor_fps_setup (p_sensor);
        p_sensor->provides = m_fields;
        p_sensor->p_ctx = &dev.ctx;
    }
    else
    {
        rd_sensor_uninitialize (p_sensor);
        memset (&dev, 0, sizeof (dev));
    }

    return err_code;
}

rd_status_t ri_mmc5616wa_uninit (rd_sensor_t * p_sensor, rd_bus_t bus,
                                 uint8_t handle)
{
    UNUSED_VARIABLE (bus);
    UNUSED_VARIABLE (handle);

    if (NULL == p_sensor)
    {
        return RD_ERROR_NULL;
    }

    dev.config.continuous_mode = false;
    dev.config.odr = 0U;
    (void) mmc5616wa_config_set (&dev.ctx, &dev.config);
    rd_sensor_uninitialize (p_sensor);
    memset (&dev, 0, sizeof (dev));
    return RD_SUCCESS;
}

rd_status_t ri_mmc5616wa_samplerate_set (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == samplerate)
    {
        return RD_ERROR_NULL;
    }

    if (RD_SENSOR_CFG_NO_CHANGE == *samplerate)
    {
        return ri_mmc5616wa_samplerate_get (samplerate);
    }

    if ( (RD_SENSOR_CFG_DEFAULT == *samplerate)
            || (RD_SENSOR_CFG_MIN == *samplerate)
            || (RI_MMC5616WA_DEFAULT_SAMPLERATE >= *samplerate))
    {
        dev.config.odr = RI_MMC5616WA_DEFAULT_SAMPLERATE;
    }
    else if (RD_SENSOR_CFG_MAX == *samplerate)
    {
        dev.config.odr = RI_MMC5616WA_MAX_SAMPLERATE;
    }
    else if (RI_MMC5616WA_MAX_SAMPLERATE >= *samplerate)
    {
        dev.config.odr = *samplerate;
    }
    else
    {
        *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED;
        return RD_ERROR_NOT_SUPPORTED;
    }

    if (RD_SENSOR_CFG_CONTINUOUS == dev.mode)
    {
        err_code |= mmc_status_to_rd (mmc5616wa_config_set (&dev.ctx, &dev.config));
    }

    err_code |= ri_mmc5616wa_samplerate_get (samplerate);
    return err_code;
}

rd_status_t ri_mmc5616wa_samplerate_get (uint8_t * samplerate)
{
    if (NULL == samplerate)
    {
        return RD_ERROR_NULL;
    }

    *samplerate = dev.config.odr;
    return RD_SUCCESS;
}

rd_status_t ri_mmc5616wa_resolution_set (uint8_t * resolution)
{
    return default_param_set (resolution, RI_MMC5616WA_RESOLUTION_BITS, dev.mode);
}

rd_status_t ri_mmc5616wa_resolution_get (uint8_t * resolution)
{
    if (NULL == resolution)
    {
        return RD_ERROR_NULL;
    }

    *resolution = RI_MMC5616WA_RESOLUTION_BITS;
    return RD_SUCCESS;
}

rd_status_t ri_mmc5616wa_scale_set (uint8_t * scale)
{
    return default_param_set (scale, RI_MMC5616WA_SCALE_GAUSS, dev.mode);
}

rd_status_t ri_mmc5616wa_scale_get (uint8_t * scale)
{
    if (NULL == scale)
    {
        return RD_ERROR_NULL;
    }

    *scale = RI_MMC5616WA_SCALE_GAUSS;
    return RD_SUCCESS;
}

rd_status_t ri_mmc5616wa_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    if ( (NULL == dsp) || (NULL == parameter))
    {
        return RD_ERROR_NULL;
    }

    if (RD_SENSOR_CFG_SLEEP != dev.mode)
    {
        return RD_ERROR_INVALID_STATE;
    }

    if ( (RD_SENSOR_CFG_DEFAULT == *dsp) || (RD_SENSOR_CFG_NO_CHANGE == *dsp)
            || (RD_SENSOR_DSP_LAST == *dsp))
    {
        *dsp = RD_SENSOR_DSP_LAST;
        *parameter = RD_SENSOR_CFG_DEFAULT;
        return RD_SUCCESS;
    }

    *dsp = RD_SENSOR_ERR_NOT_SUPPORTED;
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_mmc5616wa_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    if ( (NULL == dsp) || (NULL == parameter))
    {
        return RD_ERROR_NULL;
    }

    *dsp = RD_SENSOR_DSP_LAST;
    *parameter = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_mmc5616wa_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        return RD_ERROR_NULL;
    }

    switch (*mode)
    {
        case RD_SENSOR_CFG_SLEEP:
            dev.config.continuous_mode = false;
            err_code |= mmc_status_to_rd (mmc5616wa_config_set (&dev.ctx, &dev.config));
            dev.mode = RD_SENSOR_CFG_SLEEP;
            break;

        case RD_SENSOR_CFG_SINGLE:
            dev.config.continuous_mode = false;
            err_code |= mmc_status_to_rd (mmc5616wa_config_set (&dev.ctx, &dev.config));

            if (RD_SUCCESS == err_code)
            {
                err_code |= sample_take();
            }

            dev.mode = RD_SENSOR_CFG_SLEEP;
            *mode = RD_SENSOR_CFG_SLEEP;
            break;

        case RD_SENSOR_CFG_CONTINUOUS:
            dev.config.continuous_mode = true;
            err_code |= mmc_status_to_rd (mmc5616wa_config_set (&dev.ctx, &dev.config));
            dev.mode = RD_SENSOR_CFG_CONTINUOUS;
            break;

        default:
            err_code |= RD_ERROR_INVALID_PARAM;
            break;
    }

    return err_code;
}

rd_status_t ri_mmc5616wa_mode_get (uint8_t * mode)
{
    if (NULL == mode)
    {
        return RD_ERROR_NULL;
    }

    *mode = dev.mode;
    return RD_SUCCESS;
}

rd_status_t ri_mmc5616wa_data_get (rd_sensor_data_t * const data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == data)
    {
        return RD_ERROR_NULL;
    }

    if (RD_SENSOR_CFG_CONTINUOUS == dev.mode)
    {
        err_code |= sample_take();
    }
    else if (RD_UINT64_INVALID == dev.tsample)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        // Use the last single-shot sample.
    }

    if (RD_SUCCESS == err_code)
    {
        const rd_sensor_data_t provided =
        {
            .timestamp_ms = dev.tsample,
            .fields = m_fields,
            .valid = m_fields,
            .data = dev.values
        };
        rd_sensor_data_populate (data, &provided, data->fields);
    }

    return err_code;
}

rd_status_t ri_mmc5616wa_fifo_use (const bool enable)
{
    dev.fifo_config.enable = enable;
    dev.fifo_config.address_loop_enable = enable;
    return mmc_status_to_rd (mmc5616wa_fifo_config_set (&dev.ctx,
                             &dev.fifo_config));
}

rd_status_t ri_mmc5616wa_fifo_interrupt_use (const bool enable)
{
    dev.fifo_config.enable = enable;
    dev.fifo_config.interrupt_enable = enable;
    dev.fifo_config.address_loop_enable = enable;
    dev.fifo_config.watermark = enable ? (MMC5616WA_FIFO_DEPTH - 1U) : 0U;
    return mmc_status_to_rd (mmc5616wa_fifo_config_set (&dev.ctx,
                             &dev.fifo_config));
}

rd_status_t ri_mmc5616wa_fifo_read (size_t * num_elements,
                                    rd_sensor_data_t * data)
{
    if ( (NULL == num_elements) || (NULL == data))
    {
        return RD_ERROR_NULL;
    }

    size_t samples_read = 0U;
    mmc5616wa_fifo_sample_t samples[MMC5616WA_FIFO_BURST_MAX_SAMPLES] = {0};
    size_t samples_requested = *num_elements;

    if (MMC5616WA_FIFO_BURST_MAX_SAMPLES < samples_requested)
    {
        samples_requested = MMC5616WA_FIFO_BURST_MAX_SAMPLES;
    }

    rd_status_t err_code = mmc_status_to_rd (mmc5616wa_fifo_read (&dev.ctx,
                           samples, samples_requested, &samples_read));
    *num_elements = samples_read;

    if (RD_SUCCESS == err_code)
    {
        const uint64_t timestamp = rd_sensor_timestamp_get();

        for (size_t ii = 0U; ii < samples_read; ii++)
        {
            float values[3] =
            {
                mmc5616wa_magnetic_16bit_to_gauss (samples[ii].x),
                mmc5616wa_magnetic_16bit_to_gauss (samples[ii].y),
                mmc5616wa_magnetic_16bit_to_gauss (samples[ii].z)
            };
            const rd_sensor_data_fields_t fields =
            {
                .datas.magnetometer_x_g = 1,
                .datas.magnetometer_y_g = 1,
                .datas.magnetometer_z_g = 1
            };
            const rd_sensor_data_t provided =
            {
                .timestamp_ms = timestamp,
                .fields = fields,
                .valid = fields,
                .data = values
            };
            rd_sensor_data_populate (&data[ii], &provided, data[ii].fields);
        }
    }

    return err_code;
}

#endif