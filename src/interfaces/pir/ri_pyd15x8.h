#ifndef RUUVI_INTERFACE_PYD15X8_H
#define RUUVI_INTERFACE_PYD15X8_H

#include "ruuvi_driver_enabled_modules.h"
#if (RI_PYD15X8_ENABLED || DOXYGEN)

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

#include <stdbool.h>
#include <stddef.h>
#include "ri_pyd15x8_reg.h" 

/**
 * @addtogroup Motion
 * @{
 */

/**
 * @defgroup PYD15X8 PYD15X8 Interface
 * @brief Implement @ref rd_sensor_t functions on PYD15X8.
 *
 * This header only exposes the public API for the PYD15X8 PIR sensor.
 * The actual register‑level implementation will
 * follow once @ref pyd15x8_reg.h and the corresponding .c file are in
 * place. The API is intentionally kept source‑compatible with
 * ri_lis2dh12_*, allowing the application to switch sensors via the
 * RI_*_ENABLED build flag.
 */

/** @brief Convenience alias for "no error" in this driver. */
#define PYD_SUCCESS                      (0U)


/* =========================================================
 * rd_sensor_t interface functions
 * ========================================================= */

rd_status_t ri_pyd15x8_init               (rd_sensor_t * pir,
                                           rd_bus_t      bus,
                                           uint8_t       handle);

rd_status_t ri_pyd15x8_uninit            (rd_sensor_t *  pir,
                                           rd_bus_t      bus,
                                           uint8_t       handle);

rd_status_t ri_pyd15x8_samplerate_set     (uint8_t * samplerate);
rd_status_t ri_pyd15x8_samplerate_get     (uint8_t * samplerate);

rd_status_t ri_pyd15x8_resolution_set     (uint8_t * resolution);
rd_status_t ri_pyd15x8_resolution_get     (uint8_t * resolution);

rd_status_t ri_pyd15x8_scale_set          (uint8_t * scale);
rd_status_t ri_pyd15x8_scale_get          (uint8_t * scale);

rd_status_t ri_pyd15x8_dsp_set            (uint8_t * dsp, uint8_t * parameter);
rd_status_t ri_pyd15x8_dsp_get            (uint8_t * dsp, uint8_t * parameter);

rd_status_t ri_pyd15x8_mode_set           (uint8_t * mode);
rd_status_t ri_pyd15x8_mode_get           (uint8_t * mode);

rd_status_t ri_pyd15x8_data_get           (rd_sensor_data_t * const data);

/* ---------- Activity‑/motion‑detect interrupt ---------- */
rd_status_t ri_pyd15x8_activity_interrupt_use (const bool enable,
                                               float * limit_g);

/* =========================================================
 * Driver state container
 * ========================================================= */
typedef rd_status_t (*clear_interrupt) (pyd15x8_bus_t const * const p_clear);
typedef struct
{
    pyd15x8_bus_t    ctx;         //!< Low‑level read/write context.
    clear_interrupt  clr;
} ri_pyd15x8_dev;

#ifdef CEEDLING
extern ri_pyd15x8_dev dev;
#endif

/** @} */

#endif /* RI_PYD15X8_ENABLED || DOXYGEN */
#endif /* RUUVI_INTERFACE_PYD15X8_H */
