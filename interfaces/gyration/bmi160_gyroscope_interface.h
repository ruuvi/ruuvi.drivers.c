/**
 *  Implement ruuvi sensor abstraction functions for BMI160.
 */

#ifndef BMI160_INTERFACE_H
#define BMI160_INTERFACE_H
#include "ruuvi_error.h"
#include "ruuvi_sensor.h"

#define BMI160_125_RAW_TO_DPS(raw) (float)(raw *0.0038f)
#define BMI160_2000_RAW_TO_DPS(raw) (float)(raw *0.061f)

ruuvi_status_t bmi160_gyroscope_interface_init(ruuvi_sensor_t* gyration_sensor);
ruuvi_status_t bmi160_gyroscope_interface_uninit(ruuvi_sensor_t* gyration_sensor);
ruuvi_status_t bmi160_gyroscope_interface_samplerate_set(ruuvi_sensor_samplerate_t* samplerate);
ruuvi_status_t bmi160_gyroscope_interface_samplerate_get(ruuvi_sensor_samplerate_t* samplerate);
ruuvi_status_t bmi160_gyroscope_interface_resolution_set(ruuvi_sensor_resolution_t* resolution);
ruuvi_status_t bmi160_gyroscope_interface_resolution_get(ruuvi_sensor_resolution_t* resolution);
ruuvi_status_t bmi160_gyroscope_interface_scale_set(ruuvi_sensor_scale_t* scale);
ruuvi_status_t bmi160_gyroscope_interface_scale_get(ruuvi_sensor_scale_t* scale);
ruuvi_status_t bmi160_gyroscope_interface_dsp_set(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter);
ruuvi_status_t bmi160_gyroscope_interface_dsp_get(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter);
ruuvi_status_t bmi160_gyroscope_interface_mode_set(ruuvi_sensor_mode_t* mode);
ruuvi_status_t bmi160_gyroscope_interface_mode_get(ruuvi_sensor_mode_t* mode);
ruuvi_status_t bmi160_gyroscope_interface_interrupt_set(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp);
ruuvi_status_t bmi160_gyroscope_interface_interrupt_get(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp);
ruuvi_status_t bmi160_gyroscope_interface_data_get(void* data);

#endif