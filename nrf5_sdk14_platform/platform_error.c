/**
 * Convert NRF SDK errors to ruuvi errors
 */

#include "sdk_application_config.h"
#ifdef NRF5_SDK14_PLATFORM
#include "ruuvi_error.h"
#include "sdk_errors.h"

// NRF_ERROR_RESOURCES is not defined in nosd version of nrf_error.h
#ifndef NRF_ERROR_RESOURCES
 #define NRF_ERROR_RESOURCES (NRF_ERROR_BASE_NUM + 19)
#endif 

ruuvi_status_t platform_to_ruuvi_error(void* error)
{
  ret_code_t err_code = *(ret_code_t*)error;
  if(NRF_SUCCESS == err_code)              { return RUUVI_SUCCESS; }
  if(NRF_ERROR_INTERNAL == err_code)       { return RUUVI_ERROR_INTERNAL; }
  if(NRF_ERROR_NO_MEM == err_code)         { return RUUVI_ERROR_NO_MEM; }
  if(NRF_ERROR_NOT_FOUND == err_code)      { return RUUVI_ERROR_NOT_FOUND; }
  if(NRF_ERROR_NOT_SUPPORTED == err_code)  { return RUUVI_ERROR_NOT_SUPPORTED; }
  if(NRF_ERROR_INVALID_PARAM == err_code)  { return RUUVI_ERROR_INVALID_PARAM; }
  if(NRF_ERROR_INVALID_STATE == err_code)  { return RUUVI_ERROR_INVALID_STATE; }
  if(NRF_ERROR_INVALID_LENGTH == err_code) { return RUUVI_ERROR_INVALID_LENGTH; }
  if(NRF_ERROR_INVALID_FLAGS == err_code)  { return RUUVI_ERROR_INVALID_FLAGS; }
  if(NRF_ERROR_DATA_SIZE == err_code)      { return RUUVI_ERROR_DATA_SIZE; }
  if(NRF_ERROR_TIMEOUT == err_code)        { return RUUVI_ERROR_TIMEOUT; }
  if(NRF_ERROR_NULL == err_code)           { return RUUVI_ERROR_NULL; }
  if(NRF_ERROR_FORBIDDEN == err_code)      { return RUUVI_ERROR_FORBIDDEN; }
  if(NRF_ERROR_INVALID_ADDR == err_code)   { return RUUVI_ERROR_INVALID_ADDR; }
  if(NRF_ERROR_BUSY == err_code)           { return RUUVI_ERROR_BUSY; }
  if(NRF_ERROR_RESOURCES == err_code)      { return RUUVI_ERROR_RESOURCES; }
  return RUUVI_ERROR_INTERNAL;
}

#endif