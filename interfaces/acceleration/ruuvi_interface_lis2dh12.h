/**
 *  Ruuvi sensor abstraction functions for LIS2DH12
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_LIS2DH12_H
#define RUUVI_INTERFACE_LIS2DH12_H
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ACCELERATION_LIS2DH12_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

#include <stdbool.h>
#include <stddef.h>
#include <stddef.h>

// Counts of self-test change in 10 bit resolution 2 G scale, datasheet.
#define RUUVI_INTERFACE_LIS2DH12_SELFTEST_DIFF_MIN 17
#define RUUVI_INTERFACE_LIS2DH12_SELFTEST_DIFF_MAX 360
#define RUUVI_INTERFACE_LIS2DH12_DEFAULT_SCALE 2
#define RUUVI_INTERFACE_LIS2DH12_DEFAULT_RESOLUTION 10

ruuvi_driver_status_t ruuvi_interface_lis2dh12_init(ruuvi_driver_sensor_t* acceleration_sensor, ruuvi_driver_bus_t bus, uint8_t handle);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_uninit(ruuvi_driver_sensor_t* acceleration_sensor, ruuvi_driver_bus_t bus, uint8_t handle);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_samplerate_set(uint8_t* samplerate);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_samplerate_get(uint8_t* samplerate);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_resolution_set(uint8_t* resolution);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_resolution_get(uint8_t* resolution);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_scale_set(uint8_t* scale);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_scale_get(uint8_t* scale);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_dsp_set(uint8_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_dsp_get(uint8_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_mode_set(uint8_t*);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_mode_get(uint8_t*);
ruuvi_driver_status_t ruuvi_interface_lis2dh12_data_get(void* data);

/**
 * Enable 32-level FIFO in LIS2DH12
 * If FIFO is enabled, values are stored on LIS2DH12 FIFO and oldest element is returned on data read.
 *
 * parameter enable: true to enable FIFO, false to disable or reset FIFO.
 * return: RUUVI_DRIVER_SUCCESS on success, error code from stack on error.
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_use(const bool enable);

/**
 * Read FIFO
 * Reads up to num_elements data points from FIFO and populates pointer data with them
 *
 * parameter num_elements: Input: number of elements in data. Output: Number of elements placed in data
 * parameter data: array of ruuvi_interface_acceleration_data_t with num_elements slots.
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_ERROR_NULL if either parameter is NULL
 * return: RUUVI_DRIVER_ERROR_INVALID_STATE if FIFO is not in use
 * return: error code from stack on error.
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_read(size_t* num_elements, ruuvi_interface_acceleration_data_t* data);

/**
 * Enable FIFO full interrupt on LIS2DH12.
 * Triggers as ACTIVE HIGH interrupt once FIFO has 32 elements.
 *
 * parameter enable: True to enable interrupt, false to disable interrupt
 * return: RUUVI_DRIVER_SUCCESS on success, error code from stack otherwise.
 **/
ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_interrupt_use(const bool enable);

/**
 * Enable activity interrupt on LIS2DH12
 * Triggers as ACTIVE HIGH interrupt while detected movement is above threshold limit_g
 * Axes are high-passed for this interrupt, i.e. gravity won't trigger the interrupt
 * Axes are examined individually, compound acceleration won't trigger the interrupt.
 *
 * parameter enable:  True to enable interrupt, false to disable interrupt
 * parameter limit_g: Desired acceleration to trigger the interrupt.
 *                    Is considered as "at least", the acceleration is rounded up to next value.
 *                    Is written with value that was set to interrupt
 * returns: RUUVI_DRIVER_SUCCESS on success
 * returns: RUUVI_DRIVER_INVALID_STATE if acceleration limit is higher than maximum scale
 * returns: error code from stack on error.
 *
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_activity_interrupt_use(const bool enable, float* limit_g);

#endif
#endif