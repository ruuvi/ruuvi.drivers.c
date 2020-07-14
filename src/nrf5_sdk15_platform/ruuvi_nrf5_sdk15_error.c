#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_ENABLED
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
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "nrf_drv_twi.h"

rd_status_t ruuvi_nrf5_sdk15_to_ruuvi_error (const ret_code_t err_code)
{
    if (NRF_SUCCESS == err_code)              { return RD_SUCCESS; }

    if (NRF_ERROR_INTERNAL == err_code)       { return RD_ERROR_INTERNAL; }

    if (NRF_ERROR_NO_MEM == err_code)         { return RD_ERROR_NO_MEM; }

    if (NRF_ERROR_NOT_FOUND == err_code)      { return RD_ERROR_NOT_FOUND; }

    if (NRF_ERROR_NOT_SUPPORTED == err_code)  { return RD_ERROR_NOT_SUPPORTED; }

    if (NRF_ERROR_INVALID_PARAM == err_code)  { return RD_ERROR_INVALID_PARAM; }

    if (NRF_ERROR_INVALID_STATE == err_code)  { return RD_ERROR_INVALID_STATE; }

    if (NRF_ERROR_MODULE_ALREADY_INITIALIZED == err_code)
    {
        return RD_ERROR_INVALID_STATE;
    }

    if (NRF_ERROR_INVALID_LENGTH == err_code) { return RD_ERROR_INVALID_LENGTH; }

    if (NRF_ERROR_INVALID_FLAGS == err_code)  { return RD_ERROR_INVALID_FLAGS; }

    if (NRF_ERROR_DATA_SIZE == err_code)      { return RD_ERROR_DATA_SIZE; }

    if (NRF_ERROR_TIMEOUT == err_code)        { return RD_ERROR_TIMEOUT; }

    if (NRF_ERROR_NULL == err_code)           { return RD_ERROR_NULL; }

    if (NRF_ERROR_FORBIDDEN == err_code)      { return RD_ERROR_FORBIDDEN; }

    if (NRF_ERROR_INVALID_ADDR == err_code)   { return RD_ERROR_INVALID_ADDR; }

    if (NRF_ERROR_BUSY == err_code)           { return RD_ERROR_BUSY; }

    if (NRF_ERROR_RESOURCES == err_code)      { return RD_ERROR_RESOURCES; }

    if (NRF_ERROR_DRV_TWI_ERR_DNACK == err_code ||
            NRF_ERROR_DRV_TWI_ERR_ANACK == err_code)
    {
        return RD_ERROR_NOT_ACKNOWLEDGED;
    }

    return RD_ERROR_INTERNAL;
}
/** @} */
#endif