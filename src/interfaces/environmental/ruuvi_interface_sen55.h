#ifndef RUUVI_INTERFACE_SEN55_H
#define RUUVI_INTERFACE_SEN55_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

/**
 * @addtogroup Environmental
 */
/*@{*/
/**
 * @defgroup SEN55 SEN55 Interface
 * @brief Implement @ref rd_sensor_t functions on SEN55
 *
 * The implementation supports taking single-samples and a pseudo-continuous mode
 * by taking a new sample when data is polled in continuous mode
 */
/*@}*/
/**
 * @addtogroup SEN55
 */
/*@{*/
/**
 * @file ruuvi_interface_sen55.h
 * @author TheSomeMan
 * @date 2023-05-21
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for SEN55 basic usage. The underlying platform must provide
 * functions for I2C access.
 *
 * Testing the interface with @ref ruuvi_driver_sensor_test.h
 *
 * @code{.c}
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 *  rd_status_t err_code = RD_SUCCESS;
 *  rd_bus_t bus = RD_BUS_NONE;
 *  uint8_t handle = 0;
 *  rd_sensor_init_fp init = ri_sen55_init;
 *  bus = RD_BUS_I2C;
 *  handle = RB_SEN55_I2C_ADDRESS;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 * @endcode
 */

// #define RI_SEN55_WAKEUP_US (240U) //!< Time from wakeup cmd to rdy.

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_sen55_init (rd_sensor_t * sensor, rd_bus_t bus, uint8_t handle);

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_sen55_uninit (rd_sensor_t * sensor, rd_bus_t bus, uint8_t handle);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_samplerate_set (uint8_t * samplerate);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_samplerate_get (uint8_t * samplerate);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_resolution_set (uint8_t * resolution);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_resolution_get (uint8_t * resolution);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_scale_set (uint8_t * scale);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_scale_get (uint8_t * scale);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_dsp_set (uint8_t * dsp, uint8_t * parameter);

rd_status_t ri_sen55_dsp_get (uint8_t * dsp, uint8_t * parameter);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_mode_set (uint8_t * mode);

/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_sen55_mode_get (uint8_t * mode);

/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_sen55_data_get (rd_sensor_data_t * const p_data);

/*@}*/
#endif