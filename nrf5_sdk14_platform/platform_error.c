/**
 * Convert NRF SDK errors to ruuvi errors
 */

#include "sdk_application_config.h"
#ifdef NRF5_SDK14_PLATFORM
#include "ruuvi_error.h"
#include "sdk_errors.h"
#define NRF_SUCCESS                           (NRF_ERROR_BASE_NUM + 0)  ///< Successful command
#define NRF_ERROR_SVC_HANDLER_MISSING         (NRF_ERROR_BASE_NUM + 1)  ///< SVC handler is missing
#define NRF_ERROR_SOFTDEVICE_NOT_ENABLED      (NRF_ERROR_BASE_NUM + 2)  ///< SoftDevice has not been enabled
#define NRF_ERROR_INTERNAL                    (NRF_ERROR_BASE_NUM + 3)  ///< Internal Error
#define NRF_ERROR_NO_MEM                      (NRF_ERROR_BASE_NUM + 4)  ///< No Memory for operation
#define NRF_ERROR_NOT_FOUND                   (NRF_ERROR_BASE_NUM + 5)  ///< Not found
#define NRF_ERROR_NOT_SUPPORTED               (NRF_ERROR_BASE_NUM + 6)  ///< Not supported
#define NRF_ERROR_INVALID_PARAM               (NRF_ERROR_BASE_NUM + 7)  ///< Invalid Parameter
#define NRF_ERROR_INVALID_STATE               (NRF_ERROR_BASE_NUM + 8)  ///< Invalid state, operation disallowed in this state
#define NRF_ERROR_INVALID_LENGTH              (NRF_ERROR_BASE_NUM + 9)  ///< Invalid Length
#define NRF_ERROR_INVALID_FLAGS               (NRF_ERROR_BASE_NUM + 10) ///< Invalid Flags
#define NRF_ERROR_INVALID_DATA                (NRF_ERROR_BASE_NUM + 11) ///< Invalid Data
#define NRF_ERROR_DATA_SIZE                   (NRF_ERROR_BASE_NUM + 12) ///< Invalid Data size
#define NRF_ERROR_TIMEOUT                     (NRF_ERROR_BASE_NUM + 13) ///< Operation timed out
#define NRF_ERROR_NULL                        (NRF_ERROR_BASE_NUM + 14) ///< Null Pointer
#define NRF_ERROR_FORBIDDEN                   (NRF_ERROR_BASE_NUM + 15) ///< Forbidden Operation
#define NRF_ERROR_INVALID_ADDR                (NRF_ERROR_BASE_NUM + 16) ///< Bad Memory Address
#define NRF_ERROR_BUSY                        (NRF_ERROR_BASE_NUM + 17) ///< Busy
#define NRF_ERROR_CONN_COUNT                  (NRF_ERROR_BASE_NUM + 18) ///< Maximum connection count exceeded.
#define NRF_ERROR_RESOURCES                   (NRF_ERROR_BASE_NUM + 19) ///< Not enough resources for operation

#define RUUVI_SUCCESS 0
#define RUUVI_ERROR_INTERNAL        (1<<0)  ///< Internal Error
#define RUUVI_ERROR_NO_MEM          (1<<1)  ///< No Memory for operation
#define RUUVI_ERROR_NOT_FOUND       (1<<2)  ///< Not found
#define RUUVI_ERROR_NOT_SUPPORTED   (1<<3)  ///< Not supported
#define RUUVI_ERROR_INVALID_PARAM   (1<<4)  ///< Invalid Parameter
#define RUUVI_ERROR_INVALID_STATE   (1<<5)  ///< Invalid state, operation disallowed in this state
#define RUUVI_ERROR_INVALID_LENGTH  (1<<6)  ///< Invalid Length
#define RUUVI_ERROR_INVALID_FLAGS   (1<<7)  ///< Invalid Flags
#define RUUVI_ERROR_INVALID_DATA    (1<<8)  ///< Invalid Data
#define RUUVI_ERROR_DATA_SIZE       (1<<9)  ///< Invalid Data size
#define RUUVI_ERROR_TIMEOUT         (1<<10) ///< Operation timed out
#define RUUVI_ERROR_NULL            (1<<11) ///< Null Pointer
#define RUUVI_ERROR_FORBIDDEN       (1<<12) ///< Forbidden Operation
#define RUUVI_ERROR_INVALID_ADDR    (1<<13) ///< Bad Memory Address
#define RUUVI_ERROR_BUSY            (1<<14) ///< Busy
#define RUUVI_ERROR_RESOURCES       (1<<15) ///< Not enough resources for operation
#define RUUVI_ERROR_NOT_IMPLEMENTED (1<<16) ///< Not implememnted yet

ruuvi_status_t PLATFORM_TO_RUUVI_ERROR(void* error)
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