#ifndef RUUVI_INTERFACE_MMC5616WA_H
#define RUUVI_INTERFACE_MMC5616WA_H

#include "ruuvi_driver_enabled_modules.h"
#if (RI_MMC5616WA_ENABLED || DOXYGEN)

#include "mmc5616wa.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

#define RI_MMC5616WA_DEFAULT_SAMPLERATE (1U)
#define RI_MMC5616WA_MAX_SAMPLERATE     (200U)
#define RI_MMC5616WA_RESOLUTION_BITS    (20U)
#define RI_MMC5616WA_SCALE_GAUSS        (30U)

typedef struct
{
    uint8_t handle;
    uint8_t mode;
    uint64_t tsample;
    mmc5616wa_ctx_t ctx;
    mmc5616wa_config_t config;
    mmc5616wa_fifo_config_t fifo_config;
    float values[4];
} ri_mmc5616wa_dev_t;

rd_status_t ri_mmc5616wa_init (rd_sensor_t * p_sensor, rd_bus_t bus,
                               uint8_t handle);
rd_status_t ri_mmc5616wa_uninit (rd_sensor_t * p_sensor, rd_bus_t bus,
                                 uint8_t handle);
rd_status_t ri_mmc5616wa_samplerate_set (uint8_t * samplerate);
rd_status_t ri_mmc5616wa_samplerate_get (uint8_t * samplerate);
rd_status_t ri_mmc5616wa_resolution_set (uint8_t * resolution);
rd_status_t ri_mmc5616wa_resolution_get (uint8_t * resolution);
rd_status_t ri_mmc5616wa_scale_set (uint8_t * scale);
rd_status_t ri_mmc5616wa_scale_get (uint8_t * scale);
rd_status_t ri_mmc5616wa_dsp_set (uint8_t * dsp, uint8_t * parameter);
rd_status_t ri_mmc5616wa_dsp_get (uint8_t * dsp, uint8_t * parameter);
rd_status_t ri_mmc5616wa_mode_set (uint8_t * mode);
rd_status_t ri_mmc5616wa_mode_get (uint8_t * mode);
rd_status_t ri_mmc5616wa_data_get (rd_sensor_data_t * const data);
rd_status_t ri_mmc5616wa_fifo_use (const bool enable);
rd_status_t ri_mmc5616wa_fifo_interrupt_use (const bool enable);
rd_status_t ri_mmc5616wa_fifo_read (size_t * num_elements,
                                    rd_sensor_data_t * data);

#ifdef CEEDLING
extern ri_mmc5616wa_dev_t dev;
#endif


#endif
#endif