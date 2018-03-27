#ifndef GYRATION_H
#define GYRATION_H
#include "ruuvi_error.h"

#define GYRATION_INVALID RUUVI_FLOAT_INVALID

// Unit is millo-degrees per second
typedef struct
{
  float x_mdps;
  float y_mdps;
  float z_mdps;
}ruuvi_gyration_data_t;

#endif