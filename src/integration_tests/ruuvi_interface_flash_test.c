#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_flash_test.h"
#include "ruuvi_interface_flash.h"
#include "ruuvi_interface_power.h"
#include <stdbool.h>
#include <string.h>
/**
 * @addtogroup Flash
 * @{
 */
/**
 * @file ruuvi_interface_flash_test.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-11
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Integration test flash module implementation.
 */
#define F_BIG_RECORD_SIZE 0x900U
#define F_TEST_PAGE   0x0002U
#define F_TEST_RECORD 0x0001U
static const char  __attribute__ ( (aligned (4))) f_data1[] = "Flash test data 1";
static const char  __attribute__ ( (aligned (4))) f_data2[] = "Flash test data 2";
static uint32_t boot_count = 0;

/**
 * @brief Test flash initialization.
 *
 * Flash must initialize successfully on first try.
 * Flash must return RD_ERROR_INVALID_STATE on second try.
 *
 * @param[in] printfp Function pointer to which test result is printed.
 * @return false if test had no errors, true otherwise.
 */
static bool ri_flash_init_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    printfp ("\"init\":");
    err_code = ri_flash_init();

    if (RD_SUCCESS != err_code)
    {
        status = true;
        ri_flash_purge();
        ri_power_reset();
    }
    else
    {
        err_code = ri_flash_init();

        if (RD_ERROR_INVALID_STATE != err_code)
        {
            status = true;
        }
    }

    err_code |= ri_flash_record_get (APP_FLASH_LOG_FILE, APP_FLASH_LOG_BOOT_COUNTER_RECORD,
                                     sizeof (uint32_t), &boot_count);

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    return status;
}

/**
 * @brief test flash uninitialization.
 *
 * Uninitialization must always be successful.
 * Initialization must be successful after uninitialization.
 *
 * store, load, free, gc must return RD_ERROR_NOT_INITIALIZED after uninitialization.
 * busy must return false after uninitialization.
 *
 * @param[in] printfp Function pointer to which test result is printed
 * @return false if test had no errors, true otherwise.
 */
static bool ri_flash_uninit_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    printfp ("\"uninit\":");
    err_code = ri_flash_uninit();

    if (RD_SUCCESS != err_code)
    {
        status = true;
    }
    else
    {
        err_code = ri_flash_init();
        err_code |= ri_flash_uninit();
        err_code = ri_flash_init();

        if (RD_SUCCESS != err_code)
        {
            status = true;
        }
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    return status;
}

/**
 * @brief Test storing to flash.
 *
 * @param[in] printfp Function pointer to which test result is printed
 * @return false if test had no errors, true otherwise.
 */
static bool ri_flash_store_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    printfp ("\"store\":");
    err_code = ri_flash_uninit();
    err_code = ri_flash_record_set (F_TEST_PAGE, F_TEST_RECORD, sizeof (f_data1), f_data1);

    if (RD_SUCCESS == err_code)
    {
        status = true;
    }
    else
    {
        err_code = ri_flash_init();
        err_code |= ri_flash_record_set (F_TEST_PAGE, F_TEST_RECORD, sizeof (f_data1), f_data1);
        err_code |= ri_flash_record_set (F_TEST_PAGE, F_TEST_RECORD, sizeof (f_data2), f_data2);

        if (RD_SUCCESS != err_code)
        {
            status = true;
        }
        else
        {
            err_code |= ri_flash_record_set (F_TEST_PAGE, F_TEST_RECORD, UINT16_MAX, f_data2);

            if (RD_ERROR_DATA_SIZE != err_code)
            {
                status = true;
            }
        }
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    return status;
}

/**
 * @brief Test loading from flash.
 *
 * @param[in] printfp Function pointer to which test result is printed
 * @return false if test had no errors, true otherwise.
 */
static bool ri_flash_load_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    // Ensure we can align into 4-byte boundary.
    char load_buffer[sizeof (f_data2) + 4] = {0};
    printfp ("\"load\":");
    err_code = ri_flash_uninit();
    err_code |= ri_flash_record_get (F_TEST_PAGE, F_TEST_RECORD, sizeof (f_data1),
                                     load_buffer);

    if (RD_SUCCESS == err_code)
    {
        status = true;
    }
    else
    {
        err_code = ri_flash_init();
        err_code |= ri_flash_record_get (F_TEST_PAGE, F_TEST_RECORD, sizeof (load_buffer),
                                         load_buffer);

        if (RD_SUCCESS != err_code
                || strcmp (f_data2, load_buffer))
        {
            status = true;
        }
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    return status;
}

/**
 * @brief Test freeing from flash.
 *
 * @param[in] printfp Function pointer to which test result is printed
 * @return false if test had no errors, true otherwise.
 */
static bool ri_flash_delete_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    // Ensure we can align into 4-byte boundary.
    char load_buffer[sizeof (f_data2) + 4] = {0};
    printfp ("\"free\":");
    err_code = ri_flash_uninit();
    err_code |= ri_flash_record_delete (F_TEST_PAGE, F_TEST_RECORD);

    if (RD_SUCCESS == err_code)
    {
        status = true;
    }
    else
    {
        err_code = ri_flash_init();
        err_code |= ri_flash_record_delete (F_TEST_PAGE, F_TEST_RECORD);

        if (RD_SUCCESS != err_code)
        {
            status = true;
        }
        else
        {
            err_code |= ri_flash_record_get (F_TEST_PAGE, F_TEST_RECORD, sizeof (load_buffer),
                                             load_buffer);

            if (RD_ERROR_NOT_FOUND != err_code)
            {
                status = true;
            }
        }
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    (void) ri_flash_uninit();
    return status;
}

/**
 * @brief Test garbage collection by storing items until flash is out
 * of space and then garbage collect.
 *
 * Will also test size check.
 *
 * @param[in] printfp Function pointer to which test result is printed
 * @return false if test had no errors, true otherwise.
 */
static bool ri_flash_gc_size_busy_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    printfp ("\"gc\":");
    err_code = ri_flash_uninit();
    err_code |= ri_flash_gc_run();

    if (RD_SUCCESS == err_code)
    {
        status = true;
    }
    else
    {
        err_code = ri_flash_init();
        size_t size;

        // Test that checking free size works
        do
        {
            while (ri_flash_is_busy()) {};

            err_code = ri_flash_record_set (F_TEST_PAGE, F_TEST_RECORD, sizeof (f_data1), f_data1);

            err_code |= ri_flash_free_size_get (&size);
        } while ( ( (F_BIG_RECORD_SIZE < size) && (RD_SUCCESS == err_code))

                  || (RD_ERROR_BUSY == err_code));

        // Test that garbage collection works
        err_code = ri_flash_gc_run();

        while (ri_flash_is_busy()) {};

        err_code |= ri_flash_free_size_get (&size);

        if ( (RD_SUCCESS != err_code) || (F_BIG_RECORD_SIZE > size))
        {
            status = true;
        }
    }

    if (status)
    {
        printfp ("\"fail\"\r\n");
    }
    else
    {
        printfp ("\"pass\"\r\n");
    }

    err_code |= ri_flash_record_set (APP_FLASH_LOG_FILE, APP_FLASH_LOG_BOOT_COUNTER_RECORD,
                                     sizeof (uint32_t), &boot_count);
    err_code = ri_flash_uninit();
    return status;
}

/**
 * @brief Run all flash tests.
 *
 * @param[in] printfp Function pointer to which test result is printed.
 * @return false if test had no errors, true otherwise.
 **/
bool ri_flash_run_integration_test (const rd_test_print_fp printfp)
{
    bool status = false;
    printfp ("\"flash\":{\r\n");
    status |= ri_flash_init_test (printfp);
    status |= ri_flash_uninit_test (printfp);
    status |= ri_flash_store_test (printfp);
    status |= ri_flash_load_test (printfp);
    status |= ri_flash_delete_test (printfp);
    status |= ri_flash_gc_size_busy_test (printfp);
    printfp ("},\r\n");
    return status;
}

/* @} */
#endif
