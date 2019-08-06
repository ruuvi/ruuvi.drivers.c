#include "ruuvi_driver_enabled_modules.h"
/**
 * @addtogroup Log
 * @brief Dummy functions used when logging is not enabled
 */
/*@{*/
/**
 * @file ruuvi_interface_log_dummy.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-05
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for printing out logging.
 * This implementation disregards all and returns success. 
 *
 */
#if !(APPLICATION_LOG_ENABLED)
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_log.h"
/**
 * @brief Runs initialization code for the logging backend and sets the severity level.
 *
 * @param min_severity least severe log level that will be printed.
 * @return @ref RUUVI_DRIVER_SUCCESS if log was init, error code otherwise
 */
ruuvi_driver_status_t ruuvi_interface_log_init(const ruuvi_interface_log_severity_t
    min_severity)
{
  return RUUVI_DRIVER_SUCCESS;
}

/**
 * @brief Blocks until remaining log messages are sent out
 *
 * @return @ref RUUVI_DRIVER_SUCCESS if buffered messages were sent, error otherwise.
 */
ruuvi_driver_status_t ruuvi_interface_log_flush(void)
{
  return RUUVI_DRIVER_SUCCESS;
}

/**
 * @brief Queues messages into log.
 *
 * May block or may return as soon as data is in buffer being transferred out
 *
 * @param severity severity of the log message
 * @param message message string
 *
 */
void ruuvi_interface_log(const ruuvi_interface_log_severity_t severity,
                         const char* const message)
{
  return;
}

/**
 * @brief Queues bytes to be logged out as a hex string
 *
 * May block or may return as soon as data is in buffer being transferred out
 *
 * @param severity severity of the log message
 * @param bytes raw bytes to log
 * @param byte:length length of bytes to log.
 *
 */
void ruuvi_interface_log_hex(const ruuvi_interface_log_severity_t severity,
                             const uint8_t* const bytes,
                             size_t byte_length)
{
  return;
}

/**
 * @brief Write text description of error message into given string pointer and null-terminate it.
 * The string will be cut if it cannot fit into given space.
 *
 * @param error error code to convert to string
 * @param error_string pointer to character array where error should be written
 * @param space_remaining How many bytes there are remaining in the error string.
 * @return number of bytes written (snprintf rvalue).
 */
size_t ruuvi_interface_error_to_string(ruuvi_driver_status_t error, char* error_string,
                                       size_t space_remaining)
{
  return space_remaining;
}

/**
 * Log the given configuration parameters at given log level.
 *
 * parameter level: Level of log. RUUVI_INTERFACE_LOG_ (ERROR, WARNING, INFO, DEBUG)
 * parameter configuration: Configuration to print
 * parameter unit: String representation to the unit of a scale
 */
void ruuvi_interface_log_sensor_configuration(const ruuvi_interface_log_severity_t level,
    const ruuvi_driver_sensor_configuration_t* const configuration, const char* unit)
{
  return;
}

#endif
/*@}*/