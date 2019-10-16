/**
 * Ruuvi Firmware 3.x Tests
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 **/

#ifndef  TEST_SENSOR_H
#define  TEST_SENSOR_H

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include <stdbool.h>

/**
 * @brief Test that sensor init and uninit works as expected.
 *
 * - Sensor must return RUUVI_DRIVER_SUCCESS on first init.
 * - None of the sensor function pointers may be NULL after init
 * - Sensor should return RUUVI_DRIVER_ERROR_INVALID_STATE when initializing sensor which is already init. May return other error if check for it triggers first.
 *   - Sensor structure must be not be altered in failed init.
 *   - This also means that a sensor is "locked" until first successful initializer uninitializes.
 * - Sensor must return RUUVI_DRIVER_SUCCESS on first uninit
 * - All of sensor function pointers must return RUUVI_DRIVER_ERROR_NOT_INITIALIZED after uninit
 * - Sensor configuration is not defined after init, however the sensor must be in lowest-power state available.
 * - Sensor mode_get must return RUUVI_DRIVER_SENSOR_CFG_SLEEP after init.
 * - Sensor configuration is not defined after uninit, however the sensor must be in lowest-power state available.
 *   - Sensor power consumption is not tested by this test.
 * - Sensor initialization must be successful after uninit.
 * - Init and Uninit should return RUUVI_DRIVER_ERROR_NULL if pointer to the sensor struct is NULL. May return other error if check for it triggers first.
 *
 *
 * @param[in] init   Function pointer to sensor initialization
 * @param[in] bus    Bus of the sensor, RUUVI_DRIVER_BUS_NONE, _I2C or _SPI
 * @param[in] handle Handle of the sensor, such as SPI GPIO pin, I2C address or ADC channel.
 *
 * @return @c RUUVI_DRIVER_SUCCESS if the tests passed, error code from the test otherwise.
 */
ruuvi_driver_status_t test_sensor_init(const ruuvi_driver_sensor_init_fp init,
                                       const ruuvi_driver_bus_t bus, const uint8_t handle);

/**
 *  @brief Test that sensor sample rate, scale and resolution setters and getters work as expected.
 *
 * - MIN, MAX, DEFAULT and NO_CHANGE must always return RUUVI_DRIVER_SUCCESS
 * - On any parameter set between 1 and 200, if driver returns SUCCESS, the returned value must be at least as much as what was set.
 * - GET must return same value as SET had as an output.
 * - Get and Set should return RUUVI_DRIVER_ERROR_NULL if pointer to the value is NULL. May return other error if check for it triggers first.
 * - If setting up parameter is not supported, for example on sensor with fixed resolution or single-shot measurements only, return RUUVI_DRIVER_SENSOR_CFG_DEFAULT
 * - Sensor must return RUUVI_DRIVER_ERROR_INVALID_STATE if sensor is not in SLEEP mode while one of parameters is being set
 *
 * @param[in] init:   Function pointer to sensor initialization
 * @param[in] bus:    Bus of the sensor, RUUVI_DRIVER_BUS_NONE, _I2C or _SPI
 * @param[in] handle: Handle of the sensor, such as SPI GPIO pin, I2C address or ADC channel.
 *
 * @return RUUVI_DRIVER_SUCCESS if the tests passed, error code from the test otherwise.
 */
ruuvi_driver_status_t test_sensor_setup(const ruuvi_driver_sensor_init_fp init,
                                        const ruuvi_driver_bus_t bus, const uint8_t handle);

/**
 * @brief Test that sensor modes work as expected
 *
 * - Sensor must be in SLEEP mode after init
 * - Sensor must return all values as INVALID if sensor is read before first sample
 * - Sensor must be in SLEEP mode after mode has been set to SINGLE
 * - Sensor must have new data after setting mode to SINGLE returns
 * - Sensor must have same values, including timestamp, on successive calls to DATA_GET after SINGLE sample
 * - Sensor must stay in CONTINUOUS mode after being set to continuous
 * - Sensor must return RUUVI_DRIVER_ERROR_INVALID_STATE if set to SINGLE while in continuous mode  and remain in continuous mode
 * - Sensor must return RUUVI_DRIVER_ERROR_NULL if null mode is passed as a parameter
 * - Sensor must return updated data in CONTINUOUS mode, at least timestamp has to be updated after two ms wait.
 *   * Sensor is allowed to buffer data in CONTINUOUS mode.
 *   * if data is buffered and more samples are available, sensor must return RUUVI_DRIVER_STATUS_MORE_AVAILABLE
 *
 * @param[in] init     Function pointer to sensor initialization
 * @param[in] bus      Bus of the sensor, RUUVI_DRIVER_BUS_NONE, _I2C, _UART or _SPI
 * @param[in] handle   Handle of the sensor, such as SPI GPIO pin, I2C address or ADC channel.
 *
 * @return @c RUUVI_DRIVER_SUCCESS if the tests passed, error code from the test otherwise.
 */
ruuvi_driver_status_t test_sensor_modes(const ruuvi_driver_sensor_init_fp init,
                                        const ruuvi_driver_bus_t bus, const uint8_t handle);

/**
 * @brief Test that sensor interrupts work as expected
 *
 * Tests level and fifo interrupts.
 * Functions may return @c RUUVI_DRIVER_ERROR_NOT_SUPPORTED, otherwise:
 *  - FIFO read must return samples with different values (noise)
 *  - FIFO full interrupt must trigger after some time when in FIFO mode
 *  - FIFO full interrupt must trigger again after FIFO has been read and filled again
 *  - FIFO full interrupt must not trigger if FIFO is read at fast enough interval
 *  - FIFO full interrupt must not
 *
 * @param[in] init   Function pointer to sensor initialization
 * @param[in] bus    Bus of the sensor, RUUVI_DRIVER_BUS_NONE, _I2C, _UART or _SPI
 * @param[in] handle Handle of the sensor, such as SPI GPIO pin, I2C address or ADC channel.
 * @param[in] interactive True to enable interactive tests which require user to supply interrupt excitation.
 * @param[in] fifo_pin @ref ruuvi_interface_gpio_id_t identifying pin used to detect interrupts from FIFO
 * @param[in] fifo_pin @ref ruuvi_interface_gpio_id_t identifying pin used to detect interrupts from level.
 *
 * @return @c RUUVI_DRIVER_SUCCESS if the tests passed, error code from the test otherwise.
 */
ruuvi_driver_status_t test_sensor_interrupts(const ruuvi_driver_sensor_init_fp init,
    const ruuvi_driver_bus_t bus, const uint8_t handle,
    const bool interactive,
    const ruuvi_interface_gpio_id_t fifo_pin,
    const ruuvi_interface_gpio_id_t level_pin);

/**
 * Register a test as being run. Increments counter of total tests.
 * Read results with test_sensor_status
 *
 * @param[in] passed: True if your test was successful.
 *
 * @return RUUVI_DRIVER_SUCCESS if test was passed.
 * @return RUUVI_DRIVER_ERROR_SELFTEST if test was failed.
 */
ruuvi_driver_status_t test_sensor_register(bool passed);

/**
 * Get total number of tests run and total number of tests passed.
 *
 * @param[in] total:   pointer to value which will be set to the number of tests run
 * @param[in] passed: pointer to value which will be set to the number of tests passed
 *
 * @return RUUVI_DRIVER_SUCCESS if total tests equal passed tests
 * @return RUUVI_DRIVER_ERROR_SELFTEST if 
 */
ruuvi_driver_status_t test_sensor_status(size_t* total, size_t* passed);

#endif