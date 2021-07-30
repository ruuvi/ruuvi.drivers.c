#include "ruuvi_driver_enabled_modules.h"
#if RI_LOG_ENABLED
/**
 * @addtogroup Log
 */
/** @{ */
/**
 * @file ruuvi_interface_log.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-05-20
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for printing out logging.
 * The application configuration and underlying implementation
 * decide how the log messages are handled, i.e. sent via RTT, UART or BLE.
 *
 */
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_log.h"
#include <stdio.h>
#include <string.h>

size_t ri_error_to_string (rd_status_t error,
                           char * const error_string, const size_t space_remaining)
{
    if (NULL == error_string)
    {
        RD_ERROR_CHECK (RD_ERROR_NULL, RD_ERROR_NULL);
        return 0;
    }

    size_t written = 0;
    rd_status_t error_bit = 0;

    // Print each error individually
    do
    {
        // Print comma + space if needed
        if (written != 0)
        {
            written += snprintf (error_string + written, space_remaining - written, ", ");
        }

        // If there is some error, print the lowest bit and reset the lowest bit in error code.
        if (error)
        {
            for (uint8_t ii = 0; ii < 32; ii++)
            {
                if (error & (1 << ii))
                {
                    error_bit = 1 << ii;
                    error &= 0xFFFFFFFF - (1 << ii);
                }
            }
        }

        switch (error_bit)
        {
            case RD_SUCCESS:
                written += snprintf (error_string + written, space_remaining - written, "%s", "SUCCESS");
                break;

            case RD_ERROR_INTERNAL:
                written += snprintf (error_string + written, space_remaining - written, "%s", "INTERNAL");
                break;

            case RD_ERROR_NOT_FOUND:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "NOT_FOUND");
                break;

            case RD_ERROR_NO_MEM:
                written += snprintf (error_string + written, space_remaining - written, "%s", "NO_MEM");
                break;

            case RD_ERROR_NOT_SUPPORTED:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "NOT_SUPPORTED");
                break;

            case RD_ERROR_INVALID_STATE:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "INVALID_STATE");
                break;

            case RD_ERROR_INVALID_LENGTH:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "INVALID_LENGTH");
                break;

            case RD_ERROR_INVALID_FLAGS:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "INVALID_FLAGS");
                break;

            case RD_ERROR_INVALID_DATA:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "INVALID_DATA");
                break;

            case RD_ERROR_INVALID_PARAM:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "INVALID_PARAM");
                break;

            case RD_ERROR_DATA_SIZE:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "DATA_SIZE");
                break;

            case RD_ERROR_TIMEOUT:
                written += snprintf (error_string + written, space_remaining - written, "%s", "TIMEOUT");
                break;

            case RD_ERROR_NULL:
                written += snprintf (error_string + written, space_remaining - written, "%s", "NULL");
                break;

            case RD_ERROR_FORBIDDEN:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "FORBIDDEN");
                break;

            case RD_ERROR_INVALID_ADDR:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "INVALID_ADDR");
                break;

            case RD_ERROR_BUSY:
                written += snprintf (error_string + written, space_remaining - written, "%s", "BUSY");
                break;

            case RD_ERROR_RESOURCES:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "RESOURCES");
                break;

            case RD_ERROR_NOT_IMPLEMENTED:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "NOT_IMPLEMENTED");
                break;

            case RD_ERROR_NOT_INITIALIZED:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "NOT_INITIALIZED");
                break;

            case RD_ERROR_SELFTEST:
                written += snprintf (error_string + written, space_remaining - written, "%s", "SELFTEST");
                break;

            case RD_ERROR_NOT_ACKNOWLEDGED:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "NOT ACKNOWLEDGED");
                break;

            case RD_ERROR_NOT_ENABLED:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "NOT ENABLED");
                break;

            case RD_WARNING_DEPRECATED:
                written += snprintf (error_string + written, space_remaining - written, "%s",
                                     "DEPRECATED");
                break;

            case RD_ERROR_FATAL:
                written += snprintf (error_string + written, space_remaining - written, "%s", "FATAL");
                break;

            default:
                written = snprintf (error_string + written, space_remaining - written, "%s", "UNKNOWN");
                break;
        }
    } while (error);

    return written;
}

// Convert configuration value to string.
static char * configuration_value_to_string (const uint8_t val)
{
    static char msg[17]; // sizeof "Not implemented", including NULL
    memset (msg, 0, sizeof (msg));

    if (val <= 200 && val > 0)
    {
        snprintf (msg, sizeof (msg), "%d", val);
    }
    else switch (val)
        {
            case RD_SENSOR_CFG_MIN:
                snprintf (msg, sizeof (msg), "MIN");
                break;

            case RD_SENSOR_CFG_MAX:
                snprintf (msg, sizeof (msg), "MAX");
                break;

            case RD_SENSOR_CFG_CONTINUOUS:
                snprintf (msg, sizeof (msg), "CONTINUOUS");
                break;

            case RD_SENSOR_CFG_DEFAULT:
                snprintf (msg, sizeof (msg), "DEFAULT");
                break;

            case RD_SENSOR_CFG_NO_CHANGE:
                snprintf (msg, sizeof (msg), "No change");
                break;

            case RD_SENSOR_CFG_SINGLE:
                snprintf (msg, sizeof (msg), "Single");
                break;

            case RD_SENSOR_CFG_SLEEP:
                snprintf (msg, sizeof (msg), "Sleep");
                break;

            case RD_SENSOR_ERR_NOT_SUPPORTED:
                snprintf (msg, sizeof (msg), "Not supported");
                break;

            case RD_SENSOR_ERR_NOT_IMPLEMENTED:
                snprintf (msg, sizeof (msg), "Not implemented");
                break;

            case RD_SENSOR_ERR_INVALID:
                snprintf (msg, sizeof (msg), "Invalid");
                break;

            default:
                snprintf (msg, sizeof (msg), "Unknown");
                break;
        }

    return msg;
}

void ri_log_sensor_configuration (const ri_log_severity_t level,
                                  const rd_sensor_configuration_t * const configuration, const char * unit)
{
    char msg[RD_LOG_BUFFER_SIZE] = {0};
    snprintf (msg, RD_LOG_BUFFER_SIZE, "Sample rate: %s Hz\r\n",
              configuration_value_to_string (configuration->samplerate));
    ri_log (level, msg);
    memset (msg, 0, sizeof (msg));
    snprintf (msg, RD_LOG_BUFFER_SIZE, "Resolution:  %s bits\r\n",
              configuration_value_to_string (configuration->resolution));
    ri_log (level, msg);
    memset (msg, 0, sizeof (msg));
    snprintf (msg, RD_LOG_BUFFER_SIZE, "Scale:       %s %s\r\n",
              configuration_value_to_string (configuration->scale), unit);
    ri_log (level, msg);
    memset (msg, 0, sizeof (msg));
    size_t written = snprintf (msg, RD_LOG_BUFFER_SIZE, "DSP:         ");

    switch (configuration->dsp_function)
    {
        case RD_SENSOR_DSP_HIGH_PASS:
            written += snprintf (msg + written, RD_LOG_BUFFER_SIZE - written, "High pass x ");
            break;

        case RD_SENSOR_DSP_LAST:
            written += snprintf (msg + written, RD_LOG_BUFFER_SIZE - written, "Last x ");
            break;

        case RD_SENSOR_DSP_LOW_PASS:
            written += snprintf (msg + written, RD_LOG_BUFFER_SIZE - written, "Lowpass x ");
            break;

        case RD_SENSOR_DSP_OS:
            written += snprintf (msg + written, RD_LOG_BUFFER_SIZE - written,
                                 "Oversampling x ");
            break;

        default:
            written += snprintf (msg + written, RD_LOG_BUFFER_SIZE - written, "Unknown x");
            break;
    }

    snprintf (msg + written, RD_LOG_BUFFER_SIZE - written, "%s\r\n",
              configuration_value_to_string (configuration->dsp_parameter));
    ri_log (level, msg);
    memset (msg, 0, sizeof (msg));
    written = snprintf (msg, RD_LOG_BUFFER_SIZE, "Mode:        %s\r\n",
                        configuration_value_to_string (configuration->mode));
    ri_log (level, msg);
}

void ri_log_hex (const ri_log_severity_t severity,
                 const uint8_t * const bytes,
                 size_t byte_length)
{
    char msg[RD_LOG_BUFFER_SIZE] =  { 0 };
    size_t index = 0;

    for (size_t ii = 0; ii < byte_length; ii++)
    {
        index += snprintf (msg + index, sizeof (msg) - index, "%02X", bytes[ii]);

        if (ii < (byte_length - 1))
        {
            index += snprintf (msg + index, sizeof (msg) - index, ":");
        }

        if (index >= sizeof (msg)) { return; }
    }

    ri_log (severity, msg);
}

#else
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_log.h"
/**
 * @brief Runs initialization code for the logging backend and sets the severity level.
 *
 * @return RD_SUCCESS if log was init, error code otherwise.
 */
rd_status_t ri_log_init (const ri_log_severity_t
                         min_severity)
{
    return RD_SUCCESS;
}

/**
 * @brief Blocks until remaining log messages are sent out
 *
 * @return RD_SUCCESS if buffered messages were sent, error otherwise.
 */
rd_status_t ri_log_flush (void)
{
    return RD_SUCCESS;
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
void ri_log (const ri_log_severity_t severity,
             const char * const message)
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
 * @param byte_length length of bytes to log.
 *
 */
void ri_log_hex (const ri_log_severity_t severity,
                 const uint8_t * const bytes,
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
size_t ri_error_to_string (rd_status_t error, char * error_string,
                           size_t space_remaining)
{
    return space_remaining;
}

/**
 * Log the given configuration parameters at given log level.
 *
 * parameter level: Level of log. RI_LOG_ (ERROR, WARNING, INFO, DEBUG)
 * parameter configuration: Configuration to print
 * parameter unit: String representation to the unit of a scale
 */
void ri_log_sensor_configuration (const ri_log_severity_t level,
                                  const rd_sensor_configuration_t * const configuration, const char * unit)
{
    return;
}

#endif
