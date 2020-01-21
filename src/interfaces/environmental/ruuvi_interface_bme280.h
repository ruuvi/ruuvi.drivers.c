#ifndef RUUVI_INTERFACE_BME280_H
#define RUUVI_INTERFACE_BME280_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

/**
 * @addtogroup Environmental
 */
/*@{*/
/**
 * @defgroup BME280 BME280 Inteface
 * @brief Implement @ref ruuvi_driver_sensor_t functions on BME280
 *
 * The implementation supports
 * different samplerates, low-pass filtering and oversampling.
 */
/*@}*/
/**
 * @addtogroup BME280
 */
/*@{*/
/**
 * @file ruuvi_interface_bme280.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-08
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for BME280 basic usage. The underlying platform must provide
 * functions for SPI and/or I2C access, @ref ruuvi_interface_spi_bme280.h and
 * @ref ruuvi_interface_i2c_bme280.h.
 *
 * Testing the interface with @ref test_sensor.h
 *
 * @code{.c}
 *  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_ERROR_SELFTEST);
 *  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
 *  ruuvi_driver_bus_t bus = RUUVI_DRIVER_BUS_NONE;
 *  uint8_t handle = 0;
 *  ruuvi_driver_sensor_init_fp init = ruuvi_interface_bme280_init;
 *  bus = RUUVI_DRIVER_BUS_SPI;
 *  handle = RUUVI_BOARD_SPI_SS_ENVIRONMENTAL_PIN;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_ERROR_SELFTEST);
 * @endcode
 */

/**
 * @brief Implement delay in Bosch signature
 *
 * @param[in] time_ms time to delay
 */
void bosch_delay_ms (uint32_t time_ms);

/** @brief @ref ruuvi_driver_sensor_init_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_init (ruuvi_driver_sensor_t *
        environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle);
/** @brief @ref ruuvi_driver_sensor_init_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_uninit (ruuvi_driver_sensor_t *
        environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_samplerate_set (uint8_t * samplerate);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_samplerate_get (uint8_t * samplerate);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_resolution_set (uint8_t * resolution);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_resolution_get (uint8_t * resolution);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_scale_set (uint8_t * scale);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_scale_get (uint8_t * scale);
/** @brief @ref ruuvi_driver_sensor_dsp_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref ruuvi_driver_sensor_dsp_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_mode_set (uint8_t *);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_mode_get (uint8_t *);
/** @brief @ref ruuvi_driver_sensor_data_fp */
ruuvi_driver_status_t ruuvi_interface_bme280_data_get (ruuvi_driver_sensor_data_t * const
        data);
/*@}*/
#endif