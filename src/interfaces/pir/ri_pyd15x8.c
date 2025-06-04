#include "ruuvi_driver_enabled_modules.h"
#if (RI_PYD15X8_ENABLED || DOXYGEN)
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_yield.h"
#include "ri_pyd15x8.h"
#include "ri_pyd15x8_reg.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @addtogroup PYD15X8
 */
/*@{*/
/**
 * @file ruuvi_interface_pyd15x8.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2025-05-29
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 * Implementation for PYD15X8 basic usage. Provides motion‑interrupt operation only (Wake‑Up mode).
 *
 * Requires "ruuvi_driver_enabled_modules.h", will only get compiled if RI_PYD15X8_ENABLED is defined.
 *
 */

#ifndef CEEDLING
static
#endif
ri_pyd15x8_dev dev = {0};

#ifndef PYD15X8_DEFAULT_THRESHOLD
#define PYD15X8_DEFAULT_THRESHOLD   50U     /* counts after BPF */
#endif
#ifndef PYD15X8_DEFAULT_BLIND_05S
#define PYD15X8_DEFAULT_BLIND_05S   3U      /* 0.5 s + n·0.5 s → 2.0 s */
#endif
#ifndef PYD15X8_DEFAULT_PULSES
#define PYD15X8_DEFAULT_PULSES      0U      /* 1 pulse */
#endif
#ifndef PYD15X8_DEFAULT_WINDOW_2S
#define PYD15X8_DEFAULT_WINDOW_2S   0U      /* 2 s */
#endif

static const char m_pir_name[] = "PYD15X8";
static uint64_t m_frame = 0; // Debug data

/** @brief Macro for checking "ignored" parameters NO_CHANGE, MIN, MAX, DEFAULT */
#define RETURN_SUCCESS_ON_VALID(param) do {\
            if(RD_SENSOR_CFG_DEFAULT   == param ||\
               RD_SENSOR_CFG_MIN       == param ||\
               RD_SENSOR_CFG_MAX       == param ||\
               RD_SENSOR_CFG_NO_CHANGE == param   \
             ) return RD_SUCCESS;\
           } while(0)

static rd_status_t pyd_verify_present (const pyd15x8_bus_t * bus)
{
    /* There is no WHOAMI; instead read one frame & check the hard‑wired RSVD bits. */
    rd_status_t err = pyd15x8_forced_read (bus, &m_frame);
    if (RD_SUCCESS != err) { return err; }

    /* Bits [4:3] of cfg must equal “10” (see datasheet Table 6). */
    const uint32_t cfg25 = (uint32_t)(m_frame & 0x1FFFFFFU); /* mask 25 bits */
    const bool ok = ((cfg25 & PYD15X8_CFG_RSVD_MASK) == PYD15X8_CFG_RSVD_PATTERN);
    return ok ? RD_SUCCESS : RD_ERROR_NOT_FOUND;
}

static rd_status_t pyd_wakeup_cfg (const pyd15x8_bus_t * bus)
{
    uint32_t cfg = pyd15x8_build_cfg (
        PYD15X8_DEFAULT_THRESHOLD,
        PYD15X8_DEFAULT_BLIND_05S,
        PYD15X8_DEFAULT_PULSES,
        PYD15X8_DEFAULT_WINDOW_2S,
        PYD15X8_MODE_WAKEUP,
        PYD15X8_SRC_PIR_BPF,
        PYD15X8_HPF_0_4_HZ,
        false /* sign change required */);
    return pyd15x8_serin_write (bus, cfg);
}

static rd_status_t pyd_readout_cfg (const pyd15x8_bus_t * bus)
{
    uint32_t cfg = pyd15x8_build_cfg (
        PYD15X8_DEFAULT_THRESHOLD,
        PYD15X8_DEFAULT_BLIND_05S,
        PYD15X8_DEFAULT_PULSES,
        PYD15X8_DEFAULT_WINDOW_2S,
        PYD15X8_MODE_FORCED,
        PYD15X8_SRC_PIR_BPF,
        PYD15X8_HPF_0_4_HZ,
        false /* sign change required */);
    return pyd15x8_serin_write (bus, cfg);
}

/* ────────────────────────────────────────────────────────────────────────── */
/* Public API                                                                 */
/* ────────────────────────────────────────────────────────────────────────── */

rd_status_t ri_pyd15x8_init (rd_sensor_t * const p_sensor,
                             rd_bus_t            bus,
                             uint8_t             handle)
{
    rd_status_t err_code = RD_SUCCESS;
    if (NULL == p_sensor) { return RD_ERROR_NULL; }
    if (rd_sensor_is_init (p_sensor)) { return RD_ERROR_INVALID_STATE; }
    rd_sensor_initialize (p_sensor);
    p_sensor->name                  = m_pir_name;

    /* GPIO must be ready. Caller decides whether to init. */
    if (!ri_gpio_is_init()) { err_code |= ri_gpio_init(); }
    /* Load context if it exists 
       - hacky workaround on sensor API not containing free-form bus */
    if(p_sensor->p_ctx == NULL) { p_sensor->p_ctx = &dev; }
    const pyd15x8_bus_t * hw = &((const ri_pyd15x8_dev *)p_sensor->p_ctx)->ctx;

    /* Forced readout config to check for presence*/
    if (RD_SUCCESS == err_code) { err_code |= pyd_readout_cfg (hw); }

    /* Give config time to latch (datasheet tSLT + 2.4 ms) */
    if (RD_SUCCESS == err_code) { err_code |= ri_delay_ms (3); } 

    /* Basic presence check */
    if (RD_SUCCESS == err_code) { err_code |= pyd_verify_present (hw); }

    /* Wakeup config */
    if (RD_SUCCESS == err_code) { err_code |= pyd_wakeup_cfg (hw); }

    /* Give config time to latch (datasheet tSLT + 2.4 ms) */
    if (RD_SUCCESS == err_code) { err_code |= ri_delay_ms (3); } 

    /* Expose minimal sensor API */
    if (RD_SUCCESS == err_code)
    {
        p_sensor->init                  = ri_pyd15x8_init;
        p_sensor->uninit                = ri_pyd15x8_uninit;
        p_sensor->samplerate_set        = ri_pyd15x8_samplerate_set;
        p_sensor->samplerate_get        = ri_pyd15x8_samplerate_get;
        p_sensor->resolution_set        = ri_pyd15x8_resolution_set;
        p_sensor->resolution_get        = ri_pyd15x8_resolution_get;
        p_sensor->scale_set             = ri_pyd15x8_scale_set;
        p_sensor->scale_get             = ri_pyd15x8_scale_get;
        p_sensor->dsp_set               = ri_pyd15x8_dsp_set;
        p_sensor->dsp_get               = ri_pyd15x8_dsp_get;
        p_sensor->mode_set              = ri_pyd15x8_mode_set;
        p_sensor->mode_get              = ri_pyd15x8_mode_get;
        p_sensor->data_get              = ri_pyd15x8_data_get;
        p_sensor->configuration_set     = rd_sensor_configuration_set;
        p_sensor->configuration_get     = rd_sensor_configuration_get;
        p_sensor->level_interrupt_set   = ri_pyd15x8_activity_interrupt_use;

        /* Data provided – just motion boolean via level interrupt, no numerical values. */
        memset (&p_sensor->provides, 0, sizeof (p_sensor->provides));
        /* Store bus context in sensor */
        memcpy(&dev.ctx, p_sensor->p_ctx, sizeof(dev.ctx)); 
        dev.clr = &pyd15x8_clear_interrupt;
    }
    else
    {
        rd_sensor_uninitialize (p_sensor);
    }
    return err_code;
}

rd_status_t ri_pyd15x8_uninit (rd_sensor_t * const p_sensor,
                               rd_bus_t            bus,
                               uint8_t             handle)
{
    (void)bus; (void)handle;
    rd_status_t err_code = RD_SUCCESS;
    if (NULL == p_sensor) { return RD_ERROR_NULL; }

    /* Pull DL low for ≥tUP to ensure quiet sensor, then Set SERIN/DL High‑Z */
    const pyd15x8_bus_t * hw = &((const ri_pyd15x8_dev *)p_sensor->p_ctx)->ctx;
    if (hw)
    {
        err_code |= ri_gpio_configure (hw->pin_dl, RI_GPIO_MODE_OUTPUT_STANDARD);
        err_code |= ri_gpio_write    (hw->pin_dl, RI_GPIO_LOW);
        err_code |= ri_delay_ms (2); /* 2 ms > tUP */
        err_code |= ri_gpio_configure (hw->pin_dl, RI_GPIO_MODE_HIGH_Z);
        err_code |= ri_gpio_configure (hw->pin_serin, RI_GPIO_MODE_HIGH_Z);
    }

    rd_sensor_uninitialize (p_sensor);
    return err_code;
}

rd_status_t ri_pyd15x8_samplerate_get(uint8_t* samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    *samplerate = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_pyd15x8_samplerate_set(uint8_t* samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    uint8_t original = *samplerate;
    *samplerate = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID(original);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_pyd15x8_resolution_get(uint8_t* resolution)
{
    if (NULL == resolution) { return RD_ERROR_NULL; }

    *resolution = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_pyd15x8_resolution_set(uint8_t* resolution)
{
    if (NULL == resolution) { return RD_ERROR_NULL; }

    uint8_t original = *resolution;
    *resolution = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID(original);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_pyd15x8_scale_get(uint8_t* scale)
{
    if (NULL == scale) { return RD_ERROR_NULL; }

    *scale = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_pyd15x8_scale_set(uint8_t* scale)
{
    if (NULL == scale) { return RD_ERROR_NULL; }

    uint8_t original = *scale;
    *scale = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID(original);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_pyd15x8_dsp_get(uint8_t* dsp, uint8_t* parameter)
{
    if (NULL == dsp || NULL == parameter) { return RD_ERROR_NULL; }

    *dsp = RD_SENSOR_CFG_DEFAULT;
    *parameter = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_pyd15x8_dsp_set(uint8_t* dsp, uint8_t* parameter)
{
    if (NULL == dsp || NULL == parameter) { return RD_ERROR_NULL; }

    uint8_t original_dsp = *dsp;
    uint8_t original_param = *parameter;
    *dsp = RD_SENSOR_CFG_DEFAULT;
    *parameter = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID(original_dsp);
    RETURN_SUCCESS_ON_VALID(original_param);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_pyd15x8_mode_get(uint8_t* mode)
{
    if (NULL == mode) { return RD_ERROR_NULL; }

    *mode = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_pyd15x8_mode_set(uint8_t* mode)
{
    if (NULL == mode) { return RD_ERROR_NULL; }

    uint8_t original = *mode;
    *mode = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID(original);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_pyd15x8_data_get(rd_sensor_data_t* data)
{
    (void)data;
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_pyd15x8_activity_interrupt_use(const bool enable, float* sensitivity)
{
    if (NULL == sensitivity) { return RD_ERROR_NULL; }
    /* Sensitivity control via threshold field (counts) */
    float s = *sensitivity;
    if (s < 0.0f) { s = 0.0f; }
    if (s > 1.0f) { s = 1.0f; }
    uint8_t thresh = (uint8_t)(s * 255.0f);
    if (thresh > 0xFFU) { return RD_ERROR_INVALID_PARAM; }
    uint32_t cfg = pyd15x8_build_cfg(
        thresh,
        PYD15X8_DEFAULT_BLIND_05S,
        PYD15X8_DEFAULT_PULSES,
        PYD15X8_DEFAULT_WINDOW_2S,
        enable ? PYD15X8_MODE_WAKEUP : PYD15X8_MODE_FORCED,
        PYD15X8_SRC_PIR_BPF,
        PYD15X8_HPF_0_4_HZ,
        false);
    rd_status_t err = pyd15x8_serin_write(&dev.ctx, cfg);
    if (RD_SUCCESS == err)
    {
        /* Report back actual threshold */
        *sensitivity = (float)thresh;
        /* Clear possible pending interrupt */
        dev.clr(&dev.ctx);
    }
    return err;
}
#endif