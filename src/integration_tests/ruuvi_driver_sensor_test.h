/**
 * @file ruuvi_driver_sensor_test.h
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief Run sensor integration tests.
 * @date 2020-03-03
 */

#ifndef  SENSOR_TEST_H
#define  SENSOR_TEST_H

/**
 * @addtogroup sensor
 */
/** @{ */
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include "ruuvi_task_sensor.h"
#include <stdbool.h>

/*
 * @brief rd_sensor_run_integration_test
 *        called by run_integration_tests.c/integration_test_sensors
 *
 * @param[in] printfp Function pointer to which test JSON is sent.
 * @param[in] p_sensor_ctx Context of sensor to test.
 * @retval true if error occured in test.
 * @retval false if no errors occured.
 */
bool rd_sensor_run_integration_test (const rd_test_print_fp printfp,
                                     rt_sensor_ctx_t * p_sensor_ctx);

/**
 * @brief Print Ruuvi Sensor data in human readable JSON.
 *
 * Takes Ruuvi sensor data as input, checks the provided fields and
 * LOGs valid data if available and "NAN" if no valid data is available. Pseudocode example:
 * @code
 * float values[3];
 * rd_sensor_t data = {0};
 * data.values = values;
 * data.provided = { // Query H, P, T
 * .datas.humidity_rh = 1,
 * .datas.pressure_pa = 1,
 * .datas.temperature_c = 1
 * };
 * shtcx->data_get(&data); // Provides only H, T
 * rd_sensor_data_print(&data);
 * // Output, char array, \r\n as newline
 * // {
 * // "timestamp_ms": 2341242,
 * // "humidity_rh":"34.561%",
 * // "pressure_pa":"NAN",
 * // "temperature_c":"23.456"
 * // }
 * @endcode
 *
 * @param[in] p_data Pointer to data to print.
 * @param[in] printfp Function to print data with, returns void and takes const char* const as param.
 */
void rd_sensor_data_print (const rd_sensor_data_t * const p_data,
                           const rd_test_print_fp printfp);

/** @} */
#endif
