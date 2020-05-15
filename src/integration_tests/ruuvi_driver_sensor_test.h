#ifndef  SENSOR_TEST_H
#define  SENSOR_TEST_H

/**
 * @addtogroup sensor
 */
/** @{ */
/**
 * @file ruuvi_interface_communication_nfc_test.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-03-03
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Test implementation of NFC interface.
 */
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_driver_test.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include "ruuvi_task_sensor.h"
#include <stdbool.h>

/*
 * @brief Run sensor integration tests.
 *
 *
 * @param[in] printfp Function pointer to which test JSON is sent.
 * @param[in] p_sensor_ctx Context of sensor to test.
 * @retval true if error occured in test.
 * @retval false if no errors occured.
 */
bool rd_sensor_run_integration_test (const rd_test_print_fp printfp,
                                     rt_sensor_ctx_t * p_sensor_ctx);

/** @} */
#endif
