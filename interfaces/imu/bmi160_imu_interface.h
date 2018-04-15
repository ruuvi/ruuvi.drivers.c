/**
 *  Implement ruuvi sensor abstraction functions for BME280.
 */

#ifndef BMI160_INTERFACE_H
#define BMI160_INTERFACE_H
#include "ruuvi_error.h"
#include "ruuvi_sensor.h"
#include "imu.h"

ruuvi_status_t bmi160_interface_init(ruuvi_sensor_t* imu_sensor);
ruuvi_status_t bmi160_interface_uninit(ruuvi_sensor_t* imu_sensor);
ruuvi_status_t bmi160_interface_samplerate_set(ruuvi_sensor_samplerate_t* samplerate);
ruuvi_status_t bmi160_interface_samplerate_get(ruuvi_sensor_samplerate_t* samplerate);
ruuvi_status_t bmi160_interface_resolution_set(ruuvi_sensor_resolution_t* resolution);
ruuvi_status_t bmi160_interface_resolution_get(ruuvi_sensor_resolution_t* resolution);
ruuvi_status_t bmi160_interface_scale_set(ruuvi_sensor_scale_t* scale);
ruuvi_status_t bmi160_interface_scale_get(ruuvi_sensor_scale_t* scale);
ruuvi_status_t bmi160_interface_dsp_set(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter);
ruuvi_status_t bmi160_interface_dsp_get(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter);
ruuvi_status_t bmi160_interface_mode_set(ruuvi_sensor_mode_t*);
ruuvi_status_t bmi160_interface_mode_get(ruuvi_sensor_mode_t*);
ruuvi_status_t bmi160_interface_interrupt_set(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp);
ruuvi_status_t bmi160_interface_interrupt_get(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp);
ruuvi_status_t bmi160_interface_data_get(void* data);

ruuvi_status_t bmi160_interface_acceleration_get (void* data);
ruuvi_status_t bmi160_interface_gyration_get     (void* data);

#endif