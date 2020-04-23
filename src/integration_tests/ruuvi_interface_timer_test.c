#include "ruuvi_interface_timer_test.h"
#include "ruuvi_interface_timer.h"
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_yield.h"
#include <stddef.h>
#include <string.h>
/**
 * @addtogroup timer
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_timer_test.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-19
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Test interface functions to timer.
 *
 */

static char test_output[20];
static uint8_t executions;

static void test_handler (void * context)
{
    test_output[strlen (test_output)] = ( (char *) context) [0];
    executions++;
}

/**
 * @brief Test timer initialization.
 *
 *
 * @param[in] printfp Function to which test JSON is passed.
 *
 * @retval false if no errors occured.
 * @retval true if error occured.
 */
static bool ri_timer_init_test (const rd_test_print_fp printfp)
{
    printfp ("\"init\":");
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    err_code = ri_timer_init();

    if (RD_SUCCESS != err_code)
    {
        status = true;
    }
    else
    {
        // Dual-init.
        err_code = ri_timer_init();

        if (RD_ERROR_INVALID_STATE != err_code)
        {
            status = true;
        }

        err_code = ri_timer_uninit();
        err_code |= ri_timer_init();

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

    ri_timer_uninit();
    return status;
}

/**
 * @brief Create new timer instance.
 *
 * Verify that timer instances are freed after uninit.
 *
 * @param[in] printfp Function to which test JSON is passed.
 *
 * @retval false if no errors occured.
 * @retval true if error occured.
 */
static bool ri_timer_create_test (const rd_test_print_fp printfp)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"create\":");
    err_code |= ri_timer_init();
    ri_timer_id_t tids[RI_TIMER_MAX_INSTANCES];
    err_code |= ri_timer_create (&tids[0], RI_TIMER_MODE_SINGLE_SHOT, &test_handler);

    if (RD_SUCCESS != err_code)
    {
        status = true;
    }
    else
    {
        // Allocation should fail at max instances as one instance is allocated and not
        // released already.
        for (size_t ii = 0; ii < RI_TIMER_MAX_INSTANCES; ii++)
        {
            err_code |= ri_timer_create (&tids[ii], RI_TIMER_MODE_SINGLE_SHOT, &test_handler);
        }

        if (RD_ERROR_RESOURCES != err_code)
        {
            status |= true;
        }

        err_code = ri_timer_uninit();
        err_code |= ri_timer_create (&tids[0], RI_TIMER_MODE_SINGLE_SHOT, &test_handler);

        if (RD_ERROR_INVALID_STATE != err_code)
        {
            status |= true;
        }

        err_code = ri_timer_init();

        for (size_t ii = 0; ii < RI_TIMER_MAX_INSTANCES; ii++)
        {
            err_code |= ri_timer_create (&tids[ii], RI_TIMER_MODE_SINGLE_SHOT, &test_handler);
        }

        if (RD_SUCCESS != err_code)
        {
            status |= true;
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

    ri_timer_uninit();
    return status;
}

/**
 * @brief Test single mode.
 *
 * @param[in] printfp Function to which test JSON is passed.
 *
 * @retval false if no errors occured.
 * @retval true if error occured.
 */
static bool ri_timer_single_shot_test (const rd_test_print_fp printfp)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    executions = 0;
    char data[] = "0123456789ABCDEF";
    printfp ("\"single_shot\":");
    char expected_output[RI_TIMER_MAX_INSTANCES + 1] = {0};
    memcpy (expected_output, data, RI_TIMER_MAX_INSTANCES);
    ri_timer_id_t tids[RI_TIMER_MAX_INSTANCES];
    err_code |= ri_timer_init();

    for (size_t ii = 0; ii < RI_TIMER_MAX_INSTANCES; ii++)
    {
        err_code |= ri_timer_create (&tids[ii], RI_TIMER_MODE_SINGLE_SHOT, &test_handler);
        err_code |= ri_timer_start (tids[ii], (10U * ii) + 10U, &data[ii]);
    }

    if (RD_SUCCESS != err_code)
    {
        status |= true;
    }

    ri_delay_ms (10 * (RI_TIMER_MAX_INSTANCES + 1));

    if (strcmp (expected_output, test_output))
    {
        status = true;
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    err_code |= ri_timer_uninit();
    return status;
}

/**
 * @brief Test repeat mode.
 *
 * @param[in] printfp Function to which test JSON is passed.
 *
 * @retval false if no errors occured.
 * @retval true if error occured.
 */
static bool ri_timer_repeat_test (const rd_test_print_fp printfp)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    executions = 0;
    char data[] = "ABC";
    printfp ("\"repeat\":");
    char expected_output[] = "ABCABACBACBA";
    memset (test_output, 0, sizeof (test_output));
    ri_timer_id_t tids[RI_TIMER_MAX_INSTANCES];
    err_code |= ri_timer_init();

    for (size_t ii = 0; ii < 3; ii++)
    {
        err_code |= ri_timer_create (&tids[ii], RI_TIMER_MODE_REPEATED, &test_handler);
    }

    err_code |= ri_timer_start (tids[0], 11U, &data[0]);
    err_code |= ri_timer_start (tids[1], 13U, &data[1]);
    err_code |= ri_timer_start (tids[2], 17U, &data[2]);

    if (RD_SUCCESS != err_code)
    {
        status |= true;
    }

    // 5 times A, 4 times B, 3 times C.
    ri_delay_ms (56U);
    err_code |= ri_timer_stop (tids[0]);
    err_code |= ri_timer_stop (tids[1]);
    err_code |= ri_timer_stop (tids[2]);

    if (strcmp (expected_output, test_output))
    {
        status = true;
    }

    if (status)
    {
        printfp ("\"fail\"\r\n");
    }
    else
    {
        printfp ("\"pass\"\r\n");
    }

    ri_timer_uninit();
    return status;
}

bool ri_timer_integration_test_run (const rd_test_print_fp printfp)
{
    printfp ("\"timer\":{\r\n");
    bool status = false;
    status |= ri_timer_init_test (printfp);
    status |= ri_timer_create_test (printfp);
    status |= ri_timer_single_shot_test (printfp);
    status |= ri_timer_repeat_test (printfp);
    printfp ("},\r\n");
    return status;
}
#endif
