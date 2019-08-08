#ifndef RUUVI_INTERFACE_ACCELERATION_H
#define RUUVI_INTERFACE_ACCELERATION_H
/**
 * @defgroup Acceleration Acceleration sensing
 * @brief Interface and implementations for different accelerometers.
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_acceleration.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-07
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Accelerometer dataformat definition
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ACCELERATION_ENABLED || DOXYGEN
#include "ruuvi_driver_error.h"

#define RUUVI_INTERFACE_ACCELERATION_INVALID           RUUVI_DRIVER_FLOAT_INVALID
#define RUUVI_INTERFACE_ACCELERATION_INTERRUPT_DISABLE RUUVI_DRIVER_FLOAT_INVALID

/** @brief data structure for acceleration sample */
typedef struct
{
  uint64_t timestamp_ms; //!< Timestamp of this sample, use @ref ruuvi_driver_sensor_timestamp_get()
  float x_g;             //!< X-Acceleration value of this sample, after sensor built-in DSP
  float y_g;             //!< Y-Acceleration value of this sample, after sensor built-in DSP
  float z_g;             //!< Z-Acceleration value of this sample, after sensor built-in DSP
} ruuvi_interface_acceleration_data_t;

/*@}*/
#endif
#endif