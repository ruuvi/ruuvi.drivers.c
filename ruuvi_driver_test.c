#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_test.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/** @brief Configuration structure of GPIO test */
static ruuvi_driver_test_gpio_cfg_t gpio_test_cfg = { .input = RUUVI_INTERFACE_GPIO_ID_UNUSED, .output = RUUVI_INTERFACE_GPIO_ID_UNUSED};

void ruuvi_driver_test_gpio_cfg(const ruuvi_driver_test_gpio_cfg_t cfg)
{
  gpio_test_cfg = cfg;
}

static bool ruuvi_driver_test_gpio_run(const ruuvi_driver_test_print_fp printfp)
{
  printfp("GPIO tests ");

  if(gpio_test_cfg.input.pin != RUUVI_INTERFACE_GPIO_ID_UNUSED)
  {
    ruuvi_driver_status_t status = RUUVI_DRIVER_SUCCESS;
    bool fail = false;
    status |= ruuvi_interface_gpio_test_init();

    if(RUUVI_DRIVER_SUCCESS != status)
    {
      RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
      fail = true;
    }

    status |= ruuvi_interface_gpio_test_configure(gpio_test_cfg.input, gpio_test_cfg.output);

    if(RUUVI_DRIVER_SUCCESS != status)
    {
      RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
      fail = true;
    }

    status |= ruuvi_interface_gpio_test_toggle(gpio_test_cfg.input, gpio_test_cfg.output);

    if(RUUVI_DRIVER_SUCCESS != status)
    {
      RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
      fail = true;
    }

    if(RUUVI_DRIVER_SUCCESS == status) { printfp("PASSED.\r\n"); }
    else { printfp("FAILED.\r\n"); }
  }
  else { printfp("SKIPPED.\r\n"); }
}

bool ruuvi_driver_test_all_run(const ruuvi_driver_test_print_fp printfp)
{
  printfp("Running driver tests... \r\n");
  ruuvi_driver_test_gpio_run(printfp);
}

bool ruuvi_interface_expect_close(const float expect, const int8_t precision,
                                  const float check)
{
  if(!isfinite(expect) || !isfinite(check)) { return false; }

  const float max_delta = pow(10, precision);
  float delta = expect - check;

  if(delta < 0) { delta = 0 - delta; } // absolute value

  return max_delta > delta;
}