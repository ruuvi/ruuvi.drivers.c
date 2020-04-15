/**
 * Ruuvi communication interface.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_communication.h"
#if RUUVI_NRF5_SDK15_COMMUNICATION_ENABLED

#include "ruuvi_driver_error.h"

#include <stdbool.h>
#include <stdint.h>
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"

rd_status_t ri_comm_id_get (uint64_t * const _id)
{
    uint64_t id = 0;
    id |= (uint64_t) ( (NRF_FICR->DEVICEID[0] >> 24) & 0xFF) << 56;
    id |= (uint64_t) ( (NRF_FICR->DEVICEID[0] >> 16) & 0xFF) << 48;
    id |= (uint64_t) ( (NRF_FICR->DEVICEID[0] >> 8) & 0xFF)  << 40;
    id |= (uint64_t) ( (NRF_FICR->DEVICEID[0] >> 0) & 0xFF)  << 32;
    id |= (uint64_t) ( (NRF_FICR->DEVICEID[1] >> 24) & 0xFF) << 24;
    id |= (uint64_t) ( (NRF_FICR->DEVICEID[1] >> 16) & 0xFF) << 16;
    id |= (uint64_t) ( (NRF_FICR->DEVICEID[1] >> 8) & 0xFF)  << 8;
    id |= (uint64_t) ( (NRF_FICR->DEVICEID[1] >> 0) & 0xFF)  << 0;
    *_id = id;
    return RD_SUCCESS;
}
#endif