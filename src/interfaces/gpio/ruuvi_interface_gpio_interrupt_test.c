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
static ruuvi_interface_gpio_evt_t last_evt;

void on_interrupt (ruuvi_interface_gpio_evt_t evt)
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
 * @return @ref RUUVI_DRIVER_SUCCESS on success, error code on failure.
 */
ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_test_init (
    const ruuvi_driver_test_gpio_cfg_t cfg)
{
    ruuvi_driver_status_t status = RUUVI_DRIVER_SUCCESS;
    const uint8_t interrupt_table_size = RUUVI_INTERFACE_GPIO_INTERRUPT_TEST_TABLE_SIZE;
    ruuvi_interface_gpio_interrupt_fp_t
    interrupt_table[RUUVI_INTERFACE_GPIO_INTERRUPT_TEST_TABLE_SIZE];

    if ( (cfg.input.port_pin.pin + cfg.input.port_pin.port * 32) > interrupt_table_size ||
            (cfg.output.port_pin.pin + cfg.output.port_pin.port * 32) > interrupt_table_size)
    {
        return RUUVI_DRIVER_ERROR_NO_MEM;
    }

    // - Initialization must return @c RUUVI_DRIVER_ERROR_INVALID_STATE if GPIO is uninitialized
    status |= ruuvi_interface_gpio_uninit();
    status |= ruuvi_interface_gpio_interrupt_init (interrupt_table, interrupt_table_size);

    if (RUUVI_DRIVER_ERROR_INVALID_STATE != status)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Initialization must return @c RUUVI_DRIVER_SUCCESS on first call.
    status = ruuvi_interface_gpio_init();
    status |= ruuvi_interface_gpio_interrupt_init (interrupt_table, interrupt_table_size);

    if (RUUVI_DRIVER_SUCCESS != status)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Initialization must return @c RUUVI_DRIVER_ERROR_INVALID_STATE on second call.
    status = ruuvi_interface_gpio_interrupt_init (interrupt_table, interrupt_table_size);

    if (RUUVI_DRIVER_ERROR_INVALID_STATE != status)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Initialization must return @c RUUVI_DRIVER_SUCCESS after uninitializtion.
    status = ruuvi_interface_gpio_interrupt_uninit();
    status |= ruuvi_interface_gpio_interrupt_init (interrupt_table, interrupt_table_size);

    if (RUUVI_DRIVER_SUCCESS != status)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Initialization must return @c RUUVI_DRIVER_ERROR_NULL if interrupt handler table is @c NULL.
    status = ruuvi_interface_gpio_interrupt_init (NULL, interrupt_table_size);

    if (RUUVI_DRIVER_ERROR_NULL != status)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    status = ruuvi_interface_gpio_interrupt_uninit();
    return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_gpio_interrupt_test_enable (
    const ruuvi_driver_test_gpio_cfg_t cfg)
{
    ruuvi_driver_status_t status = RUUVI_DRIVER_SUCCESS;
    const uint8_t interrupt_table_size = RUUVI_INTERFACE_GPIO_INTERRUPT_TEST_TABLE_SIZE;
    ruuvi_interface_gpio_interrupt_fp_t
    interrupt_table[RUUVI_INTERFACE_GPIO_INTERRUPT_TEST_TABLE_SIZE] = {0};

    if ( (cfg.input.port_pin.pin + cfg.input.port_pin.port * 32) > interrupt_table_size ||
            (cfg.output.port_pin.pin + cfg.output.port_pin.port * 32) > interrupt_table_size)
    {
        return RUUVI_DRIVER_ERROR_NO_MEM;
    }

    // - Return RUUVI_DRIVER_ERROR_INVALID_STATE if GPIO or GPIO_INTERRUPT are not initialized
    ruuvi_interface_gpio_interrupt_uninit();
    ruuvi_interface_gpio_uninit();
    status |= ruuvi_interface_gpio_interrupt_enable (cfg.input,
              RUUVI_INTERFACE_GPIO_SLOPE_LOTOHI, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL, on_interrupt);

    if (RUUVI_DRIVER_ERROR_INVALID_STATE != status)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Interrupt function shall be called exactly once when input is configured as low-to-high while input is low and
    //   input goes low-to-high, high-to-low.
    num_int_trigs = 0;
    status = ruuvi_interface_gpio_init();
    status |= ruuvi_interface_gpio_interrupt_init (interrupt_table, interrupt_table_size);
    status |= ruuvi_interface_gpio_configure (cfg.output,
              RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_LOW);
    status |= ruuvi_interface_gpio_interrupt_enable (cfg.input,
              RUUVI_INTERFACE_GPIO_SLOPE_LOTOHI, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL, on_interrupt);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_HIGH);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_LOW);

    if (RUUVI_DRIVER_SUCCESS != status || 1 != num_int_trigs)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Interrupt function shall not be called after interrupt has been disabled
    status |= ruuvi_interface_gpio_interrupt_disable (cfg.input);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_HIGH);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_LOW);

    if (RUUVI_DRIVER_SUCCESS != status || 1 != num_int_trigs)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Interrupt function maybe be called once or twice when input is configured as high-to-low while input is low and
    //   input goes low-to-high, high-to-low. i.e. Triggering during activation of
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_LOW);
    num_int_trigs = 0;
    status |= ruuvi_interface_gpio_interrupt_enable (cfg.input,
              RUUVI_INTERFACE_GPIO_SLOPE_HITOLO, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL, on_interrupt);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_HIGH);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_LOW);

    if (RUUVI_DRIVER_SUCCESS != status || (1 != num_int_trigs && 2 != num_int_trigs))
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Interrupt function shall be called exactly twice when input is configured as toggle while input is low and
    //  input goes low-to-high, high-to-low.
    status = ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_LOW);
    status |= ruuvi_interface_gpio_interrupt_disable (cfg.input);
    num_int_trigs = 0;
    status |= ruuvi_interface_gpio_interrupt_enable (cfg.input,
              RUUVI_INTERFACE_GPIO_SLOPE_TOGGLE, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL, on_interrupt);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_HIGH);
    status |= ruuvi_interface_gpio_write (cfg.output, RUUVI_INTERFACE_GPIO_LOW);

    if (RUUVI_DRIVER_SUCCESS != status || 2 != num_int_trigs)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Interrupt pin shall be at logic HIGH when interrupt is enabled with a pull-up and the pin is not loaded externally
    status = ruuvi_interface_gpio_configure (cfg.output,
             RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL);
    status |= ruuvi_interface_gpio_interrupt_disable (cfg.input);
    num_int_trigs = 0;
    status |= ruuvi_interface_gpio_interrupt_enable (cfg.input,
              RUUVI_INTERFACE_GPIO_SLOPE_TOGGLE, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP, on_interrupt);
    ruuvi_interface_gpio_state_t state;
    status |= ruuvi_interface_gpio_read (cfg.output, &state);

    if (RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_HIGH != state)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    // - Interrupt pin shall be at logic LOW when interrupt is enabled with a pull-down and the pin is not loaded externally
    status |= ruuvi_interface_gpio_configure (cfg.output,
              RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL);
    status |= ruuvi_interface_gpio_interrupt_disable (cfg.input);
    num_int_trigs = 0;
    status |= ruuvi_interface_gpio_interrupt_enable (cfg.input,
              RUUVI_INTERFACE_GPIO_SLOPE_TOGGLE, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN,
              on_interrupt);
    status |= ruuvi_interface_gpio_read (cfg.output, &state);

    if (RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_LOW != state)
    {
        RUUVI_DRIVER_ERROR_CHECK (status, ~RUUVI_DRIVER_ERROR_FATAL);
        ruuvi_driver_test_register (false);
        return RUUVI_DRIVER_ERROR_SELFTEST;
    }

    ruuvi_driver_test_register (true);
    status |= ruuvi_interface_gpio_interrupt_disable (cfg.input);
    status = ruuvi_interface_gpio_uninit();
    status = ruuvi_interface_gpio_interrupt_uninit();
    return RUUVI_DRIVER_SUCCESS;
}

#endif