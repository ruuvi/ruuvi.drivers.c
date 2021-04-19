#ifndef RUUVI_INTERFACE_ADC_PHOTO_H
#define RUUVI_INTERFACE_ADC_PHOTO_H
/**
 * @addtogroup Environmental
 */
/*@{*/
/**
 * @defgroup ADC_PHOTO ADC_PHOTO Interface
 * @brief Implement @ref rd_sensor_t functions on ADC_PHOTO
 *
 * The implementation supports taking single-samples and a continuous mode
 */
/*@}*/
/**
 * @addtogroup ADC_PHOTO
 */
/*@{*/
/**
 * @file ruuvi_interface_adc_photo.h
 * @author Oleg Protasevich
 * @date 2020-06-05
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * ADC_PHOTO sensor driver.
 *
 */

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_adc_photo_init (rd_sensor_t *
                               environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_adc_photo_uninit (rd_sensor_t *
                                 environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_photo_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_photo_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_photo_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_photo_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_photo_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_photo_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_adc_photo_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_adc_photo_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_photo_mode_set (uint8_t *);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_photo_mode_get (uint8_t *);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_adc_photo_data_get (rd_sensor_data_t * const
                                   data);
/*@}*/
#endif