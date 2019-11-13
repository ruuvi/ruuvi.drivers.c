#ifndef RUUVI_INTERFACE_ADC_H
#define RUUVI_INTERFACE_ADC_H
#include "ruuvi_driver_error.h"
#include <stdint.h>

/**
 * @defgroup ADC Analog-to-digital converters
 * @brief Interface and implementations for different ADCs.
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_adc.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-07
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * ADC dataformat definition
 *
 */

#define RUUVI_INTERFACE_ADC_INVALID RUUVI_DRIVER_FLOAT_INVALID

typedef struct
{
  uint64_t timestamp_ms; // ms since boot
  float adc_v;           // V
  float adc_ratiometric; // -1 ... 1, ratio of ADC value to reference voltage.
  float reserved1;
} ruuvi_interface_adc_data_t;

/*@}*/

#endif