/**
 * Define environmental sensor abstraction functions for onboard MCU
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_BME280_H
#define RUUVI_INTERFACE_BME280_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

ruuvi_driver_status_t ruuvi_interface_bme280_init(ruuvi_driver_sensor_t* environmental_sensor);
ruuvi_driver_status_t ruuvi_interface_bme280_uninit(ruuvi_driver_sensor_t* environmental_sensor);
ruuvi_driver_status_t ruuvi_interface_bme280_samplerate_set(ruuvi_driver_sensor_samplerate_t* samplerate);
ruuvi_driver_status_t ruuvi_interface_bme280_samplerate_get(ruuvi_driver_sensor_samplerate_t* samplerate);
ruuvi_driver_status_t ruuvi_interface_bme280_resolution_set(ruuvi_driver_sensor_resolution_t* resolution);
ruuvi_driver_status_t ruuvi_interface_bme280_resolution_get(ruuvi_driver_sensor_resolution_t* resolution);
ruuvi_driver_status_t ruuvi_interface_bme280_scale_set(ruuvi_driver_sensor_scale_t* scale);
ruuvi_driver_status_t ruuvi_interface_bme280_scale_get(ruuvi_driver_sensor_scale_t* scale);
ruuvi_driver_status_t ruuvi_interface_bme280_dsp_set(ruuvi_driver_sensor_dsp_function_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_bme280_dsp_get(ruuvi_driver_sensor_dsp_function_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_bme280_mode_set(ruuvi_driver_sensor_mode_t*);
ruuvi_driver_status_t ruuvi_interface_bme280_mode_get(ruuvi_driver_sensor_mode_t*);
ruuvi_driver_status_t ruuvi_interface_bme280_data_get(void* data);

#endif