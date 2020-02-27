#ifndef NRF5_SDK5_CONFIG_H
#define NRF5_SDK5_CONFIG_H

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"    //!< Check if NRFX GPIOTE is required
#include "ruuvi_interface_flash.h"             //!< Check if FDS is required
#include "ruuvi_interface_log.h"               //!< Check if NRF_LOG is required
#include "ruuvi_interface_communication_nfc.h" //!< Check if NRF_NFC is required
#include "ruuvi_interface_power.h"             //!< Check if POWER is required
#include "ruuvi_interface_scheduler.h"         //!< Check if APP_SCHEDULER is required 
#include "ruuvi_interface_timer.h"             //!< Check if NRF_CLOCK, APP_TIMER required 
#include "ruuvi_interface_watchdog.h"          //!< Check if WDT is required
#include "ruuvi_interface_yield.h"             //!< Check if NRF_PWR_MGMT is required

#if (!NRF5_SDK15_CONFIGURED)
#        warning "NRF5 SDK15 is not configured, using defaults."
#endif

#ifndef NRF_BLE_SCAN_BUFFER
/** @brief maximum data in a BLE packet */
#define NRF_BLE_SCAN_BUFFER 31
#endif

#ifndef RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG
#define RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG 1
#endif

/** @brief priorities 0,1, 4,5 are reserved by SD */
#ifndef RUUVI_NRF5_SDK15_RADIO_IRQ_PRIORITY
#define RUUVI_NRF5_SDK15_RADIO_IRQ_PRIORITY 2
#endif

#ifndef NRF_SDH_ENABLED
/** @brief Required by SDK BLE modules not conditionally compiled */
#define NRF_SDH_ENABLED 1
#endif

#ifndef NRF_SDH_BLE_ENABLED
/** @brief Required by SDK BLE modules not conditionally compiled */
#  define NRF_SDH_BLE_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_GPIO_ENABLED
#  define GPIOTE_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_FLASH_ENABLED
#  define FDS_ENABLED 1
#  define NRF_FSTORAGE_ENABLED 1
#  ifndef NRF_SDH_SOC_ENABLED
#    define NRF_SDH_SOC_ENABLED 1
#  endif
#endif

#if RUUVI_NRF5_SDK15_LOG_ENABLED
#  define NRF_LOG_ENABLED 1
#  define NRF_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED 0
#  define NRF_LOG_DEFERRED 0
#endif

#if RUUVI_NRF5_SDK15_POWER_ENABLED
#  define POWER_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_SCHEDULER_ENABLED
#  define APP_SCHEDULER_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_TIMER_ENABLED
#  define APP_TIMER_ENABLED 1
#  define NRF_CLOCK_ENABLED 1
#  define TIMER_ENABLED 1
#  define TIMER1_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_NFC_ENABLED
#  define NFC_NDEF_MSG_ENABLED 1
#  define NFC_NDEF_RECORD_ENABLED 1
#  define NFC_PLATFORM_ENABLED 1
#  define NFC_NDEF_TEXT_RECORD_ENABLED 1
#  define NFC_NDEF_URI_MSG_ENABLED 1
#  define NFC_NDEF_URI_REC_ENABLED 1
#  define NFC_NDEF_MSG_PARSER_ENABLED 1
#  define NFC_NDEF_RECORD_PARSER_ENABLED 1
#  define NRFX_NFCT_ENABLED 1
#  define NFC_NDEF_MSG_TAG_TYPE 4
#  if (!RUUVI_NRF5_SDK15_TIMER_ENABLED)
#    error "NFC requires timer instance 4"
#  endif
#  define TIMER4_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_YIELD_ENABLED
#  define NRF_PWR_MGMT_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_WATCHDOG_ENABLED
#  define WDT_ENABLED 1
#endif



#endif