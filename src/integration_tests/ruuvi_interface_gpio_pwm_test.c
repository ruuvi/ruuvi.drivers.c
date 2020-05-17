#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include "ruuvi_interface_gpio_interrupt_test.h"
#include "ruuvi_interface_gpio_pwm.h"
#include "ruuvi_interface_gpio_pwm_test.h"
#include "ruuvi_interface_gpio_test.h"
#include "ruuvi_interface_yield.h"
#include <stdlib.h>

static uint8_t num_int_trigs = 0;

static void on_interrupt (ri_gpio_evt_t evt)
{
    num_int_trigs ++;
}

static bool ri_gpio_pwm_test_init (/*TODO*/)
{
    rd_status_t status = RD_SUCCESS;
    bool failed = false;
    // - Initialization must return @c RD_SUCCESS on first call.
    status |= ri_gpio_pwm_init (/*TODO*/);

    if (RD_SUCCESS == status)
    {
        // - Initialization must return @c RD_ERROR_INVALID_STATE on second call.
        status = ri_gpio_pwm_init (/*TODO*/);

        if (RD_ERROR_INVALID_STATE != status)
        {
            failed = true;
        }

        // - Initialization must return @c RD_SUCCESS after uninitializtion.
        status = ri_gpio_pwm_uninit (/*TODO*/);
        status |= ri_gpio_pwm_init (/*TODO*/);

        if (RD_SUCCESS != status)
        {
            failed = true;
        }
    }
    else
    {
        failed = true;
    }

    // Cleanup
    ri_gpio_pwm_uninit (/*TODO*/);
    return failed;
}

static bool ri_gpio_pwm_test_start (const rd_test_gpio_cfg_t cfg)
{
    rd_status_t status = RD_SUCCESS;
    bool failed = false;
    const float orig_freq = RI_GPIO_PWM_TEST_FREQ_HZ;
    float configured_freq = orig_freq;
    const float orig_dc = RI_GPIO_PWM_TEST_DC;
    float configured_dc = orig_dc;
    status |= ri_gpio_pwm_init (/*TODO*/);
    status |= ri_gpio_pwm_start (cfg.output, RI_GPIO_MODE_OUTPUT_HIGHDRIVE,
                                 &configured_freq, &configured_dc);
    // Note: if timer-based low-power delay is enabled, this
    // is grossly inaccurate.
    ri_delay_ms (RI_GPIO_PWM_TEST_TIME_MS);
    status |= ri_gpio_pwm_stop (cfg.output /*, TODO */);

    // TODO: Allow +-1 error margin
    if (RI_GPIO_PWM_EXPECT_TRIGS != num_int_trigs)
    {
        failed = true;
    }

    // TODO: Test input validation.
    // TODO: Other tests as necessary.
    // Cleanup
    ri_gpio_pwm_uninit (/*TODO*/);
    return failed;
}

bool ri_gpio_pwm_run_integration_test (const rd_test_print_fp printfp,
                                       const ri_gpio_id_t input, const ri_gpio_id_t output)
{
    rd_status_t status;
    bool failed = false;
    rd_test_gpio_cfg_t cfg;
    const uint8_t interrupt_table_size = RI_GPIO_INTERRUPT_TEST_TABLE_SIZE;
    ri_gpio_interrupt_fp_t interrupt_table[RI_GPIO_INTERRUPT_TEST_TABLE_SIZE];
    cfg.input = input;
    cfg.output = output;
    printfp ("\"gpio_pwm\":{\r\n");
    printfp ("\"init\":");
    status = ri_gpio_init();
    status |= ri_gpio_interrupt_init (interrupt_table, interrupt_table_size);
    status |= ri_gpio_interrupt_enable (cfg.input,
                                        RI_GPIO_SLOPE_LOTOHI, RI_GPIO_MODE_INPUT_PULLDOWN, on_interrupt);
    failed |= ri_gpio_pwm_test_init (/*TODO*/);

    if (!failed)
    {
        printfp ("\"pass\",\r\n");
    }
    else
    {
        printfp ("\"fail\",\r\n");
    }

    printfp ("\"start\":");
    failed |= ri_gpio_pwm_test_start (cfg);

    if (!failed)
    {
        printfp ("\"pass\"\r\n");
    }
    else
    {
        printfp ("\"fail\"\r\n");
        failed = true;
    }

    // Cleanup.
    ri_gpio_pwm_uninit (/*TODO*/);
    ri_gpio_interrupt_disable (cfg.input);
    ri_gpio_interrupt_uninit();
    ri_gpio_uninit();
    printfp ("},\r\n");
    return failed;
}
#endif
