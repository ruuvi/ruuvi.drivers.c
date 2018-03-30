#ifndef MAGNETISM_H
#define MAGNETISM_H
#include "ruuvi_error.h"

#define MAGNETISM_INVALID RUUVI_FLOAT_INVALID

// Unit is mg
typedef struct
{
  float x_mg;
  float y_mg;
  float z_mg;
}ruuvi_magnetism_data_t;

#endif