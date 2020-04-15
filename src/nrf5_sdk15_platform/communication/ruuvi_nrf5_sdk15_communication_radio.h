#ifndef RUUVI_NRF5_SDK15_COMMUNICATION_RADIO_H
#define RUUVI_NRF5_SDK15_COMMUNICATION_RADIO_H

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication_radio.h"
#include <stdbool.h>
#include <stdint.h>
#if RUUVI_NRF5_SDK15_RADIO_ENABLED

/**
 * @addtogroup Radio Radio operations.
 */

/** @{ */

/**
 * @file ruuvi_nrf5_sdk15_communication_radio.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-04-15
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * nRF5 SDK15 -specific helpers.
 *
 */


/**
 * @brief Convert radio modulation to nRF5 SDK15 constant.
 *
 * @return Nordic SDK constant for modulation.
 * @retval BLE_GAP_PHY_NOT_SET if radio is not initialized or configured.
 */
uint8_t ruuvi_nrf5_sdk15_radio_phy_get (void);

/**
 * @brief Configure nRF5 SDK15 channel bit mask to match enabled channels.
 *
 * @param[out] nrf_channels Pointer to nRF Channel mask to configure
 * @param[in]  channels Enabled channels
 */
void ruuvi_nrf5_sdk15_radio_channels_set (uint8_t * const nrf_channels,
        const ri_radio_channels_t channels);

/** @} */
#endif
#endif