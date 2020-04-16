#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt_test.h"
#include "ruuvi_interface_gpio_test.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool ri_expect_close (const float expect, const int8_t precision,
                      const float check)
{
    if (!isfinite (expect) || !isfinite (check)) { return false; }

    const float max_delta = pow (10, precision);
    float delta = expect - check;

    if (delta < 0) { delta = 0 - delta; } // absolute value

    return max_delta > delta;
}

#endif
