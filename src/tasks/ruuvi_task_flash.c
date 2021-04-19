/**
 * @addtogroup flash_tasks
 */
/*@{*/
/**
 * @file ruuvi_task_flash.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @copyright 2019 Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @date 2019-11-18 provide compile time choice incase no flash is available
 * This module has 2 sets of code:
 *  If flash is enabled:
 *    on_error: In the event of a fatal error, the error, source file and line number are stored in an error file in flash.
 *              Then calls the bootloader, failing that reset.
 *    task_flash_init: calls print_error_cause which retrieves error file from flash and logs it(requires nRF52 DK board).
 *              Then sets up on_error as the call back error handler.
 *  If no flash:
 *    on_error: In the event of a fatal error calls the bootloader, failing that reset.
 *    task_flash_init: sets up on_error as the call back error handler.
 * These are in addition to flash utility functions of load, free, is busy and gc_run(which yields until not busy)
 */

#include "ruuvi_driver_enabled_modules.h"
#if RT_FLASH_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_flash.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_power.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_task_flash.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#ifndef TASK_FLASH_LOG_LEVEL
#define TASK_FLASH_LOG_LEVEL RI_LOG_LEVEL_INFO
#endif

#ifndef RT_FLASH_ERROR_FILE
#  define RT_FLASH_ERROR_FILE 0xBFFE
#endif

#ifndef RT_FLASH_ERROR_RECORD
#  define RT_FLASH_ERROR_RECORD 0xBFFE
#endif

#define LOG(msg) ri_log(TASK_FLASH_LOG_LEVEL, msg)
#define LOGD(msg) ri_log(RI_LOG_DEBUG, msg)
#define LOGW(msg) ri_log(RI_LOG_WARNING, msg)
#define LOGHEX(msg, len) ri_log_hex(TASK_FLASH_LOG_LEVEL, msg, len)

typedef struct
{
    rd_status_t error;
    char filename[32];
    int line;
} rt_flash_error_cause_t;

#if 0
static void on_error (const rd_status_t err,
                      const bool fatal,
                      const char * file,
                      const int line)
{
    if (!fatal)
    {
        return;
    }

    error_cause_t error = {.error = err, .line = line };
    rd_status_t err_code;
    uint32_t timeout = 0;
    strncpy (error.filename, file, sizeof (error.filename));
    // Store reason of fatal error
    err_code = rt_flash_store (APPLICATION_FLASH_ERROR_FILE,
                               APPLICATION_FLASH_ERROR_RECORD,
                               &error, sizeof (error));

    // Wait for flash store op to complete
    while (RD_SUCCESS == err_code &&
            timeout < 1000 &&
            ri_flash_is_busy())
    {
        timeout++;
        // Use microsecond wait to busyloop instead of millisecond wait to low-power sleep
        // as low-power sleep may hang on interrupt context.
        ri_delay_us (1000);
    }

    // Try to enter bootloader, if that fails reset.
    ri_power_enter_bootloader();
    ri_power_reset();
}
#endif

#ifndef CEEDLING
static
#endif
void print_error_cause (void)
{
    rt_flash_error_cause_t error;
    rd_status_t err_code;
    err_code = rt_flash_load (RT_FLASH_ERROR_FILE,
                              RT_FLASH_ERROR_RECORD,
                              &error, sizeof (error));

    if (RD_SUCCESS == err_code)
    {
        char error_str[128];
        size_t index = 0;
        index += snprintf (error_str, sizeof (error_str), "Previous fatal error: %s:%d: ",
                           error.filename, error.line);
        index += ri_error_to_string (error.error, error_str + index,
                                     sizeof (error_str) - index);
        snprintf (error_str + index,  sizeof (error_str) - index, "\r\n");
        LOG (error_str);
    }
}

rd_status_t rt_flash_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_flash_init();

    // Error on flash? purge, reboot
    if (RD_SUCCESS != err_code)
    {
        ri_flash_purge();
        ri_power_reset();
        // Stop test execution here.
#       ifdef CEEDLING
        return err_code;
#       endif
    }

    // Print previous fatal error
    print_error_cause();
    return err_code;
}

rd_status_t rt_flash_store (const uint16_t page_id, const uint16_t record_id,
                            const void * const message, const size_t message_length)
{
    rd_status_t status = RD_SUCCESS;
    status = ri_flash_record_set (page_id, record_id, message_length, message);

    if (RD_ERROR_NO_MEM == status)
    {
        ri_flash_gc_run();

        while (rt_flash_busy())
        {
            ri_yield();
        }

        status = ri_flash_record_set (page_id, record_id, message_length, message);
    }

    return status;
}

rd_status_t rt_flash_load (const uint16_t page_id, const uint16_t record_id,
                           void * const message, const size_t message_length)
{
    return ri_flash_record_get (page_id, record_id, message_length, message);
}

rd_status_t rt_flash_free (const uint16_t file_id, const uint16_t record_id)
{
    return ri_flash_record_delete (file_id, record_id);
}

rd_status_t rt_flash_gc_run (void)
{
    return ri_flash_gc_run();
}

bool rt_flash_busy (void)
{
    return ri_flash_is_busy();
}



#else

#include "ruuvi_driver_error.h"
#include <stdlib.h>
rd_status_t rt_flash_init (void)
{
    // Setup error reset
    return RD_SUCCESS;
}

rd_status_t rt_flash_store (const uint16_t file_id, const uint16_t record_id,
                            const void * const message, const size_t message_length)
{
    return RD_SUCCESS;
}

rd_status_t rt_flash_load (const uint16_t page_id, const uint16_t record_id,
                           void * const message, const size_t message_length)
{
    return RD_ERROR_NOT_FOUND;
}

bool rt_flash_busy (void)
{
    return false;
}
#endif
/*@}*/
