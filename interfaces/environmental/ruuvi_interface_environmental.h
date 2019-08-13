#ifndef RUUVI_INTERFACE_ENVIRONMENTAL_H
#define RUUVI_INTERFACE_ENVIRONMENTAL_H
#include "ruuvi_driver_error.h"
#include <stdint.h>

/**
 * @defgroup Environmental Environmental sensing
 * @brief Interface and implementations for different, temperature, humidity and 
 *        barometric pressure sensors. 
 */
/*@{*/

/**
 * @file ruuvi_interface_environmental.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-07
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Environmental dataformat definition
 *
 */

/** @brief Signal the value is invalid for any reason */
#define RUUVI_INTERFACE_ENVIRONMENTAL_INVALID RUUVI_DRIVER_FLOAT_INVALID

typedef struct
{
  uint64_t timestamp_ms; //!< ms according to @ref ruuvi_driver_sensor_timestamp_get
  float temperature_c;   //!< C
  float humidity_rh;     //!< RH-%
  float pressure_pa;     //!< Pa
} ruuvi_interface_environmental_data_t;
/*@}*/
#endif