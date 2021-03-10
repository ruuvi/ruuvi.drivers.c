#include "ruuvi_driver_enabled_modules.h"


#if (RI_DPS310_ENABLED || DOXYGEN)
#include "ruuvi_interface_dps310.h"
#include "dps310.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_spi_dps310.h"
#include "ruuvi_interface_yield.h"
#include <stdint.h>
#include <string.h>

static void dps_sleep (const uint32_t ms)
{
    (void) ri_delay_ms (ms);
}

// Singleton access for compatibility with other driver implementations.
static uint8_t spi_comm_handle;
static dps310_ctx_t singleton_ctx_spi =
{
    .write = &ri_spi_dps310_write,
    .read  = &ri_spi_dps310_read,
    .sleep = &dps_sleep,
    .comm_ctx = &spi_comm_handle
};

static const char * const name = "DPS310";
const rd_sensor_data_fields_t dps_fields =
{
    .datas.temperature_c = 1,
    .datas.pressure_pa = 1
};
static float last_values[2];
static rd_sensor_data_t last_data;

static __attribute__ ( (nonnull)) void
dps310_singleton_spi_setup (rd_sensor_t * const p_sensor,
                            const uint8_t handle)
{
    p_sensor->p_ctx = &singleton_ctx_spi;
    spi_comm_handle = handle;
}

static __attribute__ ( (nonnull))
void dps310_fp_setup (rd_sensor_t * const p_sensor)
{
    p_sensor->init = &ri_dps310_init;
    p_sensor->uninit = &ri_dps310_uninit;
    p_sensor->samplerate_set = &ri_dps310_samplerate_set;
    p_sensor->samplerate_get = &ri_dps310_samplerate_get;
    p_sensor->resolution_set = &ri_dps310_resolution_set;
    p_sensor->resolution_get = &ri_dps310_resolution_get;
    p_sensor->scale_set = &ri_dps310_scale_set;
    p_sensor->scale_get = &ri_dps310_scale_get;
    p_sensor->dsp_set = &ri_dps310_dsp_set;
    p_sensor->dsp_get = &ri_dps310_dsp_get;
    p_sensor->mode_set = &ri_dps310_mode_set;
    p_sensor->mode_get = &ri_dps310_mode_get;
    p_sensor->data_get = &ri_dps310_data_get;
    p_sensor->configuration_set = &rd_sensor_configuration_set;
    p_sensor->configuration_get = &rd_sensor_configuration_get;
    return;
}

rd_status_t ri_dps310_init (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
    dps310_status_t dps_status = DPS310_SUCCESS;

    if (NULL == p_sensor)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (rd_sensor_is_init (p_sensor))
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        rd_sensor_initialize (p_sensor);
        p_sensor->name = name;

        if (NULL == p_sensor->p_ctx)
        {
            if (RD_BUS_SPI == bus)
            {
                dps310_singleton_spi_setup (p_sensor, handle);
            }
            else if (RD_BUS_I2C == bus)
            {
                err_code |= RD_ERROR_NOT_IMPLEMENTED;
            }
            else
            {
                err_code |= RD_ERROR_NOT_SUPPORTED;
            }
        }

        if (RD_SUCCESS == err_code)
        {
            dps_status = dps310_init (p_sensor->p_ctx);

            if (DPS310_SUCCESS == dps_status)
            {
                dps310_fp_setup (p_sensor);
                p_sensor->name = name;
                p_sensor->provides = dps_fields;
                memset (&last_data, 0, sizeof (last_data));
            }
            else
            {
                err_code |= RD_ERROR_NOT_FOUND;
            }
        }
    }

    return err_code;
}

rd_status_t ri_dps310_uninit (rd_sensor_t * sensor, rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == sensor)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        // Error code can be ignored.
        (void) dps310_uninit (sensor->p_ctx);
        rd_sensor_uninitialize (sensor);
    }

    return err_code;
}

rd_status_t ri_dps310_samplerate_set (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;
    dps310_status_t dps_status = DPS310_SUCCESS;

    if (NULL == samplerate)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (DPS310_READY != singleton_ctx_spi.device_status)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (RD_SENSOR_CFG_NO_CHANGE == *samplerate)
    {
        // No action needed.
    }
    else if (RD_SENSOR_CFG_DEFAULT == *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_DEFAULT_MR,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_DEFAULT_MR,
                                          singleton_ctx_spi.pres_osr);
    }
    else if ( (RD_SENSOR_CFG_MIN == *samplerate) || (1U == *samplerate))
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_1,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_1,
                                          singleton_ctx_spi.pres_osr);
    }
    else if (RD_SENSOR_CFG_MAX == *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_128,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_128,
                                          singleton_ctx_spi.pres_osr);
    }
    else if (2U == *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_2,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_2,
                                          singleton_ctx_spi.pres_osr);
    }
    else if (4U >= *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_4,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_4,
                                          singleton_ctx_spi.pres_osr);
    }
    else if (8U >= *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_8,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_8,
                                          singleton_ctx_spi.pres_osr);
    }
    else if (16U >= *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_16,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_16,
                                          singleton_ctx_spi.pres_osr);
    }
    else if (32U >= *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_32,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_32,
                                          singleton_ctx_spi.pres_osr);
    }
    else if (64U >= *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_64,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_64,
                                          singleton_ctx_spi.pres_osr);
    }
    else if (128U >= *samplerate)
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          DPS310_MR_128,
                                          singleton_ctx_spi.temp_osr);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          DPS310_MR_128,
                                          singleton_ctx_spi.pres_osr);
    }
    else
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }

    if ( (DPS310_SUCCESS == dps_status) && (RD_SUCCESS == err_code))
    {
        err_code |= ri_dps310_samplerate_get (samplerate);
    }
    else if (RD_SUCCESS == err_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        // No action needed.
    }

    return err_code;
}

static uint8_t dps310_mr_to_samplerate (const dps310_mr_t mr)
{
    uint8_t rate;

    switch (mr)
    {
        case DPS310_MR_1:
            rate = 1U;
            break;

        case DPS310_MR_2:
            rate = 2U;
            break;

        case DPS310_MR_4:
            rate = 4U;
            break;

        case DPS310_MR_8:
            rate = 8U;
            break;

        case DPS310_MR_16:
            rate = 16U;
            break;

        case DPS310_MR_32:
            rate = 32U;
            break;

        case DPS310_MR_64:
            rate = 64U;
            break;

        case DPS310_MR_128:
            rate = 128U;
            break;

        default:
            rate = 0U;
            break;
    }

    return rate;
}

rd_status_t ri_dps310_samplerate_get (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t rate = 0U;

    if (NULL == samplerate)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        // Separate rates for temperature and pressure aren't supported
        // by interface, so can check only temperature rate.
        rate = dps310_mr_to_samplerate (singleton_ctx_spi.temp_mr);

        if (0U == rate)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            *samplerate = rate;
        }
    }

    return err_code;
}

rd_status_t ri_dps310_resolution_set (uint8_t * resolution)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == resolution)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (DPS310_READY != singleton_ctx_spi.device_status)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if ( (RD_SENSOR_CFG_DEFAULT == *resolution)
              || (RD_SENSOR_CFG_MIN == *resolution)
              || (RD_SENSOR_CFG_MAX == *resolution)
              || (RD_SENSOR_CFG_NO_CHANGE == *resolution))
    {
        // No action needed.
    }
    else
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }

    err_code |= ri_dps310_resolution_get (resolution);
    return err_code;
}

rd_status_t ri_dps310_resolution_get (uint8_t * resolution)
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

rd_status_t ri_dps310_scale_set (uint8_t * scale)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == scale)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (DPS310_READY != singleton_ctx_spi.device_status)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if ( (RD_SENSOR_CFG_DEFAULT == *scale)
              || (RD_SENSOR_CFG_MIN == *scale)
              || (RD_SENSOR_CFG_MAX == *scale)
              || (RD_SENSOR_CFG_NO_CHANGE == *scale))
    {
        // No action needed.
    }
    else
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }

    err_code |= ri_dps310_scale_get (scale);
    return err_code;
}

rd_status_t ri_dps310_scale_get (uint8_t * scale)
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

rd_status_t ri_dps310_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;
    dps310_status_t dps_status = DPS310_SUCCESS;

    if ( (NULL == dsp) || (NULL == parameter))
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (DPS310_READY != singleton_ctx_spi.device_status)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if ( (RD_SENSOR_DSP_LAST == *dsp)
              || (RD_SENSOR_CFG_DEFAULT == *dsp))
    {
        dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                          singleton_ctx_spi.temp_mr,
                                          DPS310_OS_1);
        dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                          singleton_ctx_spi.pres_mr,
                                          DPS310_OS_1);
    }
    else if (RD_SENSOR_DSP_OS == *dsp)
    {
        if (RD_SENSOR_CFG_NO_CHANGE == *parameter)
        {
            // No action needed.
        }
        else if (RD_SENSOR_CFG_DEFAULT == *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_DEFAULT_OS);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_DEFAULT_OS);
        }
        else if ( (RD_SENSOR_CFG_MIN == *parameter) || (1U == *parameter))
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_1);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_1);
        }
        else if (RD_SENSOR_CFG_MAX == *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_128);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_128);
        }
        else if (2U == *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_2);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_2);
        }
        else if (4U >= *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_4);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_4);
        }
        else if (8U >= *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_8);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_8);
        }
        else if (16U >= *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_16);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_16);
        }
        else if (32U >= *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_32);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_32);
        }
        else if (64U >= *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_64);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_64);
        }
        else if (128U >= *parameter)
        {
            dps_status |= dps310_config_temp (&singleton_ctx_spi,
                                              singleton_ctx_spi.temp_mr,
                                              DPS310_OS_128);
            dps_status |= dps310_config_pres (&singleton_ctx_spi,
                                              singleton_ctx_spi.pres_mr,
                                              DPS310_OS_128);
        }
        else
        {
            err_code |= RD_ERROR_NOT_SUPPORTED;
        }
    }

    if (DPS310_SUCCESS == dps_status)
    {
        err_code |= ri_dps310_dsp_get (dsp, parameter);
    }
    else
    {
        err_code |= RD_ERROR_INTERNAL;
    }

    return err_code;
}

static uint8_t dps310_os_to_samplerate (const dps310_os_t os)
{
    uint8_t rate;

    switch (os)
    {
        case DPS310_OS_1:
            rate = 1U;
            break;

        case DPS310_OS_2:
            rate = 2U;
            break;

        case DPS310_OS_4:
            rate = 4U;
            break;

        case DPS310_OS_8:
            rate = 8U;
            break;

        case DPS310_OS_16:
            rate = 16U;
            break;

        case DPS310_OS_32:
            rate = 32U;
            break;

        case DPS310_OS_64:
            rate = 64U;
            break;

        case DPS310_OS_128:
            rate = 128U;
            break;

        default:
            rate = 0U;
            break;
    }

    return rate;
}

rd_status_t ri_dps310_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t sampling = 0;

    // Separate rates for temperature and pressure aren't supported
    // by interface, so can check only temperature rate.
    if ( (NULL == dsp) || (NULL == parameter))
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (singleton_ctx_spi.temp_osr > DPS310_OS_1)
    {
        sampling = dps310_os_to_samplerate (singleton_ctx_spi.temp_osr);

        if (sampling > 1)
        {
            *dsp = RD_SENSOR_DSP_OS;
            *parameter = sampling;
        }
        else
        {
            err_code = RD_ERROR_INTERNAL;
        }
    }
    else if (singleton_ctx_spi.temp_osr == DPS310_OS_1)
    {
        *dsp = RD_SENSOR_CFG_DEFAULT;
        *parameter = 1U;
    }
    else
    {
        err_code = RD_ERROR_INTERNAL;
    }

    return err_code;
}

static void last_sample_update (const float temp, const float pres, const uint64_t ts)
{
    memset (&last_data, 0, sizeof (last_data));
    last_data.fields = dps_fields;
    last_data.data = last_values;
    last_data.timestamp_ms = ts;
    rd_sensor_data_set (&last_data, RD_SENSOR_PRES_FIELD, pres);
    rd_sensor_data_set (&last_data, RD_SENSOR_TEMP_FIELD, temp);
}

rd_status_t ri_dps310_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;
    dps310_status_t dps_status = DPS310_SUCCESS;

    if (NULL == mode)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if ( (RD_SENSOR_CFG_SLEEP == *mode)
              || (RD_SENSOR_CFG_DEFAULT == *mode))
    {
        dps_status = dps310_standby (&singleton_ctx_spi);
    }
    else if (DPS310_READY != singleton_ctx_spi.device_status)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (RD_SENSOR_CFG_SINGLE == *mode)
    {
        float temperature;
        float pressure;
        dps_status |= dps310_measure_temp_once_sync (&singleton_ctx_spi, &temperature);
        dps_status |= dps310_measure_pres_once_sync (&singleton_ctx_spi, &pressure);
        last_sample_update (temperature, pressure, rd_sensor_timestamp_get());
    }
    else if (RD_SENSOR_CFG_CONTINUOUS == *mode)
    {
        dps_status |= dps310_measure_continuous_async (&singleton_ctx_spi);
    }
    else
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }

    if (DPS310_SUCCESS != dps_status)
    {
        err_code |= RD_ERROR_INTERNAL;
    }

    err_code |= ri_dps310_mode_get (mode);
    return err_code;
}

rd_status_t ri_dps310_mode_get (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        switch (singleton_ctx_spi.device_status)
        {
            case DPS310_READY:
                *mode = RD_SENSOR_CFG_SLEEP;
                break;

            case DPS310_CONTINUOUS:
                *mode = RD_SENSOR_CFG_CONTINUOUS;
                break;

            default:
                err_code |= RD_ERROR_INVALID_STATE;
        }
    }

    return err_code;
}

rd_status_t ri_dps310_data_get (rd_sensor_data_t * const data)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_DEFAULT;
    (void) ri_dps310_mode_get (&mode);

    if (NULL == data)
    {
        err_code |= RD_ERROR_NULL;
    }
    // Use cached last value if sensor is sleeping, verify there is a valid sample.
    else if ( (mode == RD_SENSOR_CFG_SLEEP) && (last_data.timestamp_ms > 0))
    {
        rd_sensor_data_populate (data, &last_data, dps_fields);
    }
    else if (mode == RD_SENSOR_CFG_CONTINUOUS)
    {
        float temperature;
        float pressure;
        dps310_status_t dps_status = dps310_get_last_result (&singleton_ctx_spi,
                                     &temperature, &pressure);
        last_sample_update (temperature, pressure, rd_sensor_timestamp_get());
        rd_sensor_data_populate (data, &last_data, dps_fields);

        if (DPS310_SUCCESS != dps_status)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
    }
    else
    {
        // No action needed.
    }

    return err_code;
}

#endif
