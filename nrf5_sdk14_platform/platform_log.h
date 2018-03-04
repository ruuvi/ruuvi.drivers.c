/**
 *  Functions for printing out log via preferred method, i.e. RTT, UART or BLE.
 *
 *  Logging should implement well-defined overflow behaviour, i.e. block or drop new / drop old
 *  if log buffer gets filled.
 *
 *  While logging could be implemented over Ruuvi Communications Interface, logging should be a separate
 *  flow which can be disabled in production.
 *
 *  Logging requires init and platform_log_level -functions.
 *  log must support levels ERROR, INFO, DEBUG.
 *
 *  Any prefixes, linenumbers etc are implemented at interface level and backend should print out raw data.
 *
 *  Usage:
 *    #define PLATFORM_LOG_LEVEL (NONE, ERROR, WARNING, INFO, DEBUG)   
 *    #include "platform_log.h"
 *    PLATFORM_LOG_MODULE_REGISTER() 
 * 
 */
#ifndef PLATFORM_LOG_H
#define PLATFORM_LOG_H

#include "application_config.h"
#ifdef NRF_LOG_ENABLED
#if NRF_LOG_ENABLED

#define NRF_LOG_MODULE_NAME PLATFORM_LOG_MODULE_NAME
#define NRF_LOG_LEVEL PLATFORM_LOG_LEVEL
#include "nrf_log.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_ctrl.h"

#define PLATFORM_LOG_INIT(timestamp_func)    NRF_LOG_INIT(timestamp_func)
#define PLATFORM_LOG_DEFAULT_BACKENDS_INIT() NRF_LOG_DEFAULT_BACKENDS_INIT()
#define PLATFORM_LOG_FLUSH()                 NRF_LOG_FLUSH()

#define PLATFORM_LOG_ERROR(...)   NRF_LOG_ERROR(__VA_ARGS__)
#define PLATFORM_LOG_WARNING(...) NRF_LOG_WARNING(__VA_ARGS__)
#define PLATFORM_LOG_INFO(...)    NRF_LOG_INFO(__VA_ARGS__)
#define PLATFORM_LOG_DEBUG(...)   NRF_LOG_DEBUG(__VA_ARGS__)

#define PLATFORM_LOG_HEXDUMP_ERROR(p_data, len)   NRF_LOG_HEXDUMP_ERROR(p_data, len)
#define PLATFORM_LOG_HEXDUMP_WARNING(p_data, len) NRF_LOG_HEXDUMP_WARNING(p_data, len)
#define PLATFORM_LOG_HEXDUMP_INFO(p_data, len)    NRF_LOG_HEXDUMP_INFO(p_data, len)
#define PLATFORM_LOG_HEXDUMP_DEBUG(p_data, len)   NRF_LOG_HEXDUMP_DEBUG(p_data, len)

/**
 * @brief Macro to be used in a formatted string to a pass float number to the log.
 *
 * Macro should be used in formatted string instead of the %f specifier together with
 * @ref NRF_LOG_FLOAT macro.
 * Example: NRF_LOG_INFO("My float number" NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(f)))
 */
#define PLATFORM_LOG_FLOAT_MARKER NRF_LOG_FLOAT_MARKER

/**
 * @brief Macro for dissecting a float number into two numbers (integer and residuum).
 */
#define PLATFORM_LOG_FLOAT NRF_LOG_FLOAT(val)

/**
 * @def PLATFORM_LOG_MODULE_REGISTER
 * @brief Macro for registering an independent module.
 */


#define _CONST const
#define PLATFORM_LOG_MODULE_REGISTER() NRF_LOG_MODULE_REGISTER()

#endif // LOG is defined
#endif // LOG is enabled
#endif //PLATFORM_LOG_HJ