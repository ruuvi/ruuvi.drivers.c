#include "ruuvi_interface_dps310.h"
#include "dps310.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_spi_dps310.h"
#include "ruuvi_interface_yield.h"
#include <stdint.h>

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
static uint8_t singleton_comm_ctx;
static const char * const name = "DPS310";

static __attribute__ ( (nonnull)) rd_status_t
dps310_singleton_setup (rd_sensor_t * const p_sensor,
                        const rd_bus_t bus,
                        const uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    if (RD_BUS_SPI == bus)
    {
        p_sensor->p_ctx = &singleton_ctx_spi;
        spi_comm_handle = handle;
    }
    else if (RD_BUS_I2C == bus)
    {
        err_code |= RD_ERROR_NOT_IMPLEMENTED;
    }
    else
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }

    return err_code;
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
        if (NULL == p_sensor->p_ctx)
        {
            dps310_singleton_setup (p_sensor, bus, handle);
        }

        if (RD_SUCCESS == err_code)
        {
            dps_status = dps310_init (p_sensor->p_ctx);

            if (DPS310_SUCCESS == dps_status)
            {
                rd_sensor_initialize (p_sensor);
                dps310_fp_setup (p_sensor);
                p_sensor->name = name;
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
    dps310_status_t dps_status = DPS310_SUCCESS;

    if (NULL == sensor)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        // Error code can be ignored.
        (void) dps310_uninit (sensor->p_ctx);
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

    if (DPS310_SUCCESS == dps_status)
    {
        err_code |= ri_dps310_samplerate_get (samplerate);
    }
    else
    {
        err_code |= RD_ERROR_INTERNAL;
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
    }

    if (0U == rate)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        *samplerate = rate;
    }

    return err_code;
}

rd_status_t ri_dps310_resolution_set (uint8_t * resolution)
{
    rd_status_t err_code = RD_SUCCESS;
    dps310_status_t dps_status = DPS310_SUCCESS;

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
        *resolution = RD_SENSOR_CFG_DEFAULT;
    }
    else
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }

    return err_code;
}

rd_status_t ri_dps310_resolution_get (uint8_t * resolution)
{
    rd_status_t err_code = RD_SUCCESS;
    dps310_status_t dps_status = DPS310_SUCCESS;

    if (NULL == resolution)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (DPS310_READY != singleton_ctx_spi.device_status)
    {
        err_code |= RD_ERROR_INVALID_STATE;
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
    dps310_status_t dps_status = DPS310_SUCCESS;

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
        *scale = RD_SENSOR_CFG_DEFAULT;
    }
    else
    {
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }

    return err_code;
}

rd_status_t ri_dps310_scale_get (uint8_t * scale)
{
    rd_status_t err_code = RD_SUCCESS;
    dps310_status_t dps_status = DPS310_SUCCESS;

    if (NULL == scale)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (DPS310_READY != singleton_ctx_spi.device_status)
    {
        err_code |= RD_ERROR_INVALID_STATE;
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

rd_status_t ri_dps310_mode_set (uint8_t * mode)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_mode_get (uint8_t * mode)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_data_get (rd_sensor_data_t * const data)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}
