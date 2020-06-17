#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_yield.h"
#include <stdbool.h>

#define PULL_DELAY_MS (1U)

rd_status_t ri_gpio_test_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // - Interface must return RD_SUCCESS after first call.
    err_code = ri_gpio_init();

    if (RD_SUCCESS != err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        return RD_ERROR_SELFTEST;
    }

    // - Interface must return RD_ERROR_INVALID_STATE when called while already initialized.
    err_code = ri_gpio_init();

    if (RD_SUCCESS == err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        return RD_ERROR_SELFTEST;
    }

    // - Interface must return RD_SUCCESS when called after uninitialization.
    err_code = ri_gpio_uninit();

    if (RD_SUCCESS != err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        return RD_ERROR_SELFTEST;
    }

    err_code = ri_gpio_uninit();

    if (RD_SUCCESS != err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        return RD_ERROR_SELFTEST;
    }

    return RD_SUCCESS;
}

rd_status_t ri_gpio_test_configure (const ri_gpio_id_t input,
                                    const ri_gpio_id_t output)
{
    rd_status_t status = RD_SUCCESS;
    // * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
    ri_gpio_state_t state;
    status |= ri_gpio_init();
    status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_NOPULL);
    status |= ri_gpio_configure (output, RI_GPIO_MODE_INPUT_PULLUP);
    status |= ri_delay_ms (PULL_DELAY_MS);
    status |= ri_gpio_read (input, &state);

    if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        status = RD_ERROR_SELFTEST;
    }
    else
    {
        // - When Input is in High-Z mode, and output mode is INPUT_PULLDOWN, input must read as LOW
        status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_NOPULL);
        status |= ri_gpio_configure (output,
                                     RI_GPIO_MODE_INPUT_PULLDOWN);
        status |= ri_delay_ms (PULL_DELAY_MS);
        status |= ri_gpio_read (input, &state);

        if (RD_SUCCESS != status || RI_GPIO_LOW != state)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            status = RD_ERROR_SELFTEST;
        }

        // - When Input is in INPUT_PULLUP mode, and output is in OUTPUT_LOW mode, input must read as LOW
        status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_PULLUP);
        status |= ri_gpio_configure (output,
                                     RI_GPIO_MODE_OUTPUT_STANDARD);
        status |= ri_gpio_write (output, RI_GPIO_LOW);
        status |= ri_gpio_read (input, &state);

        if (RD_SUCCESS != status || RI_GPIO_LOW != state)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            status = RD_ERROR_SELFTEST;
        }

        // - When Input is in INPUT_PULLDOWN mode, and output is in OUTPUT_HIGH mode, input must read as HIGH
        status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_PULLDOWN);
        status |= ri_gpio_configure (output, RI_GPIO_MODE_OUTPUT_STANDARD);
        status |= ri_gpio_write (output, RI_GPIO_HIGH);
        status |= ri_gpio_read (input, &state);

        if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            status = RD_ERROR_SELFTEST;
        }
    }

    status |= ri_gpio_uninit();
    return status;
}

rd_status_t ri_gpio_test_toggle (const ri_gpio_id_t input,
                                 const ri_gpio_id_t output)
{
    rd_status_t status = RD_SUCCESS;
    // * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
    ri_gpio_state_t state;
    status |= ri_gpio_init();
    status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_NOPULL);
    status |= ri_gpio_configure (output, RI_GPIO_MODE_OUTPUT_STANDARD);
    status |= ri_gpio_write (output, RI_GPIO_LOW);
    status |= ri_gpio_read (input, &state);

    // Verify our start state
    if (RD_SUCCESS != status || RI_GPIO_LOW != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        status = RD_ERROR_SELFTEST;
    }
    else
    {
        // Verify low-to-high
        status |= ri_gpio_toggle (output);
        status |= ri_gpio_read (input, &state);

        if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            status = RD_ERROR_SELFTEST;
        }

        // Verify high-to-low
        status |= ri_gpio_toggle (output);
        status |= ri_gpio_read (input, &state);

        if (RD_SUCCESS != status || RI_GPIO_LOW != state)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            status = RD_ERROR_SELFTEST;
        }

        // Verify second low-to-high (after toggle, not static set)
        status |= ri_gpio_toggle (output);
        status |= ri_gpio_read (input, &state);

        if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            status = RD_ERROR_SELFTEST;
        }
    }

    status |= ri_gpio_uninit();
    return status;
}

bool ri_gpio_run_integration_test (const rd_test_print_fp printfp,
                                   const ri_gpio_id_t input, const ri_gpio_id_t output)
{
    rd_status_t status;
    printfp ("\"gpio\":{\r\n");
    printfp ("\"init\":");
    status = ri_gpio_test_init();

    if (RD_SUCCESS == status)
    {
        printfp ("\"pass\",\r\n");
    }
    else
    {
        printfp ("\"fail\",\r\n");
    }

    printfp ("\"configure\":");
    status = ri_gpio_test_configure (input, output);

    if (RD_SUCCESS == status)
    {
        printfp ("\"pass\",\r\n");
    }
    else
    {
        printfp ("\"fail\",\r\n");
    }

    printfp ("\"toggle\":");
    status = ri_gpio_test_toggle (input, output);

    if (RD_SUCCESS == status)
    {
        printfp ("\"pass\"\r\n");
    }
    else
    {
        printfp ("\"fail\"\r\n");
    }

    printfp ("},\r\n");
    return status;
}

#endif
