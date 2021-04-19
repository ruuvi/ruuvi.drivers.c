#ifndef RUUVI_INTERFACE_LIS2DH12_H
#define RUUVI_INTERFACE_LIS2DH12_H
#include "ruuvi_driver_enabled_modules.h"
#if (RI_LIS2DH12_ENABLED || DOXYGEN)
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

#include <stdbool.h>
#include <stddef.h>
#include <stddef.h>
#include "lis2dh12_reg.h"

/**
 * @addtogroup Acceleration
 */
/** @{ */
/**
 * @defgroup LIS2DH12 LIS2DH12 Inteface
 * @brief Implement @ref rd_sensor_t functions on LIS2DH12
 *
 * The implementation supports
 * different resolutions, samplerates, high-passing, activity interrupt
 * and FIFO.
 */
/** @} */
/**
 * @addtogroup LIS2DH12
 */
/** @{ */
/**
 * @file ruuvi_interface_lis2dh12.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-04-24
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for LIS2DH12 basic usage. The underlying platform must provide
 * functions for SPI and/or I2C access, @ref ruuvi_interface_spi_lis2dh12.h.
 *
 * Testing the interface with @ref ruuvi_driver_sensor_test.h
 *
 * @code{.c}
 *  rd_status_t err_code = RD_SUCCESS;
 *  rd_bus_t bus = RD_BUS_NONE;
 *  uint8_t handle = 0;
 *  rd_sensor_init_fp init = ri_lis2dh12_init;
 *  bus = RD_BUS_SPI;
 *  handle = RUUVI_BOARD_SPI_SS_ACCELEROMETER_PIN;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 * @endcode
 */

/** @brief Minimum counts of self-test change in 10 bit resolution 2 G scale, ref datasheet.*/
#define RI_LIS2DH12_SELFTEST_DIFF_MIN (17)
/** @brief Maximum counts of self-test change in 10 bit resolution 2 G scale, ref datasheet.*/
#define RI_LIS2DH12_SELFTEST_DIFF_MAX (360)
/** @brief Scale used on "default" setting. */
#define RI_LIS2DH12_DEFAULT_SCALE (2U)
/** @brief Resolution used on "default" setting. */
#define RI_LIS2DH12_DEFAULT_RESOLUTION (10U)
#define LIS_SUCCESS (0)  //!< No error in LIS driver.
#define SELF_TEST_DELAY_MS (100U) //!< At least 3 samples at 400 Hz, but recommended value 100
#define SELF_TEST_SAMPLES_NUM (5) //!< 5 samples

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_lis2dh12_init (rd_sensor_t * acceleration_sensor, rd_bus_t bus,
                              uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_lis2dh12_uninit (rd_sensor_t * acceleration_sensor, rd_bus_t bus,
                                uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_lis2dh12_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_lis2dh12_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_lis2dh12_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_lis2dh12_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_lis2dh12_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_lis2dh12_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_lis2dh12_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_lis2dh12_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_lis2dh12_mode_set (uint8_t *);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_lis2dh12_mode_get (uint8_t *);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_lis2dh12_data_get (rd_sensor_data_t * const data);

/**
* @brief Enable 32-level FIFO in LIS2DH12
* If FIFO is enabled, values are stored on LIS2DH12 FIFO and oldest element is returned on data read.
*
* @param[in] enable true to enable FIFO, false to disable or reset FIFO.
* @return RD_SUCCESS on success, error code from stack on error.
*/
rd_status_t ri_lis2dh12_fifo_use (const bool enable);

/**
* @brief Read FIFO
* Reads up to num_elements data points from FIFO and populates pointer data with them
*
* @param[in, out] num_elements Input: number of elements in data. Output: Number of elements placed in data
* @param[out] data array with num_elements slots.
* @return RD_SUCCESS on success
* @return RD_ERROR_NULL if either parameter is NULL
* @return RD_ERROR_INVALID_STATE if FIFO is not in use
* @return error code from stack on error.
*/
rd_status_t ri_lis2dh12_fifo_read (size_t * num_elements, rd_sensor_data_t * data);

/**
* @brief Enable FIFO full interrupt on LIS2DH12.
* Triggers as ACTIVE HIGH interrupt once FIFO has 32 elements.
*
* @param[in] enable True to enable interrupt, false to disable interrupt
* @return RD_SUCCESS on success, error code from stack otherwise.
**/
rd_status_t ri_lis2dh12_fifo_interrupt_use (const bool enable);

/**
 * @brief Enable activity interrupt on LIS2DH12.
 *
 * Triggers as ACTIVE HIGH interrupt while detected movement is above threshold limit_g
 * Axes are high-passed for this interrupt, i.e. gravity won't trigger the interrupt
 * Axes are examined individually, compound acceleration won't trigger the interrupt.
 *
 * @param[in] enable  True to enable interrupt, false to disable interrupt
 * @param[in, out] limit_g: Desired acceleration to trigger the interrupt.
 *                    Is considered as "at least", the acceleration is rounded up to next value.
 *                    Is written with value that was set to interrupt
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if limit_g is NULL
 * @return RD_INVALID_STATE if acceleration limit is higher than maximum scale
 * @return error code from stack on other error.
 *
 */
rd_status_t ri_lis2dh12_activity_interrupt_use (const bool enable,
        float * limit_g);

/** @brief context for LIS2DH12 */
typedef struct
{
    lis2dh12_op_md_t resolution; //!< Resolution, bits. 8, 10, or 12.
    lis2dh12_fs_t scale;         //!< Scale, gravities. 2, 4, 8 or 16.
    lis2dh12_odr_t samplerate;   //!< Sample rate, 1 ... 200, or custom values for higher.
    lis2dh12_st_t selftest;      //!< Self-test enabled, positive, negative or disabled.
    uint8_t mode;                //!< Operating mode. Sleep, single or continuous.
    uint8_t handle;              //!< Device handle, SPI GPIO pin or I2C address.
    uint64_t tsample;            //!< Time of sample, @ref rd_sensor_timestamp_get
    stmdev_ctx_t ctx;            //!< Driver control structure
} ri_lis2dh12_dev;

/** @} */

#ifdef CEEDLING
// Give CEEDLING a handle to context.
extern ri_lis2dh12_dev dev;
#endif // CEEDLING

#endif // IF enabled
#endif // Include guard
