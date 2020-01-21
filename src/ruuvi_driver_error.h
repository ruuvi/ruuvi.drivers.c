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
#include <stdbool.h>
#include <stdint.h>

#define RD_FLOAT_INVALID  NAN            //!< Signal that value should not be used.
#define RD_UINT64_INVALID UINT64_MAX     //!< Signal that value should not be used.
#define RD_INT64_INVALID  INT64_MIN      //!< Signal that value should not be used.
#define RD_INT32_INVALID  INT32_MIN      //!< Signal that value should not be used.

#define RD_SUCCESS                0       ///< Internal Error
#define RD_ERROR_INTERNAL         (1<<0)  ///< Internal Error
#define RD_ERROR_NO_MEM           (1<<1)  ///< No Memory for operation
#define RD_ERROR_NOT_FOUND        (1<<2)  ///< Not found
#define RD_ERROR_NOT_SUPPORTED    (1<<3)  ///< Not supported
#define RD_ERROR_INVALID_PARAM    (1<<4)  ///< Invalid Parameter
#define RD_ERROR_INVALID_STATE    (1<<5)  ///< Invalid state, operation disallowed in this state
#define RD_ERROR_INVALID_LENGTH   (1<<6)  ///< Invalid Length
#define RD_ERROR_INVALID_FLAGS    (1<<7)  ///< Invalid Flags
#define RD_ERROR_INVALID_DATA     (1<<8)  ///< Invalid Data
#define RD_ERROR_DATA_SIZE        (1<<9)  ///< Invalid Data size
#define RD_ERROR_TIMEOUT          (1<<10) ///< Operation timed out
#define RD_ERROR_NULL             (1<<11) ///< Null Pointer
#define RD_ERROR_FORBIDDEN        (1<<12) ///< Forbidden Operation
#define RD_ERROR_INVALID_ADDR     (1<<13) ///< Bad Memory Address
#define RD_ERROR_BUSY             (1<<14) ///< Busy
#define RD_ERROR_RESOURCES        (1<<15) ///< Not enough resources for operation
#define RD_ERROR_NOT_IMPLEMENTED  (1<<16) ///< Not implemented yet
#define RD_ERROR_SELFTEST         (1<<17) ///< Self-test fail
#define RD_STATUS_MORE_AVAILABLE  (1<<18) ///< Driver has more data queued
#define RD_ERROR_NOT_INITIALIZED  (1<<19) ///< Driver is not initialized.
#define RD_ERROR_NOT_ACKNOWLEDGED (1<<20) ///< Ack was expected but not received
#define RD_ERROR_FATAL            (1<<31) ///< Program should always reset after this

typedef uint32_t rd_status_t; ///< bitfield for representing errors

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
void rd_error_check (const rd_status_t error,
                     const rd_status_t non_fatal_mask, const char * p_file, const int line);

/**
 * @brief Shorthand macro for calling the @ref rd_error_check with current file & line
 *
 * If error is considered fatal (or not non-fatal), reset the device
 * If the error is non-fatal, log an error on the console and return
 * Resetting requires that @c APPLICATION_POWER_ENABLED evaluates to true and is implemented on platform.
 * Logging requires that @c APPLICATION_LOG_ENABLED evaluates to true and is implemented on platform.
 *
 * @param[in] error error code, might have several flags in it.
 * @param[in] mask Signal that this error is acceptable for program flow and execution may continue.
 **/
#define RD_ERROR_CHECK(error, mask) rd_error_check(error, mask, __FILE__, __LINE__)

/*
 * @brief reset global error flags and return their value.
 *
 * @return errors occured after last call to this function.
 */
rd_status_t rd_errors_clear();

/* @brief Application callback on error
 *
 * If this callback is set, it will get called on error.
 *
 * @param[in] error Type of error
 * @param[in] fatal True if error is considered non-recoverable
 * @param[in] file  Pointer to name of file where error occured
 * @param[in] line  line where error occured
 */
typedef void (*rd_error_cb) (const rd_status_t error,
                             const bool fatal,
                             const char * file,
                             const int line);

/**
 * @brief Configure application callback for errors
 *
 * @param[in] cb Callback on error, NULL to clear callback
 */
void rd_error_cb_set (rd_error_cb cb);

/** @} */
#endif