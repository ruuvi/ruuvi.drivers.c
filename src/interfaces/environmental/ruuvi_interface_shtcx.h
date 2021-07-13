#ifndef RUUVI_INTERFACE_SHTCX_H
#define RUUVI_INTERFACE_SHTCX_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

/**
 * @addtogroup Environmental
 */
/*@{*/
/**
 * @defgroup SHTCX SHTCX Interface
 * @brief Implement @ref rd_sensor_t functions on SHTCX
 *
 * The implementation supports taking single-samples and a pseudo-continuous mode
 * by taking a new sample when data is polled in continuous mode
 */
/*@}*/
/**
 * @addtogroup SHTCX
 */
/*@{*/
/**
 * @file ruuvi_interface_shtcx.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-10
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for SHTCX basic usage. The underlying platform must provide
 * functions for I2C access.
 *
 * Testing the interface with @ref ruuvi_driver_sensor_test.h
 *
 * @code{.c}
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 *  rd_status_t err_code = RD_SUCCESS;
 *  rd_bus_t bus = RD_BUS_NONE;
 *  uint8_t handle = 0;
 *  rd_sensor_init_fp init = ri_shtcx_init;
 *  bus = RD_BUS_I2C;
 *  handle = RUUVI_BOARD_RUUVI_BOARD_SHTCX_I2C_ADDRESS;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 * @endcode
 */

#define RI_SHTCX_WAKEUP_US (240U) //!< Time from wakeup cmd to rdy.

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_shtcx_init (rd_sensor_t *
                           environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_shtcx_uninit (rd_sensor_t *
                             environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_dsp_set (uint8_t * dsp, uint8_t * parameter);
rd_status_t ri_shtcx_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_mode_set (uint8_t *);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_shtcx_mode_get (uint8_t *);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_shtcx_data_get (rd_sensor_data_t * const
                               p_data);
/*@}*/
#endif