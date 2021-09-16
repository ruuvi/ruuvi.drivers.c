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
#define RUUVI_DRIVERS_SEMVER "3.9.2"

#ifdef CEEDLING
#  define ENABLE_DEFAULT 1
#elif defined(DOXYGEN)
#  define ENABLE_DEFAULT 1
#elif defined(RUUVI_RUN_TESTS)
#  define ENABLE_DEFAULT 1
#else
#  define ENABLE_DEFAULT 0
#endif

#ifdef APPLICATION_DRIVER_CONFIGURED
#include "app_config.h"
#endif

#ifdef RUUVI_NRF5_SDK15_ENABLED
#include "nrf5_sdk15_app_config.h"
#endif

#ifdef RUUVI_FRUITY_ENABLED
#include "fruity_config.h"
#endif

#ifndef RI_ADV_EXTENDED_ENABLED
#   define RI_ADV_EXTENDED_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_COMM_MESSAGE_MAX_LENGTH
/** @brief Standard BLE Broadcast manufacturer specific
data payload length is the maximum length */
#   if RI_ADV_EXTENDED_ENABLED
#       define RI_COMM_MESSAGE_MAX_LENGTH 230
#   else
#       define RI_COMM_MESSAGE_MAX_LENGTH 24
#   endif
#endif

#ifndef RI_ATOMIC_ENABLED
#   define RI_ATOMIC_ENABLED ENABLE_DEFAULT
#endif

#ifndef RD_LOG_BUFFER_SIZE
/** @brief Maximum length of one log message */
#  define RD_LOG_BUFFER_SIZE (128U)
#endif

#ifndef RT_ADC_ENABLED
/** @brief Enable ADC task compilation. */
#  define RT_ADC_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_COMM_ENABLED
/** @brief Enable communication helper compilation. */
#  define RI_COMM_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_AES_ENABLED
/** @brief Enable communication helper compilation. */
#  define RI_AES_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_RADIO_ENABLED
/** brief enable radio usage */
#  define RI_RADIO_ENABLED ENABLE_DEFAULT
#endif


#ifndef RT_ADV_ENABLED
/** @brief Enable BLE advertising compilation. */
#  define RT_ADV_ENABLED ENABLE_DEFAULT
#endif

#define RI_ADV_ENABLED RT_ADV_ENABLED

#if RT_ADV_ENABLED && !(RI_COMM_ENABLED)
#  error "Advertisement task requires communication interface."
#endif

#if RT_ADV_ENABLED && !(RI_RADIO_ENABLED)
#  error "Advertisement task requires radio interface."
#endif

#ifndef RT_BUTTON_ENABLED
/** @brief Enable BLE advertising compilation. */
#  define RT_BUTTON_ENABLED ENABLE_DEFAULT
#endif

#ifndef RT_COMMUNICATION_ENABLED
#   define RT_COMMUNICATION_ENABLED ENABLE_DEFAULT
#endif

#ifndef RT_FLASH_ENABLED
/** @brief Enable Flash storage compilation. */
#  define RT_FLASH_ENABLED ENABLE_DEFAULT
#endif

#ifndef RT_GATT_ENABLED
/** @brief Enable GATT task compilation. */
#  define RT_GATT_ENABLED ENABLE_DEFAULT
#endif

#if RT_GATT_ENABLED
#   define RI_GATT_ENABLED (1U)
#endif

#if RT_GATT_ENABLED && ((!RT_ADV_ENABLED) || !(RI_COMM_ENABLED))
#  error "GATT task requires Advertisement task and communication interface."
#endif

#if RT_GATT_ENABLED && ((!RT_ADV_ENABLED) || !(RI_RADIO_ENABLED))
#  error "GATT task requires Advertisement task and radio interface."
#endif

#ifndef RT_GPIO_ENABLED
/** @brief Enable GPIO task compilation. */
#  define RT_GPIO_ENABLED ENABLE_DEFAULT
#endif

#if RT_GPIO_ENABLED
#  ifndef RT_GPIO_INT_TABLE_SIZE
#    if (!ENABLE_DEFAULT)
#      warning "Conserving space for 48 GPIOs, are you sure?"
#    endif
/** @brief Conserve RAM for led task variables.
 *
 * You should override this with a lower value.
 */
#  define RT_GPIO_INT_TABLE_SIZE 48
#  endif
#endif

#ifndef RI_I2C_ENABLED
#   define RI_I2C_ENABLED ENABLE_DEFAULT
#endif

#ifndef RT_LED_ENABLED
/** @brief Enable LED task compilation. */
#  define RT_LED_ENABLED ENABLE_DEFAULT
#endif

#if RT_LED_ENABLED
#  if (!RT_GPIO_ENABLED)
#    error "LED task requires GPIO task"
#  endif
#  ifndef RT_MAX_LED_CFG
#    if (!ENABLE_DEFAULT)
#      warning "Conserving space for 48 LEDs, are you sure?"
#    endif
/** @brief Conserve RAM for led task variables.
 *
 * You should override this with a lower value.
 */
#  define RT_MAX_LED_CFG 48
#  endif
#endif

#ifndef RT_NFC_ENABLED
#  define RT_NFC_ENABLED ENABLE_DEFAULT
#endif

#if RT_NFC_ENABLED
#  ifndef RI_NFC_ENABLED
#    define RI_NFC_ENABLED 1
#  endif
#endif

#ifndef RI_RTC_ENABLED
#  define RI_RTC_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_SCHEDULER_ENABLED
#  define RI_SCHEDULER_ENABLED ENABLE_DEFAULT
#endif

#if RI_SCHEDULER_ENABLED
#  ifndef RI_SCHEDULER_LENGTH
#    define RI_SCHEDULER_LENGTH (10U)
#  endif
#  ifndef RI_SCHEDULER_SIZE
#    define RI_SCHEDULER_SIZE (32U)
#  endif
#endif

#ifndef RI_SPI_ENABLED
#   define RI_SPI_ENABLED ENABLE_DEFAULT
#endif

#if RI_TIMER_ENABLED
#  ifndef RI_TIMER_MAX_INSTANCES
#    define RI_TIMER_MAX_INSTANCES (10U)
#  endif
#endif

#ifndef RI_UART_ENABLED
#   define RI_UART_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_YIELD_ENABLED
#define RI_YIELD_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_WATCHDOG_ENABLED
#define RI_WATCHDOG_ENABLED ENABLE_DEFAULT
#endif

/** SENSORS **/
#ifndef RT_SENSOR_ENABLED
#   define RT_SENSOR_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_BME280_ENABLED
#   define RI_BME280_ENABLED ENABLE_DEFAULT
#   ifndef RI_BME280_SPI_ENABLED
#       define RI_BME280_SPI_ENABLED ENABLE_DEFAULT
#   endif
#   ifndef RI_BME280_I2C_ENABLED
#       define RI_BME280_I2C_ENABLED ENABLE_DEFAULT
#   endif
#endif

#ifndef RI_DPS310_ENABLED
#   define RI_DPS310_ENABLED ENABLE_DEFAULT
#   ifndef RI_DPS310_SPI_ENABLED
#       define RI_DPS310_SPI_ENABLED ENABLE_DEFAULT
#   endif
#   ifndef RI_DPS310_I2C_ENABLED
#       define RI_DPS310_I2C_ENABLED ENABLE_DEFAULT
#   endif
#endif

#ifndef RI_LIS2DH12_ENABLED
#   define RI_LIS2DH12_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_TMP117_ENABLED
#   define RI_TMP117_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_SHTCX_ENABLED
#   define RI_SHTCX_ENABLED ENABLE_DEFAULT
#endif

#ifndef RI_ADC_PHOTO_ENABLED
#   define RI_ADC_PHOTO_ENABLED      ENABLE_DEFAULT
#endif

#ifndef RUUVI_NRF5_SDK15_NRF52832_ENVIRONMENTAL_ENABLED
#   define RUUVI_NRF5_SDK15_NRF52832_ENVIRONMENTAL_ENABLED ENABLE_DEFAULT
#endif

// TODO: Refactor configuration constants elsewhere
#   define RI_ADC_PHOTO_CHANNEL      2
#   define RI_ADC_PHOTO_DIVIDER      1.00f //!< Predivider installed on board

#ifndef RI_ADC_NTC_ENABLED
#   define RI_ADC_NTC_ENABLED        ENABLE_DEFAULT
#endif
// TODO: Refactor configuration constants elsewhere
// NTC is installed as VDD - fixed resistor - ADC - NTC - GND.
#define RI_ADC_NTC_CHANNEL        1
#define RI_ADC_NTC_DIVIDER        1.00f //!< Predivider installed on board
#define RI_ADC_NTC_BALANCE        10000.00f //!< Fixed resistor value
#define RI_ADC_NTC_DEFAULT_RES    10000.00f //!< NTC at default temp
#define RI_ADC_NTC_DEFAULT_TEMP   25.00f    //!< Calibration temperature
#define RI_ADC_NTC_DEFAULT_BETA   3974.0f   //!< Beta value of NTC, check datasheet.


#endif
