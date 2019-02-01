#ifndef RUUVI_INTERFACE_LOG_H
#define RUUVI_INTERFACE_LOG_H
/**
 * @defgroup Log Logging functions
 * @brief Functions for printing out logs
 * 
 */
/*@{*/
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

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include <stddef.h>

/**
 * Severity levels of log messages. Lower numerical value means more severe.
 **/
typedef enum
{
  RUUVI_INTERFACE_LOG_ERROR,      //<! An error occured
  RUUVI_INTERFACE_LOG_WARNING,    //<! Warn user abour something, such as uninitialized peripheral
  RUUVI_INTERFACE_LOG_INFO,       //<! General information, such as changing mode of application
  RUUVI_INTERFACE_LOG_DEBUG       //<! Debug messages
}ruuvi_interface_log_severity_t;

/**
 * @brief Runs initialization code for the logging backend and sets the severity level.
 *
 * @param min_severity least severe log level that will be printed.
 * @return @ref RUUVI_DRIVER_SUCCESS if log was init, error code otherwise
 */
ruuvi_driver_status_t ruuvi_interface_log_init(const ruuvi_interface_log_severity_t min_severity);

/**
 * @brief Blocks until remaining log messages are sent out
 *
 * @return @ref RUUVI_DRIVER_SUCCESS if buffered messages were sent, error otherwise.
 */
ruuvi_driver_status_t ruuvi_interface_log_flush(void);

/**
 * @brief Queues messages into log. 
 *
 * May block or may return as soon as data is in buffer being transferred out
 *
 * @param severity severity of the log message
 * @param message message string
 *
 */
void ruuvi_interface_log(const ruuvi_interface_log_severity_t severity, const char* const message);

/**
 * @brief Write text description of error message into given string pointer and null-terminate it.
 * The string will be cut if it cannot fit into given space.
 *
 * @param error error code to convert to string
 * @param error_string pointer to character array where error should be written
 * @param space_remaining How many bytes there are remaining in the error string.
 * @return number of bytes written (snprintf rvalue).
 */
size_t ruuvi_interface_error_to_string(ruuvi_driver_status_t error, char* error_string, size_t space_remaining);

/**
 * Log the given configuration parameters at given log level.
 *
 * parameter level: Level of log. RUUVI_INTERFACE_LOG_ (ERROR, WARNING, INFO, DEBUG)
 * parameter configuration: Configuration to print
 * parameter unit: String representation to the unit of a scale
 */
void ruuvi_interface_log_sensor_configuration(const ruuvi_interface_log_severity_t level, const ruuvi_driver_sensor_configuration_t* const configuration, const char* unit);

#endif