#ifndef ENVIRONMENTAL_H
#define ENVIRONMENTAL_H
#include "ruuvi_error.h"

#define ENVIRONMENTAL_INVALID RUUVI_FLOAT_INVALID
typedef struct
{
  float temperature; // C
  float humidity;    // RH-%
  float pressure;    // Pa
}ruuvi_environmental_data_t;

#endif