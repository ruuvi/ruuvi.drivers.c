#include "ruuvi_interface_scheduler_test.h"
#include "ruuvi_interface_scheduler.h"
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include <stddef.h>
#include <string.h>

/**
 * @addtogroup scheduler
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_scheduler_test.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-14
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Test interface functions to scheduler.
 *
 */

static char test_output[20];
static uint8_t executions;

static void test_handler (void * p_event_data, uint16_t event_size)
{
    memcpy (test_output, p_event_data, event_size);
    executions++;
}

/**
 * @brief Test scheduler initialization.
 *
 * Allocates memory for scheduler task queue if dynamically allocated,
 * verifies expected size if statically allocated.
 *
 * @param[in] printfp Function to which test JSON is passed.
 *
 * @retval false if no errors occured.
 * @retval true if error occured.
 */
static bool ri_scheduler_init_test (const rd_test_print_fp printfp)
{
    printfp ("\"init\":");
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    err_code = ri_scheduler_init ();

    if (RD_SUCCESS != err_code)
    {
        status |= true;
    }
    else
    {
        // Verify init check after init.
        if (!ri_scheduler_is_init())
        {
            status |= true;
        }

        // Verify that events are discarded on uninit
        ri_scheduler_event_put (NULL, 0, test_handler);
        ri_scheduler_uninit();

        // Verify init check after uninit.
        if (ri_scheduler_is_init())
        {
            status |= true;
        }

        err_code = ri_scheduler_init();
        ri_scheduler_execute();

        if (executions)
        {
            status = true;
        }

        // Dual-init.
        err_code = ri_scheduler_init();

        if (RD_ERROR_INVALID_STATE != err_code)
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

    ri_scheduler_uninit();
    return status;
}

/**
 *  @brief Executes all scheduled tasks.
 *
 *  If task schedules itself to be run immediately this will be run in a
 *  never-ending loop, without sleeping.
 *
 * @param[in] printfp Function to which test JSON is passed.
 *
 * @retval false if no errors occured.
 * @retval true if error occured.
 */
static bool ri_scheduler_execute_test (const rd_test_print_fp printfp)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    char test_input[] = "Hello scheduler";
    printfp ("\"execute\":");
    err_code |= ri_scheduler_init();
    err_code |= ri_scheduler_event_put (&test_input, sizeof (test_input), test_handler);
    err_code |= ri_scheduler_event_put (NULL, 0, test_handler);
    err_code |= ri_scheduler_event_put (NULL, 0, test_handler);
    ri_scheduler_execute();

    if (RD_SUCCESS != err_code || strcmp (test_output, test_input) || (3 != executions))
    {
        status = true;
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    ri_scheduler_uninit();
    return status;
}

/**
 * @brief Test puttinh functions to scheduler.
 *
 * @param[in] printfp Function to which test JSON is passed.
 *
 * @retval false if no errors occured.
 * @retval true if error occured.
 */
static bool ri_scheduler_event_put_test (const rd_test_print_fp printfp)
{
    printfp ("\"put\":");
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    // Verify that scheduler will end up filled.
    executions = 0;
    err_code |= ri_scheduler_init();

    for (size_t ii = 0; ii <= RI_SCHEDULER_LENGTH; ii++)
    {
        err_code |= ri_scheduler_event_put (NULL, 0, test_handler);
    }

    ri_scheduler_execute();

    if (RD_ERROR_NO_MEM != err_code || RI_SCHEDULER_LENGTH != executions)
    {
        status = true;
    }

    err_code = ri_scheduler_event_put (NULL, RI_SCHEDULER_SIZE + 1, test_handler);

    if (RD_ERROR_INVALID_LENGTH != err_code)
    {
        status = true;
    }

    err_code = ri_scheduler_event_put (NULL, 0, NULL);

    if (RD_ERROR_NULL != err_code)
    {
        status = true;
    }

    ri_scheduler_uninit();
    err_code = ri_scheduler_event_put (NULL, 0, test_handler);

    if (RD_ERROR_INVALID_STATE != err_code)
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

    return status;
}

bool ri_scheduler_run_integration_test (const rd_test_print_fp printfp)
{
    printfp ("\"scheduler\":{\r\n");
    bool status = false;
    status |= ri_scheduler_init_test (printfp);
    status |= ri_scheduler_execute_test (printfp);
    status |= ri_scheduler_event_put_test (printfp);
    printfp ("},\r\n");
    return status;
}
#endif
