/**
 * Ruuvi error codes and error check function
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/

#ifndef RUUVI_DRIVER_ERROR_H
#define RUUVI_DRIVER_ERROR_H

#include <float.h>
#include <stdint.h>

#define RUUVI_DRIVER_FLOAT_INVALID  FLT_MAX
#define RUUVI_DRIVER_UINT64_INVALID UINT64_MAX

#define RUUVI_DRIVER_SUCCESS               0
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
#define RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED (1<<16) ///< Not implemented yet
#define RUUVI_DRIVER_ERROR_SELFTEST        (1<<17) ///< Self-test fail
#define RUUVI_DRIVER_STATUS_MORE_AVAILABLE (1<<18) ///< Driver has more data queued
#define RUUVI_DRIVER_ERROR_FATAL           (1<<31) ///< Program should always reset after this

typedef int32_t ruuvi_driver_status_t;

/**
 * Check given error code and compare it to non-fatal errors.
 *
 * If error is considered fatal (or not non-fatal), reset the device
 * If the error is non-fatal, log an error on the console and return
 *
 * parameter error: error code, might have several flags in it.
 * parameter non_fatal_mask: Signal that this error is acceptable for program flow and execution may continue.
 * parameter file: file from which function was called
 * parameter line: line from which the function was called
 **/
void ruuvi_driver_error_check(ruuvi_driver_status_t error, ruuvi_driver_status_t non_fatal_mask, const char* file, int line);

// Shorthand macro for calling the error check function with current file & line
#define RUUVI_DRIVER_ERROR_CHECK(error, mask) ruuvi_driver_error_check(error, mask, __FILE__, __LINE__)

#endif