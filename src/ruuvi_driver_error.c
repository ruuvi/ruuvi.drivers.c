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

#include <stdio.h>
#include <string.h>

/** Store all occured errors until errors are checked by application here */
static rd_status_t m_errors = RD_SUCCESS;

/** Application callback on errors **/
static rd_error_cb m_cb;

void rd_error_check (rd_status_t error,
                     rd_status_t non_fatal_mask, const char * file, int line)
{
    // Do nothing on success
    if (RD_SUCCESS == error) { return; }

    m_errors |= error;
    char message[RD_LOG_BUFFER_SIZE];
    size_t index = 0;
    bool fatal = ~non_fatal_mask & error;
    // Cut out the full path
    const char * filename = strrchr (file, '/');

    // If on Windows
    if (NULL == filename) { filename = strrchr (file, '\\'); }

    // In case the file was already only the name
    if (NULL == filename) { filename = file; }
    // Otherwise skip the slash
    else { filename++; }

    // Reset on fatal error
    if (fatal)
    {
        index += snprintf (message, sizeof (message), "%s:%d FATAL: ", filename, line);
        index += ri_error_to_string (error, (message + index),
                                     (sizeof (message) - index));
        snprintf ( (message + index), (sizeof (message) - index), "\r\n");
        ri_log_flush();
        ri_log (RI_LOG_LEVEL_ERROR, message);
        ri_log_flush();
    }
    // Log non-fatal errors
    else if (RD_SUCCESS != error)
    {
        index += snprintf (message, sizeof (message), "%s:%d WARNING: ", filename, line);
        index += ri_error_to_string (error, (message + index),
                                     (sizeof (message) - index));
        snprintf ( (message + index), (sizeof (message) - index), "\r\n");
        ri_log (RI_LOG_LEVEL_WARNING, message);
    }

    // Call error callback
    if (RD_SUCCESS != error && NULL != m_cb)
    {
        m_cb (error, fatal, filename, line);
    }
}

/*
 * @brief reset global error flags and return their value.
 *
 * @return errors occured after last call to this function.
 */
rd_status_t rd_errors_clear()
{
    rd_status_t errors = m_errors;
    m_errors = RD_SUCCESS;
    return errors;
}

void rd_error_cb_set (rd_error_cb cb)
{
    m_cb = cb;
}

/** @} */
