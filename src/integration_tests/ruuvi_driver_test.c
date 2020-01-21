#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt_test.h"
#include "ruuvi_interface_gpio_test.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/** @brief counter of total tests run */
static size_t tests_total = 0;
/** @brief counter of total tests passed */
static size_t tests_passed = 0;


/** @brief Configuration structure of GPIO test */
static rd_test_gpio_cfg_t gpio_test_cfg = { .input = RUUVI_INTERFACE_GPIO_ID_UNUSED, .output = RUUVI_INTERFACE_GPIO_ID_UNUSED};

void rd_test_gpio_cfg (const rd_test_gpio_cfg_t cfg)
{
    gpio_test_cfg = cfg;
}

static bool rd_test_gpio_run (const rd_test_print_fp printfp)
{
    printfp ("GPIO tests ");

    if (gpio_test_cfg.input.pin != RUUVI_INTERFACE_GPIO_ID_UNUSED)
    {
        rd_status_t status = RD_SUCCESS;
        bool fail = false;
        status |= ruuvi_interface_gpio_test_init();

        if (RD_SUCCESS != status)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            fail = true;
        }

        status |= ruuvi_interface_gpio_test_configure (gpio_test_cfg.input, gpio_test_cfg.output);

        if (RD_SUCCESS != status)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            fail = true;
        }

        status |= ruuvi_interface_gpio_test_toggle (gpio_test_cfg.input, gpio_test_cfg.output);

        if (RD_SUCCESS != status)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            fail = true;
        }

        if (RD_SUCCESS == status) { printfp ("PASSED.\r\n"); }
        else { printfp ("FAILED.\r\n"); }
    }
    else { printfp ("SKIPPED.\r\n"); }
}

static bool rd_test_gpio_interrupt_run (const rd_test_print_fp printfp)
{
    printfp ("GPIO interrupt tests ");
    bool fail = false;

    if (gpio_test_cfg.input.pin != RUUVI_INTERFACE_GPIO_ID_UNUSED)
    {
        rd_status_t status = RD_SUCCESS;
        status |= ruuvi_interface_gpio_interrupt_test_init (gpio_test_cfg);

        if (RD_SUCCESS != status)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            fail = true;
        }

        status = ruuvi_interface_gpio_interrupt_test_enable (gpio_test_cfg);

        if (RD_SUCCESS != status)
        {
            RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
            fail = true;
        }

        if (RD_SUCCESS == status) { printfp ("PASSED.\r\n"); }
        else { printfp ("FAILED.\r\n"); }

        // todo: move to a separate teardown
        ruuvi_interface_gpio_interrupt_disable (gpio_test_cfg.input);
        ruuvi_interface_gpio_uninit();
        ruuvi_interface_gpio_interrupt_uninit();
    }
    else { printfp ("SKIPPED.\r\n"); }

    return !fail;
}

bool rd_test_all_run (const rd_test_print_fp printfp)
{
    tests_passed = 0;
    tests_total  = 0;
    printfp ("Running driver tests... \r\n");
    rd_test_gpio_run (printfp);
    rd_test_gpio_interrupt_run (printfp);
}

bool ruuvi_interface_expect_close (const float expect, const int8_t precision,
                                   const float check)
{
    if (!isfinite (expect) || !isfinite (check)) { return false; }

    const float max_delta = pow (10, precision);
    float delta = expect - check;

    if (delta < 0) { delta = 0 - delta; } // absolute value

    return max_delta > delta;
}

/**
 * @brief Register a test as being run.
 * Increments counter of total tests.
 * Read results with rd_test_status
 *
 * @param passed[in]: True if your test was successful.
 *
 * @return RD_SUCCESS
 */
rd_status_t rd_test_register (const bool passed)
{
    tests_total++;

    if (true == passed)
    {
        tests_passed++;
    }

    return RD_SUCCESS;
}

rd_status_t rd_test_status (size_t * const total, size_t * const passed)
{
    *total  = tests_total;
    *passed = tests_passed;
    return RD_SUCCESS;
}