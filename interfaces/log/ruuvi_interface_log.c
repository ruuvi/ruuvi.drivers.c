#include "ruuvi_driver_enabled_modules.h"
#if APPLICATION_LOG_ENABLED
/**
 * @addtogroup Log
 * @{
 */
/**
 * @file ruuvi_interface_log.c
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
#include "ruuvi_interface_log.h"
#include <stdio.h>
#include <string.h>

size_t ruuvi_interface_error_to_string(ruuvi_driver_status_t error,
                                       char* const error_string, const size_t space_remaining)
{
  if(NULL == error_string)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_NULL, RUUVI_DRIVER_ERROR_NULL);
    return 0;
  }

  size_t written = 0;
  ruuvi_driver_status_t error_bit = 0;

  // Print each error individually
  do
  {
    // Print comma + space if needed
    if(written != 0)
    {
      written += snprintf(error_string + written, space_remaining - written, ", ");
    }

    // If there is some error, print the lowest bit and reset the lowest bit in error code.
    if(error)
    {
      for(uint8_t ii = 0; ii < 32; ii++)
      {
        if(error & (1 << ii))
        {
          error_bit = 1 << ii;
          error &= 0xFFFFFFFF - (1 << ii);
        }
      }
    }

    switch(error_bit)
    {
      case RUUVI_DRIVER_SUCCESS:
        written += snprintf(error_string + written, space_remaining - written, "%s", "SUCCESS");
        break;

      case RUUVI_DRIVER_ERROR_INTERNAL:
        written += snprintf(error_string + written, space_remaining - written, "%s", "INTERNAL");
        break;

      case RUUVI_DRIVER_ERROR_NOT_FOUND:
        written += snprintf(error_string + written, space_remaining - written, "%s", "NOT_FOUND");
        break;

      case RUUVI_DRIVER_ERROR_NO_MEM:
        written += snprintf(error_string + written, space_remaining - written, "%s", "NO_MEM");
        break;

      case RUUVI_DRIVER_ERROR_NOT_SUPPORTED:
        written += snprintf(error_string + written, space_remaining - written, "%s",
                            "NOT_SUPPORTED");
        break;

      case RUUVI_DRIVER_ERROR_INVALID_STATE:
        written += snprintf(error_string + written, space_remaining - written, "%s",
                            "INVALID_STATE");
        break;

      case RUUVI_DRIVER_ERROR_INVALID_LENGTH:
        written += snprintf(error_string + written, space_remaining - written, "%s",
                            "INVALID_LENGTH");
        break;

      case RUUVI_DRIVER_ERROR_INVALID_FLAGS:
        written += snprintf(error_string + written, space_remaining - written, "%s",
                            "INVALID_FLAGS");
        break;

      case RUUVI_DRIVER_ERROR_INVALID_DATA:
        written += snprintf(error_string + written, space_remaining - written, "%s",
                            "INVALID_DATA");
        break;

      case RUUVI_DRIVER_ERROR_INVALID_PARAM:
        written += snprintf(error_string + written, space_remaining - written, "%s",
                            "INVALID_PARAM");
        break;

      case RUUVI_DRIVER_ERROR_DATA_SIZE:
        written += snprintf(error_string + written, space_remaining - written, "%s", "DATA_SIZE");
        break;

      case RUUVI_DRIVER_ERROR_TIMEOUT:
        written += snprintf(error_string + written, space_remaining - written, "%s", "TIMEOUT");
        break;

      case RUUVI_DRIVER_ERROR_NULL:
        written += snprintf(error_string + written, space_remaining - written, "%s", "NULL");
        break;

      case RUUVI_DRIVER_ERROR_FORBIDDEN:
        written += snprintf(error_string + written, space_remaining - written, "%s", "FORBIDDEN");
        break;

      case RUUVI_DRIVER_ERROR_INVALID_ADDR:
        written += snprintf(error_string + written, space_remaining - written, "%s",
                            "INVALID_ADDR");
        break;

      case RUUVI_DRIVER_ERROR_BUSY:
        written += snprintf(error_string + written, space_remaining - written, "%s", "BUSY");
        break;

      case RUUVI_DRIVER_ERROR_RESOURCES:
        written += snprintf(error_string + written, space_remaining - written, "%s", "RESOURCES");
        break;

      case RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED:
        written += snprintf(error_string + written, space_remaining - written, "%s",
                            "NOT_IMPLEMENTED");
        break;

      case RUUVI_DRIVER_ERROR_SELFTEST:
        written += snprintf(error_string + written, space_remaining - written, "%s", "SELFTEST");
        break;

      case RUUVI_DRIVER_ERROR_FATAL:
        written += snprintf(error_string + written, space_remaining - written, "%s", "FATAL");
        break;

      default:
        written = snprintf(error_string + written, space_remaining - written, "%s", "UNKNOWN");
        break;
    }
  } while(error);

  return written;
}

// Convert configuration value to string.
static char* configuration_value_to_string(const uint8_t val)
{
  static char msg[17]; // sizeof "Not implemented", including NULL
  memset(msg, 0, sizeof(msg));

  if(val <= 200 && val > 0)
  {
    snprintf(msg, sizeof(msg), "%d", val);
  }
  else switch(val)
    {
      case RUUVI_DRIVER_SENSOR_CFG_MIN:
        snprintf(msg, sizeof(msg), "MIN");
        break;

      case RUUVI_DRIVER_SENSOR_CFG_MAX:
        snprintf(msg, sizeof(msg), "MAX");
        break;

      case RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS:
        snprintf(msg, sizeof(msg), "CONTINUOUS");
        break;

      case RUUVI_DRIVER_SENSOR_CFG_DEFAULT:
        snprintf(msg, sizeof(msg), "DEFAULT");
        break;

      case RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE:
        snprintf(msg, sizeof(msg), "No change");
        break;

      case RUUVI_DRIVER_SENSOR_CFG_SINGLE:
        snprintf(msg, sizeof(msg), "Single");
        break;

      case RUUVI_DRIVER_SENSOR_CFG_SLEEP:
        snprintf(msg, sizeof(msg), "Sleep");
        break;

      case RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED:
        snprintf(msg, sizeof(msg), "Not supported");
        break;

      case RUUVI_DRIVER_SENSOR_ERR_NOT_IMPLEMENTED:
        snprintf(msg, sizeof(msg), "Not implemented");
        break;

      case RUUVI_DRIVER_SENSOR_ERR_INVALID:
        snprintf(msg, sizeof(msg), "Invalid");
        break;

      default:
        snprintf(msg, sizeof(msg), "Unknown");
        break;
    }

  return msg;
}

void ruuvi_interface_log_sensor_configuration(const ruuvi_interface_log_severity_t level,
    const ruuvi_driver_sensor_configuration_t* const configuration, const char* unit)
{
  char msg[APPLICATION_LOG_BUFFER_SIZE] = {0};
  snprintf(msg, APPLICATION_LOG_BUFFER_SIZE, "Sample rate: %s Hz\r\n",
           configuration_value_to_string(configuration->samplerate));
  ruuvi_interface_log(level, msg);
  memset(msg, 0, sizeof(msg));
  snprintf(msg, APPLICATION_LOG_BUFFER_SIZE, "Resolution:  %s bits\r\n",
           configuration_value_to_string(configuration->resolution));
  ruuvi_interface_log(level, msg);
  memset(msg, 0, sizeof(msg));
  snprintf(msg, APPLICATION_LOG_BUFFER_SIZE, "Scale:       %s %s\r\n",
           configuration_value_to_string(configuration->scale), unit);
  ruuvi_interface_log(level, msg);
  memset(msg, 0, sizeof(msg));
  size_t written = snprintf(msg, APPLICATION_LOG_BUFFER_SIZE, "DSP:         ");

  switch(configuration->dsp_function)
  {
    case RUUVI_DRIVER_SENSOR_DSP_HIGH_PASS:
      written += snprintf(msg + written, APPLICATION_LOG_BUFFER_SIZE - written, "High pass x ");
      break;

    case RUUVI_DRIVER_SENSOR_DSP_LAST:
      written += snprintf(msg + written, APPLICATION_LOG_BUFFER_SIZE - written, "Last x ");
      break;

    case RUUVI_DRIVER_SENSOR_DSP_LOW_PASS:
      written += snprintf(msg + written, APPLICATION_LOG_BUFFER_SIZE - written, "Lowpass x ");
      break;

    case RUUVI_DRIVER_SENSOR_DSP_OS:
      written += snprintf(msg + written, APPLICATION_LOG_BUFFER_SIZE - written,
                          "Oversampling x ");
      break;

    default:
      written += snprintf(msg + written, APPLICATION_LOG_BUFFER_SIZE - written, "Unknown x");
      break;
  }

  snprintf(msg + written, APPLICATION_LOG_BUFFER_SIZE - written, "%s\r\n",
           configuration_value_to_string(configuration->dsp_parameter));
  ruuvi_interface_log(level, msg);
  memset(msg, 0, sizeof(msg));
  written = snprintf(msg, APPLICATION_LOG_BUFFER_SIZE, "Mode:        %s\r\n",
                     configuration_value_to_string(configuration->mode));
  ruuvi_interface_log(level, msg);
}
/** @} */
#endif
