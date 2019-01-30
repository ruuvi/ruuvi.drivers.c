/**
 * Acceleration data format
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_ACCELERATION_H
#define RUUVI_INTERFACE_ACCELERATION_H

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ACCELERATION_ENABLED
#include "ruuvi_driver_error.h"

#define RUUVI_INTERFACE_ACCELERATION_INVALID           RUUVI_DRIVER_FLOAT_INVALID
#define RUUVI_INTERFACE_ACCELERATION_INTERRUPT_DISABLE RUUVI_DRIVER_FLOAT_INVALID

// Unit is g
typedef struct
{
  uint64_t timestamp_ms;
  float x_g;
  float y_g;
  float z_g;
}ruuvi_interface_acceleration_data_t;

#endif