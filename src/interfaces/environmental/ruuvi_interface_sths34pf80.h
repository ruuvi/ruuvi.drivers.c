#ifndef RUUVI_INTERFACE_STHS34PF80_H
#define RUUVI_INTERFACE_STHS34PF80_H
#include "ruuvi_driver_enabled_modules.h"
#if RI_STHS34PF80_ENABLED || DOXYGEN
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "sths34pf80_reg.h"

/**
 * @addtogroup Environmental
 */
/** @{ */
/**
 * @defgroup STHS34PF80 STHS34PF80 Interface
 * @brief Implement @ref rd_sensor_t functions on STHS34PF80
 *
 * The STHS34PF80 is a thermal infrared presence/motion sensor.
 * This implementation supports:
 * - Ambient temperature
 * - Presence detection (boolean flag + raw signal)
 * - Motion detection (boolean flag + raw signal)
 * - IR object signal (dimensionless)
 */
/** @} */
/**
 * @addtogroup STHS34PF80
 */
/** @{ */
/**
 * @file ruuvi_interface_sths34pf80.h
 * @author Ruuvi Innovations Ltd
 * @date 2025-12-22
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for STHS34PF80 thermal infrared presence sensor.
 * The underlying platform must provide functions for I2C access.
 *
 * Testing the interface with @ref ruuvi_driver_sensor_test.h
 *
 * @code{.c}
 *  rd_status_t err_code = RD_SUCCESS;
 *  rd_bus_t bus = RD_BUS_I2C;
 *  uint8_t handle = 0x5A;  // I2C address
 *  rd_sensor_init_fp init = ri_sths34pf80_init;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 * @endcode
 */

#define STHS34PF80_I2C_ADDR_DEFAULT (0x5AU) //!< Default I2C address (7-bit)
// Note: WHO_AM_I value (0xD3) is defined as STHS34PF80_ID in sths34pf80_reg.h

/**
 * @brief STHS34PF80-specific sample rate values for sub-1Hz operation.
 *
 * Use these with ri_sths34pf80_samplerate_set() for low-power sub-1Hz sampling.
 * Standard integer values 1, 2, 4, 8, 15, 30 Hz are also supported.
 */
#define RI_STHS34PF80_SAMPLERATE_0HZ25  RD_SENSOR_CFG_CUSTOM_1 //!< 0.25 Hz (4 second period)
#define RI_STHS34PF80_SAMPLERATE_0HZ50  RD_SENSOR_CFG_CUSTOM_2 //!< 0.50 Hz (2 second period)

/**
 * @brief Default algorithm configuration values from ST reference example.
 * @{
 */
#define RI_STHS34PF80_AVG_TMOS_DEFAULT          STHS34PF80_AVG_TMOS_32  //!< TMOS averaging: 32 samples
#define RI_STHS34PF80_AVG_TAMB_DEFAULT          STHS34PF80_AVG_T_8     //!< Tambient averaging: 8 samples
#define RI_STHS34PF80_TAMB_SHOCK_THS_DEFAULT    (35U)   //!< Ambient shock threshold [LSB]
#define RI_STHS34PF80_TAMB_SHOCK_HYS_DEFAULT    (5U)    //!< Ambient shock hysteresis [LSB]
#define RI_STHS34PF80_PRESENCE_THS_DEFAULT      (200U)  //!< Presence threshold [LSB]
#define RI_STHS34PF80_PRESENCE_HYS_DEFAULT      (20U)   //!< Presence hysteresis [LSB]
#define RI_STHS34PF80_MOTION_THS_DEFAULT        (300U)  //!< Motion threshold [LSB]
#define RI_STHS34PF80_MOTION_HYS_DEFAULT        (30U)   //!< Motion hysteresis [LSB]
/** @} */

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_sths34pf80_init (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_sths34pf80_uninit (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sths34pf80_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sths34pf80_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sths34pf80_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sths34pf80_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sths34pf80_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sths34pf80_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_sths34pf80_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_sths34pf80_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sths34pf80_mode_set (uint8_t * mode);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sths34pf80_mode_get (uint8_t * mode);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_sths34pf80_data_get (rd_sensor_data_t * const data);

/**
 * @brief Configure STHS34PF80 algorithm parameters with recommended defaults.
 *
 * Applies averaging, threshold, and hysteresis settings from ST reference example.
 * This should be called after init and before starting continuous mode.
 * Sensor must be in sleep mode.
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_INVALID_STATE if sensor is not in sleep mode.
 * @return RD_ERROR_INTERNAL on driver error.
 */
rd_status_t ri_sths34pf80_configure_defaults (void);

/** @} */
#endif // RI_STHS34PF80_ENABLED
#endif // RUUVI_INTERFACE_STHS34PF80_H
