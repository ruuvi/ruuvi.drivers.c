#ifndef ri_ENVIRONMENTAL_MCU_H
#define ri_ENVIRONMENTAL_MCU_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

/**
 * @addtogroup Environmental
 */
/*@{*/
/**
 * @defgroup ENVIRONMENTAL_MCU Environmental sensing with MCU
 * @brief Implement @ref rd_sensor_t functions for environmental sensing on MCU
 *
 * Supported features depend on the underlying MCU
 */
/*@}*/
/**
 * @addtogroup ENVIRONMENTAL_MCU
 */
/*@{*/
/**
 * @file ruuvi_interface_environmental_mcu.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-10
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for
 *
 * Testing the interface with @ref test_sensor.h
 *
 * @code{.c}
 *  rd_ERROR_CHECK(err_code, rd_ERROR_SELFTEST);
 *  rd_status_t err_code = rd_SUCCESS;
 *  rd_bus_t bus = rd_BUS_NONE;
 *  uint8_t handle = 0;
 *  rd_sensor_init_fp init = ri_environmental_mcu_init;
 *  bus = NONE;
 *  err_code = test_sensor_init(init, bus, handle);
 *  err_code = test_sensor_setup(init, bus, handle);
 *  err_code = test_sensor_modes(init, bus, handle);
 *  rd_ERROR_CHECK(err_code, rd_ERROR_SELFTEST);
 * @endcode
 */

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_environmental_mcu_init (rd_sensor_t *
        environmental_sensor, rd_bus_t, uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_environmental_mcu_uninit (
    rd_sensor_t * environmental_sensor, rd_bus_t, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_environmental_mcu_samplerate_set (
    uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_environmental_mcu_samplerate_get (
    uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_environmental_mcu_resolution_set (
    uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_environmental_mcu_resolution_get (
    uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_environmental_mcu_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_environmental_mcu_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_environmental_mcu_dsp_set (uint8_t * dsp,
        uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_environmental_mcu_dsp_get (uint8_t * dsp,
        uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_environmental_mcu_mode_set (uint8_t *);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_environmental_mcu_mode_get (uint8_t *);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_environmental_mcu_data_get (
    rd_sensor_data_t * const data);
/*@}*/
#endif