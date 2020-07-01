#ifndef NRF5_SDK5_CONFIG_H
#define NRF5_SDK5_CONFIG_H

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_log.h"               //!< Check if NRF_LOG is required

#if (!FRUITY_CONFIGURED)
#        warning "FRUITYMESH platform is not configured, using defaults."
#endif

#endif