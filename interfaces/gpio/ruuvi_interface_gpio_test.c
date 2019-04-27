#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include <stdbool.h>

ruuvi_driver_status_t ruuvi_interface_gpio_test_init(void)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  // - Interface must return RUUVI_DRIVER_SUCCESS after first call.
  err_code = ruuvi_interface_gpio_init();

  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  // - Interface must return RUUVI_DRIVER_ERROR_INVALID_STATE when called while already initialized.
  err_code = ruuvi_interface_gpio_init();

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  // - Interface must return RUUVI_DRIVER_SUCCESS when called after uninitialization.
  err_code = ruuvi_interface_gpio_uninit();

  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  err_code = ruuvi_interface_gpio_uninit();

  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_gpio_test_configure(const ruuvi_interface_gpio_id_t
    input,
    const ruuvi_interface_gpio_id_t output)
{
  ruuvi_driver_status_t status = RUUVI_DRIVER_SUCCESS;
  // * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
  ruuvi_interface_gpio_state_t state;
  status |= ruuvi_interface_gpio_init();
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL);
  status |= ruuvi_interface_gpio_configure(output, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_HIGH != state)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  // - When Input is in High-Z mode, and output mode is INPUT_PULLDOWN, input must read as LOW
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL);
  status |= ruuvi_interface_gpio_configure(output,
            RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_LOW != state)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  // - When Input is in INPUT_PULLUP mode, and output is in OUTPUT_LOW mode, input must read as LOW
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP);
  status |= ruuvi_interface_gpio_configure(output,
            RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD);
  status |= ruuvi_interface_gpio_write(output, RUUVI_INTERFACE_GPIO_LOW);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_LOW != state)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  // - When Input is in INPUT_PULLDOWN mode, and output is in OUTPUT_HIGH mode, input must read as HIGH
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN);
  status |= ruuvi_interface_gpio_configure(output,
            RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD);
  status |= ruuvi_interface_gpio_write(output, RUUVI_INTERFACE_GPIO_HIGH);
  status |= ruuvi_interface_gpio_read(input, &state);
  status |= ruuvi_interface_gpio_uninit();

  if(RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_HIGH != state)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_gpio_test_toggle(const ruuvi_interface_gpio_id_t
    input,
    const ruuvi_interface_gpio_id_t output)
{
  ruuvi_driver_status_t status = RUUVI_DRIVER_SUCCESS;
  // * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
  ruuvi_interface_gpio_state_t state;
  status |= ruuvi_interface_gpio_init();
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL);
  status |= ruuvi_interface_gpio_configure(output,
            RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD);
  status |= ruuvi_interface_gpio_write(output, RUUVI_INTERFACE_GPIO_LOW);
  status |= ruuvi_interface_gpio_read(input, &state);

  // Verify our start state
  if(RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_LOW != state)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  // Verify low-to-high
  status |= ruuvi_interface_gpio_toggle(output);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_HIGH != state)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  // Verify high-to-low
  status |= ruuvi_interface_gpio_toggle(output);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_LOW != state)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  // Verify second low-to-high (after toggle, not static set)
  status |= ruuvi_interface_gpio_toggle(output);
  status |= ruuvi_interface_gpio_read(input, &state);
  status |= ruuvi_interface_gpio_uninit();

  if(RUUVI_DRIVER_SUCCESS != status || RUUVI_INTERFACE_GPIO_HIGH != state)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    ruuvi_driver_test_register(false);
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  ruuvi_driver_test_register(true);
  return RUUVI_DRIVER_SUCCESS;
}
#endif