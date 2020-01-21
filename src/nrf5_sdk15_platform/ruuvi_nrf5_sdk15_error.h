#ifndef RUUVI_NRF5_SDK15_ERROR_H
#define RUUVI_NRF5_SDK15_ERROR_H
#include "nrf_error.h"
#include "sdk_errors.h"
#include "ruuvi_driver_error.h"
/**
 * @addtogroup Error
 * @{
 */
/**
* @file ruuvi_nrf5_sdk15_error.h
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2019-01-31
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
*
* Converts nRF5 SDK15 error codes to Ruuvi error codes
*
*/

/**
 * @brief convert nrf5 sdk15 error code into Ruuvi error code.
 *
 * @param[in] error error to convert
 * @return Ruuvi error corresponding to given error.
 */
rd_status_t ruuvi_nrf5_sdk15_to_ruuvi_error (const ret_code_t error);

/** @} */
#endif