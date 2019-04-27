#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include <stdbool.h>

ruuvi_driver_status_t ruuvi_interface_gpio_test_init(void)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  // - Interface must return RUUVI_DRIVER_SUCCESS after first call.
  err_code = ruuvi_interface_gpio_init();

  if(RUUVI_DRIVER_SUCCESS != err_code) { return RUUVI_DRIVER_ERROR_SELFTEST; }

  // - Interface must return RUUVI_DRIVER_ERROR_INVALID_STATE when called while already initialized.
  err_code = ruuvi_interface_gpio_init();

  if(RUUVI_DRIVER_SUCCESS == err_code) { return RUUVI_DRIVER_ERROR_SELFTEST; }

  // - Interface must return RUUVI_DRIVER_SUCCESS when called after uninitialization.
  err_code = ruuvi_interface_gpio_uninit();

  if(RUUVI_DRIVER_SUCCESS != err_code) { return RUUVI_DRIVER_ERROR_SELFTEST; }

  err_code = ruuvi_interface_gpio_uninit();

  if(RUUVI_DRIVER_SUCCESS != err_code) { return RUUVI_DRIVER_ERROR_SELFTEST; }

  return RUUVI_DRIVER_SUCCESS;
}

bool ruuvi_interface_gpio_test_configure(const ruuvi_interface_gpio_id_t input,
    const ruuvi_interface_gpio_id_t output)
{
  ruuvi_driver_status_t status = RUUVI_DRIVER_SUCCESS;
  // * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
  ruuvi_interface_gpio_state_t state;
  status |= ruuvi_interface_gpio_init();
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL);
  status |= ruuvi_interface_gpio_configure(output, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != state || RUUVI_INTERFACE_GPIO_HIGH != state) { return false; }

  // - When Input is in High-Z mode, and output mode is INPUT_PULLDOWN, input must read as LOW
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_NOPULL);
  status |= ruuvi_interface_gpio_configure(output,
            RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != state || RUUVI_INTERFACE_GPIO_LOW != state) { return false; }

  // - When Input is in INPUT_PULLUP mode, and output is in OUTPUT_LOW mode, input must read as LOW
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP);
  status |= ruuvi_interface_gpio_configure(output,
            RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD);
  status |= ruuvi_interface_gpio_write(output, RUUVI_INTERFACE_GPIO_LOW);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != state || RUUVI_INTERFACE_GPIO_LOW != state) { return false; }

  // - When Input is in INPUT_PULLDOWN mode, and output is in OUTPUT_HIGH mode, input must read as HIGH
  status |= ruuvi_interface_gpio_configure(input, RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLDOWN);
  status |= ruuvi_interface_gpio_configure(output,
            RUUVI_INTERFACE_GPIO_MODE_OUTPUT_STANDARD);
  status |= ruuvi_interface_gpio_write(output, RUUVI_INTERFACE_GPIO_HIGH);
  status |= ruuvi_interface_gpio_read(input, &state);
  status |= ruuvi_interface_gpio_uninit();

  if(RUUVI_DRIVER_SUCCESS != state || RUUVI_INTERFACE_GPIO_HIGH != state) { return false; }

  return true;
}

/**
 * @brief Toggle the state of a pin of a port.
 *
 * Input is in High-Z mode. Value read by it must toggle after output pin is toggled.
 *
 * @param input[in]  Pin used to check the state of output pin.
 * @param output[in] Pin being toggled.
 *
 * @return @c true if test passes, @c false on error.
 */
bool ruuvi_interface_gpio_test_toggle(const ruuvi_interface_gpio_id_t input,
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
  if(RUUVI_DRIVER_SUCCESS != state || RUUVI_INTERFACE_GPIO_LOW != state) { return false; }

  // Verify low-to-high
  status |= ruuvi_interface_gpio_toggle(output);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != state || RUUVI_INTERFACE_GPIO_HIGH != state) { return false; }

  // Verify high-to-low
  status |= ruuvi_interface_gpio_toggle(output);
  status |= ruuvi_interface_gpio_read(input, &state);

  if(RUUVI_DRIVER_SUCCESS != state || RUUVI_INTERFACE_GPIO_LOW != state) { return false; }

  // Verify second low-to-high (after toggle, not static set)
  status |= ruuvi_interface_gpio_toggle(output);
  status |= ruuvi_interface_gpio_read(input, &state);
  status |= ruuvi_interface_gpio_uninit();

  if(RUUVI_DRIVER_SUCCESS != state || RUUVI_INTERFACE_GPIO_HIGH != state) { return false; }

  return true;
}
