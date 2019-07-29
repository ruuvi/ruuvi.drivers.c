/**
 *  Define adc sensor abstraction functions for onboard MCU
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_ADC_MCU_H
#define RUUVI_INTERFACE_ADC_MCU_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

typedef enum
{
  RUUVI_INTERFACE_ADC_AIN0,
  RUUVI_INTERFACE_ADC_AIN1,
  RUUVI_INTERFACE_ADC_AIN2,
  RUUVI_INTERFACE_ADC_AIN3,
  RUUVI_INTERFACE_ADC_AIN4,
  RUUVI_INTERFACE_ADC_AIN5,
  RUUVI_INTERFACE_ADC_AIN6,
  RUUVI_INTERFACE_ADC_AIN7,
  RUUVI_INTERFACE_ADC_AINVDD,
  RUUVI_INTERFACE_ADC_AINGND,
} ruuvi_interface_adc_channel_t;

ruuvi_driver_status_t ruuvi_interface_adc_mcu_init(ruuvi_driver_sensor_t* adc_sensor,
    ruuvi_driver_bus_t, uint8_t handle);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_uninit(ruuvi_driver_sensor_t* adc_sensor,
    ruuvi_driver_bus_t, uint8_t handle);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_samplerate_set(uint8_t* samplerate);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_samplerate_get(uint8_t* samplerate);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_resolution_set(uint8_t* resolution);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_resolution_get(uint8_t* resolution);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_scale_set(uint8_t* scale);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_scale_get(uint8_t* scale);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_dsp_set(uint8_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_dsp_get(uint8_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_mode_set(uint8_t*);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_mode_get(uint8_t*);
ruuvi_driver_status_t ruuvi_interface_adc_mcu_data_get(void* data);

#endif