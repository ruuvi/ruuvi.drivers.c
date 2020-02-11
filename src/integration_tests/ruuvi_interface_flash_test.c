#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_flash_test.h"
#include "ruuvi_interface_flash.h"
#include <stdbool.h>
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

/**
 * @brief Run all flash tests.
 *
 * @param[in] printfp Function pointer to which test result is printed.
 * @return false if test had no errors, true otherwise.
 **/
bool ri_flash_run_integration_test (const rd_test_print_fp printfp)
{
    bool status = false;
    printfp ("'flash':{\r\n");
    status |= ri_flash_init_test (printfp);
    status |= ri_flash_uninit_test (printfp);
    printfp ("}\r\n");
}


/**
 * @brief Test flash initialization.
 *
 * Flash must initialize successfully on first try.
 * Flash must return RD_ERROR_INVALID_STATE on second try.
 *
 * @param[in] printfp Function pointer to which test result is printed.
 * @return false if test had no errors, true otherwise.
 */
bool ri_flash_init_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    printfp ("'init':");
    err_code = ri_flash_init();

    if (RD_SUCCESS != err_code)
    {
        status = true;
    }
    else
    {
        err_code = ri_flash_init();

        if (RD_ERROR_INVALID_STATE != err_code)
        {
            status = true;
        }
    }

    if (status)
    {
        printfp ("'fail',\r\n");
    }
    else
    {
        printfp ("'pass',\r\n");
    }

    return err_code;
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
bool ri_flash_uninit_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    printfp ("'uninit':");
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
        printfp ("'fail',\r\n");
    }
    else
    {
        printfp ("'pass',\r\n");
    }

    return err_code;
}
/* @} */
