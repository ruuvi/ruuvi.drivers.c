#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_atomic.h"
#if RUUVI_NRF5_SDK15_ATOMIC_ENABLED
#include "nrf_atomic.h"

bool ri_atomic_flag (ri_atomic_t * const flag, const bool set)
{
    uint32_t expected = !set;
    return nrf_atomic_u32_cmp_exch (flag, &expected, set);
}

#endif