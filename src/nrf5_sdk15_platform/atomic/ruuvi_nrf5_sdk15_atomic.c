#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_ATOMIC_ENABLED
#include "ruuvi_interface_atomic.h"
#include "nrf_atomic.h"

bool ri_atomic_flag(ri_atomic_t* const flag, const bool set)
{
  uint32_t expected = !set;
  return nrf_atomic_u32_cmp_exch(flag, &expected, set);
}

#endif