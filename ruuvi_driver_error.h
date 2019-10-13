#ifndef RUUVI_DRIVER_ERROR_H
#define RUUVI_DRIVER_ERROR_H
/**
 * @defgroup Error Error reporting and handling
 * @brief Functions and definitions for errors and error handling
 *
 *
 */
/*@{*/
/**
 * @file ruuvi_driver_error.h
 * @author Otso Jousimaa
 * @date 2019-02-17
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause
 * @brief Ruuvi error codes and error check function
 *
 */

#include <float.h>
#include <math.h>
#include <stdint.h>

#define RUUVI_DRIVER_FLOAT_INVALID  NAN            //!< Signal that value should not be used.
#define RUUVI_DRIVER_UINT64_INVALID UINT64_MAX     //!< Signal that value should not be used.
#define RUUVI_DRIVER_INT64_INVALID  INT64_MIN      //!< Signal that value should not be used.
#define RUUVI_DRIVER_INT32_INVALID  INT32_MIN      //!< Signal that value should not be used.

#define RUUVI_DRIVER_SUCCESS                0       ///< Internal Error
#define RUUVI_DRIVER_ERROR_INTERNAL         (1<<0)  ///< Internal Error
#define RUUVI_DRIVER_ERROR_NO_MEM           (1<<1)  ///< No Memory for operation
#define RUUVI_DRIVER_ERROR_NOT_FOUND        (1<<2)  ///< Not found
#define RUUVI_DRIVER_ERROR_NOT_SUPPORTED    (1<<3)  ///< Not supported
#define RUUVI_DRIVER_ERROR_INVALID_PARAM    (1<<4)  ///< Invalid Parameter
#define RUUVI_DRIVER_ERROR_INVALID_STATE    (1<<5)  ///< Invalid state, operation disallowed in this state
#define RUUVI_DRIVER_ERROR_INVALID_LENGTH   (1<<6)  ///< Invalid Length
#define RUUVI_DRIVER_ERROR_INVALID_FLAGS    (1<<7)  ///< Invalid Flags
#define RUUVI_DRIVER_ERROR_INVALID_DATA     (1<<8)  ///< Invalid Data
#define RUUVI_DRIVER_ERROR_DATA_SIZE        (1<<9)  ///< Invalid Data size
#define RUUVI_DRIVER_ERROR_TIMEOUT          (1<<10) ///< Operation timed out
#define RUUVI_DRIVER_ERROR_NULL             (1<<11) ///< Null Pointer
#define RUUVI_DRIVER_ERROR_FORBIDDEN        (1<<12) ///< Forbidden Operation
#define RUUVI_DRIVER_ERROR_INVALID_ADDR     (1<<13) ///< Bad Memory Address
#define RUUVI_DRIVER_ERROR_BUSY             (1<<14) ///< Busy
#define RUUVI_DRIVER_ERROR_RESOURCES        (1<<15) ///< Not enough resources for operation
#define RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED  (1<<16) ///< Not implemented yet
#define RUUVI_DRIVER_ERROR_SELFTEST         (1<<17) ///< Self-test fail
#define RUUVI_DRIVER_STATUS_MORE_AVAILABLE  (1<<18) ///< Driver has more data queued
#define RUUVI_DRIVER_ERROR_NOT_INITIALIZED  (1<<19) ///< Driver is not initialized.
#define RUUVI_DRIVER_ERROR_NOT_ACKNOWLEDGED (1<<20) ///< Ack was expected but not received
#define RUUVI_DRIVER_ERROR_FATAL            (1<<31) ///< Program should always reset after this

typedef int32_t ruuvi_driver_status_t; ///< bitfield for representing errors

/**
 * @brief Check given error code and compare it to non-fatal errors.
 *
 * If error is considered fatal (or not non-fatal), reset the device
 * If the error is non-fatal, log an error on the console and return
 * Resetting requires that @c APPLICATION_POWER_ENABLED evaluates to true and is implemented on platform.
 * Logging requires that @c APPLICATION_LOG_ENABLED evaluates to true and is implemented on platform.
 *
 * @param[in] error error code, might have several flags in it.
 * @param[in] non_fatal_mask Signal that this error is acceptable for program flow and execution may continue.
 * @param[in] p_file file from which function was called
 * @param[in] line line from which the function was called
 **/
void ruuvi_driver_error_check(const ruuvi_driver_status_t error,
                              const ruuvi_driver_status_t non_fatal_mask, const char* p_file, const int line);

/**
 * @brief Shorthand macro for calling the @ref ruuvi_driver_error_check with current file & line
 *
 * If error is considered fatal (or not non-fatal), reset the device
 * If the error is non-fatal, log an error on the console and return
 * Resetting requires that @c APPLICATION_POWER_ENABLED evaluates to true and is implemented on platform.
 * Logging requires that @c APPLICATION_LOG_ENABLED evaluates to true and is implemented on platform.
 *
 * @param[in] error error code, might have several flags in it.
 * @param[in] mask Signal that this error is acceptable for program flow and execution may continue.
 **/
#define RUUVI_DRIVER_ERROR_CHECK(error, mask) ruuvi_driver_error_check(error, mask, __FILE__, __LINE__)

/*
 * @brief reset global error flags and return their value.
 *
 * @return errors occured after last call to this function.
 */
ruuvi_driver_status_t ruuvi_driver_errors_clear();

/** @} */
#endif