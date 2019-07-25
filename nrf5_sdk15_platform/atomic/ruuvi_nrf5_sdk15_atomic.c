#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_ATOMIC_ENABLED
#include "ruuvi_interface_atomic.h"
#include "nrf_atomic.h"

bool ruuvi_interface_atomic_flag(ruuvi_interface_atomic_ptr flag, const bool set)
{
  uint32_t expected = !set;
  return nrf_atomic_u32_cmp_exch((ruuvi_interface_atomic_t*) flag, &expected, set);
}

#endif