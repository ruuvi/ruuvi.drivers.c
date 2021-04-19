#ifndef RUUVI_INTERFACE_DPS310_H
#define RUUVI_INTERFACE_DPS310_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

/**
 * @addtogroup Environmental
 */
/** @{ */
/**
 * @defgroup DPS310 DPS310 Inteface
 * @brief Implement @ref rd_sensor_t functions on DPS310
 *
 * The implementation supports
 * different samplerates and oversampling.
 */
/** @} */
/**
 * @addtogroup DPS310
 */
/** @{ */
/**
 * @file ruuvi_interface_dps310.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-12-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for DPS310 basic usage. The underlying platform must provide
 * functions for SPI access, @ref ruuvi_interface_spi_dps310.h.
 *
 * Testing the interface with @ref ruuvi_driver_sensor_test.h
 *
 * @code{.c}
 *  rd_status_t err_code = RD_SUCCESS;
 *  rd_bus_t bus = RD_BUS_NONE;
 *  uint8_t handle = 0;
 *  rd_sensor_init_fp init = ri_dps310_init;
 *  bus = RD_BUS_SPI;
 *  handle = RUUVI_BOARD_SPI_SS_ENVIRONMENTAL_PIN;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  RD_ERROR_CHECK(err_code, RD_ERROR_SELFTEST);
 * @endcode
 */

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_dps310_init (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_dps310_uninit (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_dps310_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_dps310_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_mode_set (uint8_t * mode);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_dps310_mode_get (uint8_t * mode);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_dps310_data_get (rd_sensor_data_t * const data);

/** @} */
#endif // RUUVI_INTERFACE_DPS310_H
