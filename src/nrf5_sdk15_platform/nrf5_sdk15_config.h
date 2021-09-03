#ifndef NRF5_SDK5_CONFIG_H
#define NRF5_SDK5_CONFIG_H

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_aes.h"
#include "ruuvi_interface_communication_ble_advertising.h"
#include "ruuvi_interface_communication_ble_gatt.h"
#include "ruuvi_interface_communication_nfc.h" //!< Check if NRF_NFC is required
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"    //!< Check if NRFX GPIOTE is required
#include "ruuvi_interface_gpio_pwm.h"
#include "ruuvi_interface_flash.h"             //!< Check if FDS is required
#include "ruuvi_interface_i2c.h"               //!< Check if TWI is required.
#include "ruuvi_interface_log.h"               //!< Check if NRF_LOG is required
#include "ruuvi_interface_power.h"             //!< Check if POWER is required
#include "ruuvi_interface_rtc.h"               //!< Check if RTC is required
#include "ruuvi_interface_scheduler.h"         //!< Check if APP_SCHEDULER is required 
#include "ruuvi_interface_spi.h"               //!< Check if SPI is required
#include "ruuvi_interface_timer.h"             //!< Check if NRF_CLOCK, APP_TIMER required 
#include "ruuvi_interface_communication_uart.h"//!< Check if Serial is required
#include "ruuvi_interface_watchdog.h"          //!< Check if WDT is required
#include "ruuvi_interface_yield.h"             //!< Check if NRF_PWR_MGMT is required
#include "ruuvi_interface_adc_mcu.h"           //!< Check if NRF_SAADC is required

#if (!NRF5_SDK15_CONFIGURED)
#        warning "NRF5 SDK15 is not configured, using defaults."
#endif

#if RUUVI_NRF5_SDK15_AES_ENABLED
#   define NRF_CRYPTO_AES_ENABLED 1
#endif

#ifndef RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG
#define RUUVI_NRF5_SDK15_BLE4_STACK_CONN_TAG 1
#endif

/** @brief priorities 0,1, 4,5 are reserved by SD */
#ifndef RUUVI_NRF5_SDK15_RADIO_IRQ_PRIORITY
#define RUUVI_NRF5_SDK15_RADIO_IRQ_PRIORITY 2
#endif

#ifndef NRF_SDH_ENABLED
/** @brief Required by SDK BLE module conditional compilation */
#define NRF_SDH_ENABLED 1
#endif

#ifndef NRF_SDH_BLE_ENABLED
/** @brief Required by SDK BLE module conditional compilation */
#  define NRF_SDH_BLE_ENABLED NRF_SDH_ENABLED
#endif

#ifndef NRF_SDH_SOC_ENABLED
/** @brief Required by SDK BLE module conditional compilation */
#   define NRF_SDH_SOC_ENABLED NRF_SDH_ENABLED
#endif

#if RUUVI_NRF5_SDK15_ADV_ENABLED
/** @brief Required by SDK BLE module conditional compilation */
#   define NRF_QUEUE_ENABLED 1
/** @brief Required by SDK BLE module conditional compilation */
#   define NRF_BLE_SCAN_ENABLED 1
#   define NRF_BLE_SCAN_SCAN_INTERVAL (1000U) //!< Scan interval in 625 us units.
/**
 * @brief Scan window in 625 us units.
 * If scan_phys contains both BLE_GAP_PHY_1MBPS and BLE_GAP_PHY_CODED
 * interval shall be larger than or equal to twice the scan window.
 */
#   define NRF_BLE_SCAN_SCAN_WINDOW   (200U)
/** @brief Scan timeout in 10 ms units. */
#   define NRF_BLE_SCAN_SCAN_DURATION (3U * NRF_BLE_SCAN_SCAN_INTERVAL / 10U)
/** @brief Relevant only to centrals, but required. Milliseconds. */
#   define NRF_BLE_SCAN_SUPERVISION_TIMEOUT (4000U)
/** @brief Relevant only to centrals, but required. Milliseconds. */
#   define NRF_BLE_SCAN_MIN_CONNECTION_INTERVAL (20U)
/** @brief Relevant only to centrals, but required. Milliseconds. */
#   define NRF_BLE_SCAN_MAX_CONNECTION_INTERVAL (1000U)
/** @brief Relevant only to centrals, but required. Allowed skipped intervals. */
#   define NRF_BLE_SCAN_SLAVE_LATENCY 29

#   define RUUVI_NRF5_SDK15_ADV_QUEUE_LENGTH  3   //!< Number of advertisements that can be queued.
#   define RUUVI_NRF5_SDK15_SCAN_QUEUE_LENGTH 3   //!< Number of scans that can be queued.
#   if RUUVI_NRF5_SDK15_ADV_EXTENDED_ENABLED
#       define RUUVI_NRF5_SDK15_ADV_LENGTH    238 //!< Extended connectable data length
#       define RUUVI_NRF5_SDK15_SCAN_LENGTH   31  //!< Cannot have extended data + scanrsp
#       define NRF_BLE_SCAN_BUFFER            255 //!< Maximum scannable extended advertisement.
#   else
#       define RUUVI_NRF5_SDK15_ADV_LENGTH    31  //!< Standard message length
#       define RUUVI_NRF5_SDK15_SCAN_LENGTH   31  //!< Standard message length
#       define NRF_BLE_SCAN_BUFFER            31  //!< Standard advertisement legth
#   endif
#endif

#if RUUVI_NRF5_SDK15_GATT_ENABLED
#   define NRF_BLE_GATT_ENABLED (1U)
#   define NRF_BLE_QWR_ENABLED  (1U)
#   define NRF_BLE_CONN_PARAMS_ENABLED (1U)
#   define PEER_MANAGER_ENABLED (1U)
#   define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT (1U) //!< Only 1 allowed in SDK15
#   define NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION (1U) //!< Larger deviation will be renegotiated.
#   define NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION (100U) //!< 10 ms units, 1 s deviation allowed
#   define BLE_DFU_ENABLED (1U) //!< Enable DFU Service
#   define BLE_DIS_ENABLED (1U) //!< Enable DIS Service
#   define BLE_NUS_ENABLED (1U) //!< Enable NUS Service
#   define NRF_SDH_BLE_VS_UUID_COUNT (BLE_DFU_ENABLED\
                                      + BLE_DIS_ENABLED\
                                      + BLE_NUS_ENABLED)
#define NRF_SDH_BLE_SERVICE_CHANGED (1U) //!< Refresh service cache on connect
#endif

#if RUUVI_NRF5_SDK15_GPIO_ENABLED
#   define GPIOTE_ENABLED 1
#   define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS RT_GPIO_INT_TABLE_SIZE
#endif

#if RUUVI_NRF5_SDK15_ADC_ENABLED
#   define SAADC_ENABLED 1
#   define SAADC_CONFIG_RESOLUTION 1
#   define SAADC_CONFIG_OVERSAMPLE 0
#   define SAADC_CONFIG_LP_MODE 0
#   define SAADC_CONFIG_IRQ_PRIORITY 7
#endif

#if RUUVI_NRF5_SDK15_GPIO_PWM_ENABLED
#define PWM_ENABLED  1
#define PWM0_ENABLED 1
#define PWM1_ENABLED 0
#define PWM2_ENABLED 0
#define PWM_DEFAULT_CONFIG_OUT0_PIN 255
#define PWM_DEFAULT_CONFIG_OUT1_PIN 255
#define PWM_DEFAULT_CONFIG_OUT2_PIN 255
#define PWM_DEFAULT_CONFIG_OUT3_PIN 255
#define PWM_DEFAULT_CONFIG_IRQ_PRIORITY 6
#define PWM_DEFAULT_CONFIG_BASE_CLOCK 0
#define PWM_DEFAULT_CONFIG_COUNT_MODE 0
#define PWM_DEFAULT_CONFIG_TOP_VALUE 1000
#define PWM_DEFAULT_CONFIG_LOAD_MODE 2
#define PWM_DEFAULT_CONFIG_STEP_MODE 0
#endif

#if RUUVI_NRF5_SDK15_FLASH_ENABLED
#   define FDS_ENABLED 1
#   define NRF_FSTORAGE_ENABLED 1
#   define FDS_VIRTUAL_PAGES (RI_FLASH_PAGES + 1U) // +1 page for FDS GC.
#endif

#if RUUVI_NRF5_SDK15_I2C_ENABLED
#   if defined (NRF52832_XXAA) || defined (NRF52840_XXAA)
#        define TWI_ENABLED 1
#        define TWI1_ENABLED 1
#        define TWI1_USE_EASYDMA 0
#        define I2C_INSTANCE 1        //!< Leave instance 0 for SPI
#   elif(NRF52811_XXAA)
#        define TWI_ENABLED 1
#        define TWI0_ENABLED 1
#        define TWI0_USE_EASYDMA 0
#        define I2C_INSTANCE 0        //!< 811 shares instance number.
#   endif
#endif

#if RUUVI_NRF5_SDK15_SPI_ENABLED
#    define SPI_ENABLED 1
#    define SPI0_ENABLED 1
#    define SPI0_USE_EASYDMA 1
#    define SPI_INSTANCE 0
// <0=> NRF_GPIO_PIN_NOPULL
// <1=> NRF_GPIO_PIN_PULLDOWN
// <3=> NRF_GPIO_PIN_PULLUP
#define NRF_SPI_DRV_MISO_PULLUP_CFG 0
#endif

#if RUUVI_NRF5_SDK15_LOG_ENABLED
#   define NRF_LOG_ENABLED 1
#   define NRF_FPRINTF_FLAG_AUTOMATIC_CR_ON_LF_ENABLED 0
#   define NRF_LOG_DEFERRED 0
#endif

#if RUUVI_NRF5_SDK15_POWER_ENABLED
#  define POWER_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_RTC_ENABLED
#   if defined (NRF52832_XXAA) || defined (NRF52840_XXAA)
#       define RTC_ENABLED        1
#       define RTC2_ENABLED       1
#       define NRF5_SDK15_RTC_INSTANCE 2
#   endif
#endif

#if RUUVI_NRF5_SDK15_SCHEDULER_ENABLED
#   define APP_SCHEDULER_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_TIMER_ENABLED
#   define APP_TIMER_ENABLED 1
/** @brief Reserve space for 2 events per timer */
#   define APP_TIMER_CONFIG_OP_QUEUE_SIZE (RI_TIMER_MAX_INSTANCES * 2U)
#   define NRF_CLOCK_ENABLED 1
#   define TIMER_ENABLED 1
#   define TIMER1_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_NFC_ENABLED
#   define NFC_NDEF_MSG_ENABLED 1
#   define NFC_NDEF_RECORD_ENABLED 1
#   define NFC_PLATFORM_ENABLED 1
#   define NFC_NDEF_TEXT_RECORD_ENABLED 1
#   define NFC_NDEF_URI_MSG_ENABLED 1
#   define NFC_NDEF_URI_REC_ENABLED 1
#   define NFC_NDEF_MSG_PARSER_ENABLED 1
#   define NFC_NDEF_RECORD_PARSER_ENABLED 1
#   define NRFX_NFCT_ENABLED 1
#   define NFC_NDEF_MSG_TAG_TYPE 4
#   if (!RUUVI_NRF5_SDK15_TIMER_ENABLED)
#       error "NFC requires timer instance 4"
#   endif
#   define TIMER4_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_UART_ENABLED
#   define NRF_SERIAL_ENABLED 1
#   if defined (NRF52811_XXAA)
#       define NRFX_UARTE_ENABLED 1
#       define NRFX_UARTE0_ENABLED 1
#       define NRFX_UART_ENABLED 1
#       define NRFX_UART0_ENABLED 1
#       define UART0_CONFIG_USE_EASY_DMA 1
#       define NRFX_PRS_ENABLED 1
#       define NRFX_PRS_BOX_2_ENABLED 1
#   endif
#   if defined (NRF52832_XXAA)
#       // Serial module requires UART + UARTE
#       define NRFX_UARTE_ENABLED 1
#       define NRFX_UARTE0_ENABLED 1
#       define NRFX_UART_ENABLED 1
#       define NRFX_UART0_ENABLED 1
#       define UART0_CONFIG_USE_EASY_DMA 1
#       // PRS module allows UART + UARTE co-existence.
#       define NRFX_PRS_ENABLED 1
#       define NRFX_PRS_BOX_4_ENABLED 1
#   endif
#   if defined (NRF52840_XXAA)
#       // Serial module requires UART + UARTE
#       define NRFX_UARTE_ENABLED 1
#       define NRFX_UARTE0_ENABLED 1
#       define NRFX_UART_ENABLED 1
#       define NRFX_UART0_ENABLED 1
#       define UART0_CONFIG_USE_EASY_DMA 1
#       // PRS module allows UART + UARTE co-existence.
#       define NRFX_PRS_ENABLED 1
#       define NRFX_PRS_BOX_4_ENABLED 1
#   endif
#   define UART_EASY_DMA_SUPPORT 1
#   define UART_LEGACY_SUPPORT 1
#   define UART0_ENABLED 1
#else
// Required for Nordic SDK
#   define NRF_DRV_UART_WITH_UART 1
#endif

#if RUUVI_NRF5_SDK15_YIELD_ENABLED
#   define NRF_PWR_MGMT_ENABLED 1
#endif

#if RUUVI_NRF5_SDK15_WATCHDOG_ENABLED
#   define WDT_ENABLED 1
#endif



#endif