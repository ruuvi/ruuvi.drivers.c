/**
 * @addtogroup Error
 * @{
 */
/**
 * @file ruuvi_driver_error.c
 * @author Otso Jousimaa
 * @date 2019-01-31
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause
 * @brief Check given error code, log warning on non-fatal error and reset on fatal error
 *
 */
#include "ruuvi_driver_enabled_modules.h"

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_power.h"

#include <stdio.h>
#include <string.h>


void ruuvi_driver_error_check(ruuvi_driver_status_t error,
                              ruuvi_driver_status_t non_fatal_mask, const char* file, int line)
{
  char message[APPLICATION_LOG_BUFFER_SIZE];
  size_t index = 0;
  // Cut out the full path
  const char* filename = strrchr(file, '/');

  // If on Windows
  if(NULL == filename) { filename = strrchr(file, '\\'); }

  // In case the file was already only the name
  if(NULL == filename) { filename = file; }
  // Otherwise skip the slash
  else { filename++; }

  // Reset on fatal error
  if(~non_fatal_mask & error)
  {
    index += snprintf(message, sizeof(message), "%s:%d FATAL: ", filename, line);
    index += ruuvi_interface_error_to_string(error, (message + index),
             (sizeof(message) - index));
    snprintf((message + index), (sizeof(message) - index), "\r\n");
    ruuvi_interface_log(RUUVI_INTERFACE_LOG_ERROR, message);
    ruuvi_interface_log_flush();
    ruuvi_interface_power_reset();
  }
  // Log non-fatal errors
  else if(RUUVI_DRIVER_SUCCESS != error)
  {
    index += snprintf(message, sizeof(message), "%s:%d WARNING: ", filename, line);
    index += ruuvi_interface_error_to_string(error, (message + index),
             (sizeof(message) - index));
    snprintf((message + index), (sizeof(message) - index), "\r\n");
    ruuvi_interface_log(RUUVI_INTERFACE_LOG_WARNING, message);
  }

  // Do nothing on success
}
/** @} */
