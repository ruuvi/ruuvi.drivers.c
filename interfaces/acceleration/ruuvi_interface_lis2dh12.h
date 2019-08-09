#ifndef RUUVI_INTERFACE_LIS2DH12_H
#define RUUVI_INTERFACE_LIS2DH12_H
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ACCELERATION_LIS2DH12_ENABLED || DOXYGEN
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

#include <stdbool.h>
#include <stddef.h>
#include <stddef.h>

/**
 * @addtogroup Acceleration
 */
/*@{*/
/**
 * @defgroup LIS2DH12 LIS2DH12 Inteface
 * @brief Implement @ref ruuvi_driver_sensor_t functions on LIS2DH12
 *
 * The implementation supports
 * different resolutions, samplerates, high-passing, activity interrupt
 * and FIFO. 
 */
/*@}*/
/**
 * @addtogroup LIS2DH12
 */
/*@{*/
/**
 * @file ruuvi_interface_lis2dh12.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-08
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for LIS2DH12 basic usage. The underlying platform must provide
 * functions for SPI and/or I2C access, @ref ruuvi_interface_spi_lis2dh12.h.
 *
 * Testing the interface with @ref test_sensor.h
 *
 * @code{.c}
 *  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_ERROR_SELFTEST);
 *  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
 *  ruuvi_driver_bus_t bus = RUUVI_DRIVER_BUS_NONE;
 *  uint8_t handle = 0;
 *  ruuvi_driver_sensor_init_fp init = ruuvi_interface_lis2dh12_init;
 *  bus = RUUVI_DRIVER_BUS_SPI;
 *  handle = RUUVI_BOARD_SPI_SS_ACCELEROMETER_PIN;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_ERROR_SELFTEST);
 * @endcode
 */

/** @brief Minimum counts of self-test change in 10 bit resolution 2 G scale, ref datasheet.*/
#define RUUVI_INTERFACE_LIS2DH12_SELFTEST_DIFF_MIN 17
/** @brief Maximum counts of self-test change in 10 bit resolution 2 G scale, ref datasheet.*/
#define RUUVI_INTERFACE_LIS2DH12_SELFTEST_DIFF_MAX 360
/** @brief Scale used on "default" setting. */
#define RUUVI_INTERFACE_LIS2DH12_DEFAULT_SCALE 2
/** @brief Resolution used on "default" setting. */
#define RUUVI_INTERFACE_LIS2DH12_DEFAULT_RESOLUTION 10

/** @brief @ref ruuvi_driver_sensor_init_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_init(ruuvi_driver_sensor_t*
    acceleration_sensor, ruuvi_driver_bus_t bus, uint8_t handle);
/** @brief @ref ruuvi_driver_sensor_init_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_uninit(ruuvi_driver_sensor_t*
    acceleration_sensor, ruuvi_driver_bus_t bus, uint8_t handle);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_samplerate_set(uint8_t* samplerate);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_samplerate_get(uint8_t* samplerate);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_resolution_set(uint8_t* resolution);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_resolution_get(uint8_t* resolution);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_scale_set(uint8_t* scale);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_scale_get(uint8_t* scale);
/** @brief @ref ruuvi_driver_sensor_dsp_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_dsp_set(uint8_t* dsp, uint8_t* parameter);
/** @brief @ref ruuvi_driver_sensor_dsp_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_dsp_get(uint8_t* dsp, uint8_t* parameter);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_mode_set(uint8_t*);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_mode_get(uint8_t*);
/** @brief @ref ruuvi_driver_sensor_data_fp */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_data_get(void* data);

/**
* @brief Enable 32-level FIFO in LIS2DH12
* If FIFO is enabled, values are stored on LIS2DH12 FIFO and oldest element is returned on data read.
*
* @param[in] enable true to enable FIFO, false to disable or reset FIFO.
* @return RUUVI_DRIVER_SUCCESS on success, error code from stack on error.
*/
ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_use(const bool enable);

/**
* @brief Read FIFO
* Reads up to num_elements data points from FIFO and populates pointer data with them
*
* @param[in, out] num_elements Input: number of elements in data. Output: Number of elements placed in data
* @param[out] data array of ruuvi_interface_acceleration_data_t with num_elements slots.
* @param RUUVI_DRIVER_SUCCESS on success
* @param RUUVI_DRIVER_ERROR_NULL if either parameter is NULL
* @param RUUVI_DRIVER_ERROR_INVALID_STATE if FIFO is not in use
* @param error code from stack on error.
*/
ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_read(size_t* num_elements,
    ruuvi_driver_sensor_data_t* data);

/**
* @brief Enable FIFO full interrupt on LIS2DH12.
* Triggers as ACTIVE HIGH interrupt once FIFO has 32 elements.
*
* @param[in] enable True to enable interrupt, false to disable interrupt
* @return RUUVI_DRIVER_SUCCESS on success, error code from stack otherwise.
**/
ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_interrupt_use(const bool enable);

/**
* Enable activity interrupt on LIS2DH12
* Triggers as ACTIVE HIGH interrupt while detected movement is above threshold limit_g
* Axes are high-passed for this interrupt, i.e. gravity won't trigger the interrupt
* Axes are examined individually, compound acceleration won't trigger the interrupt.
*
* @param[in] enable  True to enable interrupt, false to disable interrupt
* @param[in, out] limit_g: Desired acceleration to trigger the interrupt.
*                    Is considered as "at least", the acceleration is rounded up to next value.
*                    Is written with value that was set to interrupt
* @return RUUVI_DRIVER_SUCCESS on success
* @return RUUVI_DRIVER_ERROR_NULL if limit_g is NULL
* @return RUUVI_DRIVER_INVALID_STATE if acceleration limit is higher than maximum scale
* @return error code from stack on other error.
*
*/
ruuvi_driver_status_t ruuvi_interface_lis2dh12_activity_interrupt_use(const bool enable,
    float* limit_g);
/*@}*/
#endif
#endif