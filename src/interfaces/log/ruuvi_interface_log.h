#ifndef RUUVI_INTERFACE_LOG_H
#define RUUVI_INTERFACE_LOG_H
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @defgroup Log Logging functions
 * @brief Functions for printing out logs
 *
 */
/** @{ */
/**
 * @file ruuvi_interface_log.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-01-31
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for printing out logging.
 * The application configuration and underlying implementation
 * decide how the log messages are handled, i.e. sent via RTT, UART or BLE.
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include <stddef.h>

/** @brief Enable implementation selected by application */
#if RI_LOG_ENABLED
#   define RUUVI_NRF5_SDK15_LOG_ENABLED RUUVI_NRF5_SDK15_ENABLED
#   define RUUVI_FRUITY_LOG_ENABLED RUUVI_FRUITY_ENABLED
#endif


/**
 * Severity levels of log messages. Lower numerical value means more severe.
 **/
typedef enum
{
    RI_LOG_LEVEL_NONE = 0,   //<! Log nothing
    RI_LOG_LEVEL_ERROR,      //<! An error occured
    RI_LOG_LEVEL_WARNING,    //<! Warn user abour something, such as uninitialized peripheral
    RI_LOG_LEVEL_INFO,       //<! General information, such as changing mode of application
    RI_LOG_LEVEL_DEBUG       //<! Debug messages
} ri_log_severity_t;

/**
 * @brief Runs initialization code for the logging backend and sets the severity level.
 *
 * @param min_severity least severe log level that will be printed.
 * @retval RD_SUCCESS if log was init.
 * @retval RD_ERROR_INVALID_STATE if log had already been initialized.
 */
rd_status_t ri_log_init (const ri_log_severity_t min_severity);

/**
 * @brief Blocks until remaining log messages are sent out
 *
 * @return @ref RD_SUCCESS if buffered messages were sent, error otherwise.
 */
rd_status_t ri_log_flush (void);

/**
 * @brief Queues messages into log.
 *
 * May block or may return as soon as data is in buffer being transferred out
 *
 * @param severity severity of the log message
 * @param message message string
 *
 */
void ri_log (const ri_log_severity_t severity,
             const char * const message);

/**
 * @brief Queues bytes to be logged out as a hex string
 *
 * May block or may return as soon as data is in buffer being transferred out
 *
 * @param severity severity of the log message
 * @param bytes raw bytes to log
 * @param byte_length length of bytes to log.
 *
 */
void ri_log_hex (const ri_log_severity_t severity,
                 const uint8_t * const bytes,
                 size_t byte_length);

/**
 * @brief Write text description of error message into given string pointer and null-terminate it.
 * The string will be cut if it cannot fit into given space.
 *
 * @param error error code to convert to string
 * @param error_string pointer to character array where error should be written
 * @param space_remaining How many bytes there are remaining in the error string.
 * @return number of bytes written (snprintf rvalue).
 */
size_t ri_error_to_string (rd_status_t error, char * error_string,
                           size_t space_remaining);

/**
 * Log the given configuration parameters at given log level.
 *
 * parameter level: Level of log. RI_LOG_ (ERROR, WARNING, INFO, DEBUG)
 * parameter configuration: Configuration to print
 * parameter unit: String representation to the unit of a scale
 */
void ri_log_sensor_configuration (const ri_log_severity_t level,
                                  const rd_sensor_configuration_t * const configuration, const char * unit);
/** @} */
#endif
#ifdef __cplusplus
}
#endif
