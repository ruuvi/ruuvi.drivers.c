/**
 * ADC data definition
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_ADC_H
#define RUUVI_INTERFACE_ADC_H
#include "ruuvi_driver_error.h"
#include <stdint.h>

#define RUUVI_INTERFACE_ADC_INVALID RUUVI_DRIVER_FLOAT_INVALID

typedef struct
{
  uint64_t timestamp_ms; // ms since boot
  float adc_v;           // V
  float reserved0;       // Add unused floats to keep struct at the same size as other data formats.
  float reserved1;
}ruuvi_interface_adc_data_t;

#endif