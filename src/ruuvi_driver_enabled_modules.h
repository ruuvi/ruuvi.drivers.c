/**
* @file ruuvi_driver_enabled_modules.h
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2020-01-18
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
* @brief Header to enable and disable module compilation.
*
* Use this header to select which interfaces will be included in project and
* which implementation to use for each interface.
*
* By default everything is disabled to save resources, application should
* have "app_config.h" which is in the include search path and further includes
* any platform-specific configuration such as "nrf5_sdk15_app_config.h".
* If application has configuration, define APPLICATION_DRIVER_CONFIGURED 1 in preprocessor.
*/

#ifndef RUUVI_DRIVER_ENABLED_MODULES_H
#define RUUVI_DRIVER_ENABLED_MODULES_H

/** @brief SemVer string, must match latest tag. */
#define RUUVI_DRIVER_SEMVER "0.1.2"

#ifdef CEEDLING
#define ENABLE_DEFAULT 1
#elif defined(DOXYGEN)
#define ENABLE_DEFAULT 1
#else
#define ENABLE_DEFAULT 0
#endif

#ifdef APPLICATION_DRIVER_CONFIGURED
#include "app_config.h"
#endif

#ifdef RUUVI_NRF5_SDK15_ENABLED
#include "nrf5_sdk15_app_config.h"
#endif

#ifndef RI_COMMUNICATION_MESSAGE_MAX_LENGTH
/** @brief Standard BLE Broadcast manufacturer specific
data payload length is the maximum length */
#define RI_COMMUNICATION_MESSAGE_MAX_LENGTH 24
#endif

#ifndef RD_LOG_BUFFER_SIZE
/** @brief Maximum length of one log message */
#define RD_LOG_BUFFER_SIZE (128U)
#endif

#ifndef RT_ADC_ENABLED
/** @brief Enable ADC task compilation. */
#define RT_ADC_ENABLED ENABLE_DEFAULT
#endif

#ifndef RT_ADV_ENABLED
/** @brief Enable BLE advertising compilation. */
#define RT_ADV_ENABLED ENABLE_DEFAULT
#endif

#ifndef RT_BUTTON_ENABLED
/** @brief Enable BLE advertising compilation. */
#define RT_BUTTON_ENABLED ENABLE_DEFAULT
#endif

#ifndef RT_GATT_ENABLED
/** @brief Enable GATT task compilation. */
#define RT_GATT_ENABLED ENABLE_DEFAULT
#endif

#if RT_GATT_ENABLED && (!RT_ADV_ENABLED)
#  error "GATT task requires Advertisement task"
#endif


#ifndef RT_GPIO_ENABLED
/** @brief Enable GPIO task compilation. */
#define RT_GPIO_ENABLED ENABLE_DEFAULT
#endif

#ifndef RT_LED_ENABLED
/** @brief Enable LED task compilation. */
#define RT_LED_ENABLED ENABLE_DEFAULT
#endif

#if RT_LED_ENABLED && (!RT_GPIO_ENABLED)
#  error "LED task requires GPIO task"
#endif

#ifndef RT_MAX_LED_CFG
#  if (!ENABLE_DEFAULT)
#    warning "Conserving space for 48 LEDs, are you sure?"
#  endif
/** @brief Conserve RAM for led task variables.
 *
 * You should override this with a lower value .
 */
#  define RT_MAX_LED_CFG 48
#endif

#endif