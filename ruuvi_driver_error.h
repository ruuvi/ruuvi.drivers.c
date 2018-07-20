#ifndef RUUVI_DRIVER_ERROR_H
#define RUUVI_DRIVER_ERROR_H

#include <float.h>
#include <stdint.h>

#define RUUVI_DRIVER_FLOAT_INVALID FLT_MAX

#define RUUVI_DRIVER_SUCCESS 0
#define RUUVI_DRIVER_ERROR_INTERNAL        (1<<0)  ///< Internal Error
#define RUUVI_DRIVER_ERROR_NO_MEM          (1<<1)  ///< No Memory for operation
#define RUUVI_DRIVER_ERROR_NOT_FOUND       (1<<2)  ///< Not found
#define RUUVI_DRIVER_ERROR_NOT_SUPPORTED   (1<<3)  ///< Not supported
#define RUUVI_DRIVER_ERROR_INVALID_PARAM   (1<<4)  ///< Invalid Parameter
#define RUUVI_DRIVER_ERROR_INVALID_STATE   (1<<5)  ///< Invalid state, operation disallowed in this state
#define RUUVI_DRIVER_ERROR_INVALID_LENGTH  (1<<6)  ///< Invalid Length
#define RUUVI_DRIVER_ERROR_INVALID_FLAGS   (1<<7)  ///< Invalid Flags
#define RUUVI_DRIVER_ERROR_INVALID_DATA    (1<<8)  ///< Invalid Data
#define RUUVI_DRIVER_ERROR_DATA_SIZE       (1<<9)  ///< Invalid Data size
#define RUUVI_DRIVER_ERROR_TIMEOUT         (1<<10) ///< Operation timed out
#define RUUVI_DRIVER_ERROR_NULL            (1<<11) ///< Null Pointer
#define RUUVI_DRIVER_ERROR_FORBIDDEN       (1<<12) ///< Forbidden Operation
#define RUUVI_DRIVER_ERROR_INVALID_ADDR    (1<<13) ///< Bad Memory Address
#define RUUVI_DRIVER_ERROR_BUSY            (1<<14) ///< Busy
#define RUUVI_DRIVER_ERROR_RESOURCES       (1<<15) ///< Not enough resources for operation
#define RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED (1<<16) ///< Not implememnted yet
#define RUUVI_DRIVER_ERROR_SELFTEST        (1<<17) ///< Self-test fail

typedef int32_t ruuvi_driver_status_t;

/** Convert error code from platform to Ruuvi error code. **/
ruuvi_driver_status_t platform_to_ruuvi_error(void* error);

#endif