#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include "ruuvi_interface_gpio_interrupt_test.h"
#include "ruuvi_interface_gpio_test.h"
#include <stdlib.h>

static uint8_t num_int_trigs = 0;
static ri_gpio_evt_t last_evt;

void on_interrupt (ri_gpio_evt_t evt)
{
    num_int_trigs ++;
    last_evt = evt;
}


/**
 * @brief Test GPIO interrupt initialization.
 *
 *
 * @param[in] cfg configuration of GPIO pins to test. Required to determine interrupt table size.
 *
 * @return @ref RD_SUCCESS on success, error code on failure.
 */
rd_status_t ri_gpio_interrupt_test_init (
    const rd_test_gpio_cfg_t cfg)
{
    rd_status_t status = RD_SUCCESS;
    const uint8_t interrupt_table_size = RI_GPIO_INTERRUPT_TEST_TABLE_SIZE;
    ri_gpio_interrupt_fp_t interrupt_table[RI_GPIO_INTERRUPT_TEST_TABLE_SIZE];

    if ( ( (cfg.input % 32) + ( (cfg.input >> 8) * 32)) > interrupt_table_size
            || ( (cfg.output % 32) + ( (cfg.input >> 8) * 32)) > interrupt_table_size)
    {
        status |= RD_ERROR_NO_MEM;
    }

    if (RD_SUCCESS == status)
    {
        // - Initialization must return @c RD_ERROR_INVALID_STATE if GPIO is uninitialized
        status |= ri_gpio_uninit();
        status |= ri_gpio_interrupt_init (interrupt_table, interrupt_table_size);

        if (RD_ERROR_INVALID_STATE != status)
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
        }
        else
        {
            status = RD_SUCCESS;
        }
    }

    if (RD_SUCCESS == status)
    {
        // - Initialization must return @c RD_SUCCESS on first call.
        status = ri_gpio_init();
        status |= ri_gpio_interrupt_init (interrupt_table, interrupt_table_size);

        if (RD_SUCCESS != status)
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
        }
        else
        {
            // - Initialization must return @c RD_ERROR_INVALID_STATE on second call.
            status = ri_gpio_interrupt_init (interrupt_table, interrupt_table_size);

            if (RD_ERROR_INVALID_STATE != status)
            {
                RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
            }

            // - Initialization must return @c RD_SUCCESS after uninitializtion.
            status = ri_gpio_interrupt_uninit();
            status |= ri_gpio_interrupt_init (interrupt_table, interrupt_table_size);

            if (RD_SUCCESS != status)
            {
                RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
                return RD_ERROR_SELFTEST;
            }
        }
    }

    if (RD_SUCCESS == status)
    {
        // - Initialization must return @c RD_ERROR_NULL if interrupt handler table is @c NULL.
        status = ri_gpio_interrupt_init (NULL, interrupt_table_size);

        if (RD_ERROR_NULL != status)
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
        }
        else
        {
            status = RD_SUCCESS;
        }
    }

    status |= ri_gpio_interrupt_uninit();
    status |= ri_gpio_uninit();
    return status;
}

rd_status_t ri_gpio_interrupt_test_enable (
    const rd_test_gpio_cfg_t cfg)
{
    rd_status_t status = RD_SUCCESS;
    const uint8_t interrupt_table_size = RI_GPIO_INTERRUPT_TEST_TABLE_SIZE;
    ri_gpio_interrupt_fp_t
    interrupt_table[RI_GPIO_INTERRUPT_TEST_TABLE_SIZE] = {0};

    if ( ( (cfg.input % 32) + ( (cfg.input >> 8) * 32)) > interrupt_table_size
            || ( (cfg.output % 32) + ( (cfg.input >> 8) * 32)) > interrupt_table_size)
    {
        return RD_ERROR_NO_MEM;
    }

    // - Return RD_ERROR_INVALID_STATE if GPIO or GPIO_INTERRUPT are not initialized
    ri_gpio_interrupt_uninit();
    ri_gpio_uninit();
    status |= ri_gpio_interrupt_enable (cfg.input,
                                        RI_GPIO_SLOPE_LOTOHI, RI_GPIO_MODE_INPUT_NOPULL, on_interrupt);

    if (RD_ERROR_INVALID_STATE != status)
    {
        RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
    }
    else
    {
        status = RD_SUCCESS;
    }

    if (RD_SUCCESS == status)
    {
        // - Interrupt function shall be called exactly once when input is configured as low-to-high while input is low and
        //   input goes low-to-high, high-to-low.
        num_int_trigs = 0;
        status = ri_gpio_init();
        status |= ri_gpio_interrupt_init (interrupt_table, interrupt_table_size);
        status |= ri_gpio_configure (cfg.output,
                                     RI_GPIO_MODE_OUTPUT_STANDARD);
        status |= ri_gpio_write (cfg.output, RI_GPIO_LOW);
        status |= ri_gpio_interrupt_enable (cfg.input,
                                            RI_GPIO_SLOPE_LOTOHI, RI_GPIO_MODE_INPUT_NOPULL, on_interrupt);
        status |= ri_gpio_write (cfg.output, RI_GPIO_HIGH);
        status |= ri_gpio_write (cfg.output, RI_GPIO_LOW);

        if (RD_SUCCESS != status || 1 != num_int_trigs)
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
            status |= RD_ERROR_SELFTEST;
        }
    }

    if (RD_SUCCESS == status)
    {
        // - Interrupt function shall not be called after interrupt has been disabled
        status |= ri_gpio_interrupt_disable (cfg.input);
        status |= ri_gpio_write (cfg.output, RI_GPIO_HIGH);
        status |= ri_gpio_write (cfg.output, RI_GPIO_LOW);

        if (RD_SUCCESS != status || 1 != num_int_trigs)
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
            status |= RD_ERROR_SELFTEST;
        }

        // - Interrupt function maybe be called once or twice when input is configured as high-to-low while input is low and
        //   input goes low-to-high, high-to-low. i.e. Triggering during activation of test.
        status |= ri_gpio_write (cfg.output, RI_GPIO_LOW);
        num_int_trigs = 0;
        status |= ri_gpio_interrupt_enable (cfg.input,
                                            RI_GPIO_SLOPE_HITOLO, RI_GPIO_MODE_INPUT_NOPULL, on_interrupt);
        status |= ri_gpio_write (cfg.output, RI_GPIO_HIGH);
        status |= ri_gpio_write (cfg.output, RI_GPIO_LOW);

        if (RD_SUCCESS != status
                || ( (1 != num_int_trigs) && (2 != num_int_trigs)))
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
            status |= RD_ERROR_SELFTEST;
        }

        // - Interrupt function shall be called exactly twice when input is configured as toggle while input is low and
        //  input goes low-to-high, high-to-low.
        status = ri_gpio_write (cfg.output, RI_GPIO_LOW);
        status |= ri_gpio_interrupt_disable (cfg.input);
        num_int_trigs = 0;
        status |= ri_gpio_interrupt_enable (cfg.input,
                                            RI_GPIO_SLOPE_TOGGLE, RI_GPIO_MODE_INPUT_NOPULL, on_interrupt);
        status |= ri_gpio_write (cfg.output, RI_GPIO_HIGH);
        status |= ri_gpio_write (cfg.output, RI_GPIO_LOW);

        if (RD_SUCCESS != status || 2 != num_int_trigs)
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
            status |= RD_ERROR_SELFTEST;
        }

        // - Interrupt pin shall be at logic HIGH when interrupt is enabled with a pull-up and the pin is not loaded externally
        status = ri_gpio_configure (cfg.output,
                                    RI_GPIO_MODE_INPUT_NOPULL);
        status |= ri_gpio_interrupt_disable (cfg.input);
        num_int_trigs = 0;
        status |= ri_gpio_interrupt_enable (cfg.input,
                                            RI_GPIO_SLOPE_TOGGLE, RI_GPIO_MODE_INPUT_PULLUP, on_interrupt);
        ri_gpio_state_t state;
        status |= ri_gpio_read (cfg.output, &state);

        if (RD_SUCCESS != status || RI_GPIO_HIGH != state)
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
            status |= RD_ERROR_SELFTEST;
        }

        // - Interrupt pin shall be at logic LOW when interrupt is enabled with a pull-down and the pin is not loaded externally
        status |= ri_gpio_configure (cfg.output, RI_GPIO_MODE_INPUT_NOPULL);
        status |= ri_gpio_interrupt_disable (cfg.input);
        num_int_trigs = 0;
        status |= ri_gpio_interrupt_enable (cfg.input,
                                            RI_GPIO_SLOPE_TOGGLE, RI_GPIO_MODE_INPUT_PULLDOWN,
                                            on_interrupt);
        status |= ri_gpio_read (cfg.output, &state);

        if (RD_SUCCESS != status || RI_GPIO_LOW != state)
        {
            RD_ERROR_CHECK (status, ~RD_ERROR_FATAL);
            status |= RD_ERROR_SELFTEST;
        }
    }

    status |= ri_gpio_interrupt_disable (cfg.input);
    status |= ri_gpio_interrupt_uninit();
    status |= ri_gpio_uninit();
    return status;
}
bool ri_gpio_interrupt_run_integration_test (const rd_test_print_fp printfp,
        const ri_gpio_id_t input, const ri_gpio_id_t output)
{
    rd_status_t status;
    rd_test_gpio_cfg_t cfg;
    cfg.input = input;
    cfg.output = output;
    printfp ("\"gpio_interrupt\":{\r\n");
    printfp ("\"init\":");
    status = ri_gpio_interrupt_test_init (cfg);

    if (RD_SUCCESS == status)
    {
        printfp ("\"pass\",\r\n");
    }
    else
    {
        printfp ("\"fail\",\r\n");
    }

    printfp ("\"enable\":");
    status = ri_gpio_interrupt_test_enable (cfg);

    if (RD_SUCCESS == status)
    {
        printfp ("\"pass\"\r\n");
    }
    else
    {
        printfp ("\"fail\"\r\n");
    }

    printfp ("},\r\n");
    return (RD_SUCCESS != status);
}
#endif