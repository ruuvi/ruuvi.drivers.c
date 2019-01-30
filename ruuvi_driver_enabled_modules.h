/**
 * Ruuvi error codes and error check function
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/

#ifndef RUUVI_DRIVER_EXTERNAL_INCLUDES_H
#define RUUVI_DRIVER_EXTERNAL_INCLUDES_H
#include "application_driver_configuration.h" //<! Enable modules used by your application.

#ifndef RUUVI_INTERFACE_ACCELERATION_ENABLED
#define RUUVI_INTERFACE_ACCELERATION_ENABLED 0 
#endif

#ifndef RUUVI_INTERFACE_ENVIRONMENTAL_BME280_ENABLED
#define RUUVI_INTERFACE_ENVIRONMENTAL_BME280_ENABLED 0
#endif

#ifndef RUUVI_NRF5_SDK15_ENABLED
#define RUUVI_NRF5_SDK15_ENABLED 0
#endif

#endif