/**
* @file ruuvi_driver_enabled_modules.h
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2019-03-30
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
* @brief Header to enable and disable module compilation.
*
* Use this header to select which interfaces will be included in project and
* which implementation to use for each interface.
*/

#ifndef RUUVI_DRIVER_ENABLED_MODULES_H
#define RUUVI_DRIVER_ENABLED_MODULES_H

/** @brief Enable modules used by your application. See @ref application_driver_configuration.h.example */
#include "application_driver_configuration.h"

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