#ifndef RUUVI_INTERFACE_ACCELERATION_H
#define RUUVI_INTERFACE_ACCELERATION_H
#include "ruuvi_driver_error.h"

#define RUUVI_INTERFACE_ACCELERATION_INVALID RUUVI_DRIVER_FLOAT_INVALID

// Unit is mg
typedef struct
{
  uint64_t timestamp_ms;
  float x_mg;
  float y_mg;
  float z_mg;
}ruuvi_interface_acceleration_data_t;

#endif