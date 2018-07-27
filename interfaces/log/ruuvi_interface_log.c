/**
 * Platform-independent helper function for 
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/

 #include "ruuvi_driver_error.h"
 #include <stdio.h>
size_t ruuvi_platform_error_to_string(ruuvi_driver_status_t error, char* error_string, size_t space_remaining)
{
  if(NULL == error_string)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_NULL, RUUVI_DRIVER_ERROR_NULL);
    return 0; 
  }

  size_t written = 0;
  switch(error)
  {
    case RUUVI_DRIVER_SUCCESS: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_SUCCESS");
      break;
    
    case RUUVI_DRIVER_ERROR_INTERNAL: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_INTERNAL");
      break;

    case RUUVI_DRIVER_ERROR_NOT_FOUND: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_NOT_FOUND");
      break;

    case RUUVI_DRIVER_ERROR_NO_MEM: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_NO_MEM");
      break;

    case RUUVI_DRIVER_ERROR_NOT_SUPPORTED: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_NOT_SUPPORTED");
      break;
    
     case RUUVI_DRIVER_ERROR_INVALID_STATE: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_INVALID_STATE");
      break;

    case RUUVI_DRIVER_ERROR_INVALID_LENGTH: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_INVALID_LENGTH");
      break;

    case RUUVI_DRIVER_ERROR_INVALID_FLAGS: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_INVALID_FLAGS");
      break;

    case RUUVI_DRIVER_ERROR_INVALID_DATA: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_INVALID_DATA");
      break;

    case RUUVI_DRIVER_ERROR_DATA_SIZE: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_DATA_SIZE");
      break;

    case RUUVI_DRIVER_ERROR_TIMEOUT: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_TIMEOUT");
      break;

    case RUUVI_DRIVER_ERROR_NULL: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_NULL");
      break;

    case RUUVI_DRIVER_ERROR_FORBIDDEN: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_FORBIDDEN");
      break;

    case RUUVI_DRIVER_ERROR_INVALID_ADDR: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_INVALID_ADDR");
      break;

    case RUUVI_DRIVER_ERROR_BUSY: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_BUSY");
      break;

    case RUUVI_DRIVER_ERROR_RESOURCES: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_RESOURCES");
      break;

    case RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED");
      break;

    case RUUVI_DRIVER_ERROR_FATAL: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_FATAL");
      break;

    default: 
      written = snprintf(error_string, space_remaining, "%s", "RUUVI_DRIVER_ERROR_UNKNOWN");
      break;
  }
  return written;
}