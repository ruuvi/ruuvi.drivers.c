/**
 *  Define environmental sensor abstraction functions for onboard MCU
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#ifndef RUUVI_INTERFACE_ENVIRONMENTAL_MCU_H
#define RUUVI_INTERFACE_ENVIRONMENTAL_MCU_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_init(ruuvi_driver_sensor_t* environmental_sensor, ruuvi_driver_bus_t, uint8_t handle);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_uninit(ruuvi_driver_sensor_t* environmental_sensor, ruuvi_driver_bus_t, uint8_t handle);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_samplerate_set(uint8_t* samplerate);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_samplerate_get(uint8_t* samplerate);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_resolution_set(uint8_t* resolution);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_resolution_get(uint8_t* resolution);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_scale_set(uint8_t* scale);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_scale_get(uint8_t* scale);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_dsp_set(uint8_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_dsp_get(uint8_t* dsp, uint8_t* parameter);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_mode_set(uint8_t*);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_mode_get(uint8_t*);
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_data_get(void* data);

#endif