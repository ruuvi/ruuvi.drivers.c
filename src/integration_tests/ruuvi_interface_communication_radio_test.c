#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_communication_radio_test.h"
#include "ruuvi_interface_communication_radio.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_yield.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/**
 * @addtogroup NFC
 * @{
 */
/**
 * @file ruuvi_interface_communication_radio_test.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-03-04
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Integration test Radio implementation.
 */

static bool ri_radio_init_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    ri_radio_modulation_t modulation = 0;
    printfp ("\"init\":");
    err_code |= ri_radio_init (RI_RADIO_BLE_1MBPS);

    if (RD_SUCCESS != err_code)
    {
        status = true;
    }
    else
    {
        err_code |= ri_radio_init (RI_RADIO_BLE_1MBPS);

        if (RD_ERROR_INVALID_STATE != err_code)
        {
            status = true;
        }
        else
        {
            err_code = ri_radio_uninit();
            err_code |= ri_radio_init (RI_RADIO_BLE_1MBPS);
            err_code |= ri_radio_get_modulation (&modulation);

            if ( (RD_SUCCESS != err_code)
                    || !ri_radio_is_init()
                    || RI_RADIO_BLE_1MBPS != modulation)
            {
                status = true;
            }
        }
    }

    if (status)
    {
        printfp ("\"fail\",\r\n");
    }
    else
    {
        printfp ("\"pass\",\r\n");
    }

    (void) ri_radio_uninit();
    return status;
}

/**
 * Writes maximum 64-bit unique address of the device to the pointer. This address
 * may be changed during runtime. The address is identifier of the device on radio network,
 * such as BLE MAC address.
 *
 * @param[out] address Value of radio address, i.e. MAC address.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_NOT_SUPPORTED if address cannot be returned on given platform
 */
static bool ri_radio_address_test (const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    bool status = false;
    const uint64_t addr_set = 0xCFEDAABBCCDDU;
    uint64_t addr_old = 0;
    uint64_t addr_new = 0;
    printfp ("\"address_set_get\":");
    err_code |= ri_radio_init (RI_RADIO_BLE_1MBPS);

    if (RD_SUCCESS != err_code)
    {
        status = true;
    }
    else
    {
        err_code |= ri_radio_address_get (&addr_old);
        err_code |= ri_radio_address_set (addr_set);
        err_code |= ri_radio_address_get (&addr_new);

        if ( (RD_SUCCESS != err_code)
                || (addr_set != addr_new)
                || (addr_old == addr_new))
        {
            status = true;
        }
    }

    if (status)
    {
        printfp ("\"fail\"\r\n");
    }
    else
    {
        printfp ("\"pass\"\r\n");
    }

    (void) ri_radio_uninit();
    return status;
}

void print_modulation (const rd_test_print_fp printfp,
                       const ri_radio_modulation_t modulation)
{
    switch (modulation)
    {
        case RI_RADIO_BLE_125KBPS:
            printfp ("coded");
            break;

        case RI_RADIO_BLE_1MBPS:
            printfp ("1_mbit");
            break;

        case RI_RADIO_BLE_2MBPS:
            printfp ("2_mbit");
            break;

        default:
            printfp ("error");
            break;
    }
}



bool ri_communication_radio_run_integration_test (const rd_test_print_fp printfp)
{
    bool status = false;
    printfp ("\"radio\":{\r\n");
    status |= ri_radio_init_test (printfp);
    status |= ri_radio_address_test (printfp);
    printfp ("},\r\n");
    return status;
}

/* @} */
#endif
