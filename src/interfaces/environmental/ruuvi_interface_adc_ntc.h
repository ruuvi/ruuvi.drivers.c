#ifndef RUUVI_INTERFACE_ADC_NTC_H
#define RUUVI_INTERFACE_ADC_NTC_H
/**
 * @addtogroup Environmental
 */
/*@{*/
/**
 * @defgroup ADC_NTC ADC_NTC Interface
 * @brief Implement @ref ruuvi_driver_sensor_t functions on ADC_NTC
 *
 * The implementation supports taking single-samples and a continuous mode
 */
/*@}*/
/**
 * @addtogroup ADC_NTC
 */
/*@{*/
/**
 * @file ruuvi_interface_adc_ntc.h
 * @author Oleg Protasevich
 * @date 2020-06-05
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * ADC_NTC sensor driver.
 *
 */

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_adc_ntc_init (rd_sensor_t *
                             environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_adc_ntc_uninit (rd_sensor_t *
                               environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_ntc_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_ntc_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_ntc_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_ntc_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_ntc_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_ntc_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_adc_ntc_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_adc_ntc_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_ntc_mode_set (uint8_t *);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_adc_ntc_mode_get (uint8_t *);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_adc_ntc_data_get (rd_sensor_data_t * const
                                 data);
/*@}*/
#endif