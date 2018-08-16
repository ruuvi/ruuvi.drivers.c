/**
 * Environmental data format
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_ENVIRONMENTAL_H
#define RUUVI_INTERFACE_ENVIRONMENTAL_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_environmental_mcu.h"
#include "ruuvi_interface_bme280.h"
#include <stdint.h>

#define RUUVI_INTERFACE_ENVIRONMENTAL_INVALID RUUVI_DRIVER_FLOAT_INVALID

// List of sensors which can provide environmental functionality at least partially
typedef enum {
  RUUVI_INTERFACE_ENVIRONMENTAL_SENSOR_MCU    = 1,
  RUUVI_INTERFACE_ENVIRONMENTAL_SENSOR_BME280 = 2
}ruuvi_interface_environmental_supported_sensors;



typedef struct
{
  uint64_t timestamp_ms; // ms since boot
  float temperature_c;  // C
  float humidity_rh;    // RH-%
  float pressure_pa;    // Pa
}ruuvi_interface_environmental_data_t;

#endif