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
 * @brief Implement @ref ruuvi_driver_sensor_t functions on SHTCX
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
 * functions for I2C access, @ref ruuvi_interface_i2c_shtxc.h.
 *
 * Testing the interface with @ref test_sensor.h
 *
 * @code{.c}
 *  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_ERROR_SELFTEST);
 *  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
 *  ruuvi_driver_bus_t bus = RUUVI_DRIVER_BUS_NONE;
 *  uint8_t handle = 0;
 *  ruuvi_driver_sensor_init_fp init = ruuvi_interface_shtcx_init;
 *  bus = RUUVI_DRIVER_BUS_I2C;
 *  handle = RUUVI_BOARD_RUUVI_BOARD_SHTCX_I2C_ADDRESS;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_ERROR_SELFTEST);
 * @endcode
 */
/** @brief @ref ruuvi_driver_sensor_init_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_init(ruuvi_driver_sensor_t*
    environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle);
/** @brief @ref ruuvi_driver_sensor_init_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_uninit(ruuvi_driver_sensor_t*
    environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_samplerate_set(uint8_t* samplerate);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_samplerate_get(uint8_t* samplerate);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_resolution_set(uint8_t* resolution);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_resolution_get(uint8_t* resolution);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_scale_set(uint8_t* scale);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_scale_get(uint8_t* scale);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_dsp_set(uint8_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_shtcx_dsp_get(uint8_t* dsp, uint8_t* parameter);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_mode_set(uint8_t*);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_mode_get(uint8_t*);
/** @brief @ref ruuvi_driver_sensor_data_fp */
ruuvi_driver_status_t ruuvi_interface_shtcx_data_get(ruuvi_driver_sensor_data_t* const
    p_data);
/*@}*/
#endif