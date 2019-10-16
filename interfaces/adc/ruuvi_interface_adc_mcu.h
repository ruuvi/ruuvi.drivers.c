#ifndef RUUVI_INTERFACE_ADC_MCU_H
#define RUUVI_INTERFACE_ADC_MCU_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_adc.h"
#include "ruuvi_interface_gpio.h"
/**
 * @addtogroup ADC
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_adc_mcu.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-07
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Interface for controlling ADC onboard MCU
 *
 *
 */
/* Analog input channels of device */
typedef enum
{
  RUUVI_INTERFACE_ADC_AIN0,          //!< Channel 0 of ADC
  RUUVI_INTERFACE_ADC_AIN1,          //!< Channel 1 of ADC
  RUUVI_INTERFACE_ADC_AIN2,          //!< Channel 2 of ADC
  RUUVI_INTERFACE_ADC_AIN3,          //!< Channel 3 of ADC
  RUUVI_INTERFACE_ADC_AIN4,          //!< Channel 4 of ADC
  RUUVI_INTERFACE_ADC_AIN5,          //!< Channel 6 of ADC
  RUUVI_INTERFACE_ADC_AIN6,          //!< Channel 7 of ADC
  RUUVI_INTERFACE_ADC_AIN7,          //!< Channel 7 of ADC
  RUUVI_INTERFACE_ADC_AINVDD,        //!< Analog supply voltage
  RUUVI_INTERFACE_ADC_AINGND,        //!< Analog ground
  RUUVI_INTERFACE_ADC_VREF_INTERNAL, //!< Internal voltage reference
  RUUVI_INTERFACE_ADC_VREF_EXTERNAL  //!< External voltage reference
} ruuvi_interface_adc_channel_t;

/** @brief Define a complex sample */
typedef struct {
  ruuvi_interface_adc_channel_t positive;  //!< Positive channel
  ruuvi_interface_adc_channel_t negative;  //!< Negative channel
  ruuvi_interface_adc_channel_t reference; //!< Reference channel
  uint8_t gain;                            //!< Gain of measurement. 1 for no gain. Rounded down. 
  uint8_t division;                        //!< Division of measurement. 1 -> unused. Gain must be 1 if division is not 1. Rounded down. 
  uint8_t oversamples;                     //!< Number os samples to take, rounded up to nearest possible value.
  uint16_t t_acquisition_us;               //!< Acquisition time, us. Rounded up to nearest possible value.
  bool positive_pullup_active;             //!< True to enable pullup to AVDD
  bool positive_pulldown_active;           //!< True to enable pulldown to AGND
  bool negative_pullup_active;             //!< True to enable pullup to AVDD
  bool negative_pulldown_active;           //!< True to enable pulldown to AGND
}ruuvi_interface_adc_sample_t;

/** @brief @ref ruuvi_driver_sensor_init_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_init(ruuvi_driver_sensor_t* adc_sensor,
    ruuvi_driver_bus_t, uint8_t handle);
/** @brief @ref ruuvi_driver_sensor_init_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_uninit(ruuvi_driver_sensor_t* adc_sensor,
    ruuvi_driver_bus_t, uint8_t handle);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_samplerate_set(uint8_t* samplerate);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_samplerate_get(uint8_t* samplerate);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_resolution_set(uint8_t* resolution);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_resolution_get(uint8_t* resolution);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_scale_set(uint8_t* scale);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_scale_get(uint8_t* scale);
/** @brief @ref ruuvi_driver_sensor_dsp_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_dsp_set(uint8_t* dsp, uint8_t* parameter);
/** @brief @ref ruuvi_driver_sensor_dsp_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_dsp_get(uint8_t* dsp, uint8_t* parameter);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_mode_set(uint8_t*);
/** @brief @ref ruuvi_driver_sensor_setup_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_mode_get(uint8_t*);
/** @brief @ref ruuvi_driver_sensor_data_fp */
ruuvi_driver_status_t ruuvi_interface_adc_mcu_data_get(ruuvi_driver_sensor_data_t* const data);

/**
 * @brief take complex sample
 *
 * This function fills the need for more complex sampling, such as using differential 
 * measurement, different reference voltages and oversampling. 
 * Initializes the ADC before sampling and uninitializes the ADC after sampling. 
 *
 * @param[in]  sample definition of the sample to take
 * @param[out] data value of sample in volts and as a ratio to reference. 
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_NULL if either parameter is NULL
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if ADC is already initialized
 * @return RUUVI_DRIVER_ERROR_INVALID_PARAMETER if configuration is invalid in any manner
 * @return error code from stack on other error. 
 */
ruuvi_driver_status_t ruuvi_interface_adc_complex_sample(const ruuvi_interface_adc_sample_t* const sample, ruuvi_interface_adc_data_t* const data);
/*@}*/
#endif