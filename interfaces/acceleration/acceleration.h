#ifndef ACCELERATION_H
#define ACCELERATION_H
#include "ruuvi_error.h"

#define ACCELERATION_INVALID RUUVI_FLOAT_INVALID

// Unit is mg
typedef struct
{
  float x_mg;
  float y_mg;
  float z_mg;
}ruuvi_acceleration_data_t;

#endif