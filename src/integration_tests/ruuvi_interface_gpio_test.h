#ifndef RUUVI_INTERFACE_GPIO_TEST_H
#define RUUVI_INTERFACE_GPIO_TEST_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_gpio.h"
#include <stdbool.h>
/**
 * @addtogroup GPIO
 * @{
 */
/**
* @file ruuvi_interface_gpio_test.h
* @author Otso Jousimaa <otso@ojousima.net>
* @date 2019-04-27
* @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
*
* Test functionality defined in @ref ruuvi_interface_gpio.h
*
*/

/** @brief structure to configure GPIO test with input and output. These GPIOs must be physically connected on board. */
typedef struct
{
    ri_gpio_id_t input;  //!< Input pin used in test. Must be interrupt-capable.
    ri_gpio_id_t output; //!< Output pin used in test. Must be PWM-capable.
} rd_test_gpio_cfg_t;

/**
 * @brief Test GPIO module initialization
 *
 * - Interface must return RD_SUCCESS after first call.
 * - Interface must return RD_ERROR_INVALID_STATE when called while already initialized.
 * - Interface must return RD_SUCCESS when called after uninitialization.
 * @return @c RD_SUCCESS if all tests pass, error code on failure
 */
rd_status_t ri_gpio_test_init (void);

/**
 * @brief Test configuring a pin of a port into a mode.
 *
 * - When both pins are in High-Z mode, input is undefined (not tested)
 * - When Input is in High-Z mode, and output mode is INPUT_PULLUP, input must read as HIGH
 * - When Input is in High-Z mode, and output mode is INPUT_PULLDOWN, input must read as LOW
 * - When Input is in INPUT_PULLUP mode, and output is in OUTPUT_LOW mode, input must read as LOW
 * - When Input is in INPUT_PULLDOWN mode, and output is in OUTPUT_HIGH mode, input must read as HIGH
 *
 * @param[in] input  Pin used to check the state of output pin
 * @param[in] output Pin being configured into various modes.
 *
 * @return @c RD_SUCCESS if all tests pass, error code on failure
 */
rd_status_t ri_gpio_test_configure (const ri_gpio_id_t input, const ri_gpio_id_t output);

/**
 * @brief Test toggling the state of a pin of a port.
 *
 * Input is in High-Z mode. Value read by it must toggle after output pin is toggled.
 *
 * @param[in] input  Pin used to check the state of output pin.
 * @param[in] output Pin being toggled.
 *
 * @return @c RD_SUCCESS if all tests pass, error code on failure
 */
rd_status_t ri_gpio_test_toggle (const ri_gpio_id_t input, const ri_gpio_id_t output);

/**
 * @brief Run all GPIO integration tests
 *
 * @param[in] printfp Function pointer to which test result strings are sent.
 * @param[in] input  Pin used to check the state of output pin.
 * @param[in] output Pin being toggled.
 *
 * @return false if there are no errors, true otherwise.
 */
bool ri_gpio_run_integration_test (const rd_test_print_fp printfp,
                                   const ri_gpio_id_t input, const ri_gpio_id_t output);

/** @} */
#endif