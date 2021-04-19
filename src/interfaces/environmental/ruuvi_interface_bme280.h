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
 * @brief Implement @ref rd_sensor_t functions on BME280
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
 * @date 2020-11-05
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for BME280 basic usage. The underlying platform must provide
 * functions for SPI and/or I2C access, @ref ruuvi_interface_spi_bme280.h and
 * @ref ruuvi_interface_i2c_bme280.h.
 *
 * Testing the interface with @ref ruuvi_driver_sensor_test.h
 *
 * @code{.c}
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 *  rd_status_t err_code = RD_SUCCESS;
 *  rd_bus_t bus = RD_BUS_NONE;
 *  uint8_t handle = 0;
 *  rd_sensor_init_fp init = ri_bme280_init;
 *  bus = RD_BUS_SPI;
 *  handle = RUUVI_BOARD_SPI_SS_ENVIRONMENTAL_PIN;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 * @endcode
 */

#define BME280_HUMIDITY_OFFSET (-3.0f) //!< Generally, BMEs show 3% too little. Compensate.

/**
 * @brief Implement delay in Bosch signature
 *
 * @param[in] time_ms time to delay
 */
void bosch_delay_ms (uint32_t time_ms);

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_bme280_init (rd_sensor_t *
                            environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_bme280_uninit (rd_sensor_t *
                              environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_bme280_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_bme280_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_mode_set (uint8_t * mode);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_bme280_mode_get (uint8_t * mode);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_bme280_data_get (rd_sensor_data_t * const
                                data);

#ifdef CEEDLING
#include "bme280_defs.h"
rd_status_t bme280_i2c_init (const struct bme280_dev * const p_dev, const uint8_t handle);
rd_status_t bme280_spi_init (const struct bme280_dev * const p_dev, const uint8_t handle);
rd_status_t ri2bme_rate (struct bme280_dev * p_dev, uint8_t * const samplerate);
uint32_t bme280_max_meas_time (const uint8_t oversampling);
#endif

/*@}*/
#endif
