#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include <stdbool.h>

rd_status_t ri_gpio_test_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    // - Interface must return RD_SUCCESS after first call.
    err_code = ri_gpio_init();

    if (RD_SUCCESS != err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    // - Interface must return RD_ERROR_INVALID_STATE when called while already initialized.
    err_code = ri_gpio_init();

    if (RD_SUCCESS == err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    // - Interface must return RD_SUCCESS when called after uninitialization.
    err_code = ri_gpio_uninit();

    if (RD_SUCCESS != err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    err_code = ri_gpio_uninit();

    if (RD_SUCCESS != err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    return RD_SUCCESS;
}

rd_status_t ri_gpio_test_configure (const ri_gpio_id_t
                                    input,
                                    const ri_gpio_id_t output)
{
    rd_status_t status = RD_SUCCESS;
    // * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
    ri_gpio_state_t state;
    status |= ri_gpio_init();
    status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_NOPULL);
    status |= ri_gpio_configure (output, RI_GPIO_MODE_INPUT_PULLUP);
    status |= ri_gpio_read (input, &state);

    if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    // - When Input is in High-Z mode, and output mode is INPUT_PULLDOWN, input must read as LOW
    status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_NOPULL);
    status |= ri_gpio_configure (output,
                                 RI_GPIO_MODE_INPUT_PULLDOWN);
    status |= ri_gpio_read (input, &state);

    if (RD_SUCCESS != status || RI_GPIO_LOW != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    // - When Input is in INPUT_PULLUP mode, and output is in OUTPUT_LOW mode, input must read as LOW
    status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_PULLUP);
    status |= ri_gpio_configure (output,
                                 RI_GPIO_MODE_OUTPUT_STANDARD);
    status |= ri_gpio_write (output, RI_GPIO_LOW);
    status |= ri_gpio_read (input, &state);

    if (RD_SUCCESS != status || RI_GPIO_LOW != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    // - When Input is in INPUT_PULLDOWN mode, and output is in OUTPUT_HIGH mode, input must read as HIGH
    status |= ri_gpio_configure (input,
                                 RI_GPIO_MODE_INPUT_PULLDOWN);
    status |= ri_gpio_configure (output,
                                 RI_GPIO_MODE_OUTPUT_STANDARD);
    status |= ri_gpio_write (output, RI_GPIO_HIGH);
    status |= ri_gpio_read (input, &state);
    status |= ri_gpio_uninit();

    if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    return RD_SUCCESS;
}

rd_status_t ri_gpio_test_toggle (const ri_gpio_id_t
                                 input,
                                 const ri_gpio_id_t output)
{
    rd_status_t status = RD_SUCCESS;
    // * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
    ri_gpio_state_t state;
    status |= ri_gpio_init();
    status |= ri_gpio_configure (input, RI_GPIO_MODE_INPUT_NOPULL);
    status |= ri_gpio_configure (output,
                                 RI_GPIO_MODE_OUTPUT_STANDARD);
    status |= ri_gpio_write (output, RI_GPIO_LOW);
    status |= ri_gpio_read (input, &state);

    // Verify our start state
    if (RD_SUCCESS != status || RI_GPIO_LOW != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    // Verify low-to-high
    status |= ri_gpio_toggle (output);
    status |= ri_gpio_read (input, &state);

    if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    // Verify high-to-low
    status |= ri_gpio_toggle (output);
    status |= ri_gpio_read (input, &state);

    if (RD_SUCCESS != status || RI_GPIO_LOW != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    // Verify second low-to-high (after toggle, not static set)
    status |= ri_gpio_toggle (output);
    status |= ri_gpio_read (input, &state);
    status |= ri_gpio_uninit();

    if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        rd_test_register (false);
        return RD_ERROR_SELFTEST;
    }

    rd_test_register (true);
    return RD_SUCCESS;
}
#endif