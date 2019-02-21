#include "ruuvi_driver_test.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/*

bool ruuvi_driver_test_all_run(const ruuvi_driver_test_print_fp printfp)
{
  uint32_t total_tests = 0;
  uint32_t passed = 0;

  total_tests++;
  passed += ruuvi_interface_test_gpio_pullup();

  total_tests++;
  passed += ruuvi_interface_test_gpio_pulldown();

  char msg[128] = {0};
  snprintf(msg, sizeof(msg), "Driver tests ran: %d, passed: %d.\r\n", total_tests, passed);
  printfp(msg);

  return (total_tests == passed);
}

bool ruuvi_interface_expect_close(const float expect, const int8_t precision, const float check)
{
  if(!isfinite(expect) || !isfinite(check)) { return false; }
  const float max_delta = pow(10, precision);
  float delta = expect - check;
  if(delta < 0) { delta = 0 - delta; } // absolute value
  return max_delta > delta;
}*/