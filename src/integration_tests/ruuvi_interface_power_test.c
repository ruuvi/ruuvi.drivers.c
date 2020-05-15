#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_RUN_TESTS
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_power.h"
#include <stdbool.h>

bool ri_power_run_integration_test (const rd_test_print_fp printfp,
                                    const ri_power_regulators_t regulators)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"power\":{\r\n");

    if (regulators.DCDC_INTERNAL)
    {
        printfp ("\"dcdc_internal\":");
        ri_power_regulators_t test = {0};
        test.DCDC_INTERNAL = 1;
        err_code = ri_power_regulators_enable (test);

        if (RD_SUCCESS == err_code)
        {
            printfp ("\"pass\"");
        }
        else
        {
            printfp ("\"fail\"");
            status = true;
        }

        if (regulators.DCDC_HV)
        {
            printfp (",\r\n");
        }
        else
        {
            printfp ("\r\n");
        }
    }

    if (regulators.DCDC_HV)
    {
        printfp ("\"dcdc_hv:\"{\r\n");
        ri_power_regulators_t test = {0};
        test.DCDC_HV = 1;
        err_code = ri_power_regulators_enable (test);

        if (RD_SUCCESS == err_code)
        {
            printfp ("\"pass\"\r\n");
        }
        else
        {
            printfp ("\"fail\"\r\n");
            status = true;
        }
    }

    ri_power_regulators_t test = {0};
    err_code = ri_power_regulators_enable (test);
    printfp ("},\r\n");
    return (status || (RD_SUCCESS != err_code));
}
#endif
