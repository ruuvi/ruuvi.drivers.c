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
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_samplerate_get (uint8_t * samplerate)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_resolution_set (uint8_t * resolution)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_resolution_get (uint8_t * resolution)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_scale_set (uint8_t * scale)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_scale_get (uint8_t * scale)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

rd_status_t ri_dps310_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    return RD_ERROR_NOT_IMPLEMENTED;
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
