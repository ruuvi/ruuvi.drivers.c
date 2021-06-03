/**
 * @file ruuvi_driver_sensor_test.c
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief all sensors should act the same so test them the same way
 * @date 2020-06-18
 *.      2021-05-28 show timestamp_ms in uint32_t ARMGCC does not like %lld (Segger ES does)
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include "ruuvi_interface_gpio_interrupt_test.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_driver_sensor_test.h"

#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#if RUUVI_RUN_TESTS

#define RETURN_ON_ERROR(status) if(status)          \
{                                                   \
RD_ERROR_CHECK(RD_ERROR_SELFTEST, ~RD_ERROR_FATAL); \
return status;                                      \
}
#define BITFIELD_MASK         (1U)
#define MAX_LOG_BUFFER_SIZE (128U)
#define MAX_SENSOR_NAME_LEN  (20U)
#define MAX_BITS_PER_BYTE     (8U) //!< Number of bits in a byte.
#define MAX_SENSORS (sizeof(rd_sensor_data_fields_t)* MAX_BITS_PER_BYTE)
#define MAX_RETRIES    (50U) //!< Number of times to run test on statistics-dependent tests, such as sampling noise.
#define MAX_FIFO_DEPTH (32U) //!< How many samples to fetch from FIFO at max
#define MAX_SENSOR_PROVIDED_FIELDS (4U) //!< Largest number of different fields tested sensor can have.

#define LOG_PRINT_DELAY_MS (10U)


static inline void LOG (const char * const msg)
{
    ri_log (RI_LOG_LEVEL_INFO, msg);
    ri_delay_ms (LOG_PRINT_DELAY_MS); // Avoid overflowing log buffer.
}

static volatile bool fifo_int  = false;
static volatile bool level_int = false;

static bool initialize_sensor_once (rd_sensor_t * DUT,
                                    const rd_sensor_init_fp init,
                                    const rd_bus_t bus, const uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = init (DUT, bus, handle);

    if (RD_SUCCESS != err_code)
    {
        return true;
    }

    return false;
}

static bool uninitialize_sensor (rd_sensor_t * DUT,
                                 const rd_sensor_init_fp init,
                                 const rd_bus_t bus, const uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = DUT->uninit (DUT, bus, handle);

    if (RD_SUCCESS != err_code)
    {
        return true;
    }

    return false;
}

static bool initialize_sensor_twice (rd_sensor_t * DUT,
                                     const rd_sensor_init_fp init,
                                     const rd_bus_t bus, const uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = init (DUT, bus, handle);
    err_code = init (DUT, bus, handle);

    if (RD_SUCCESS == err_code)
    {
        RD_ERROR_CHECK (err_code, ~RD_ERROR_FATAL);
        return true;
    }

    return false;
}

static bool validate_sensor_setup (rd_sensor_t * DUT)
{
    // - None of the sensor function pointers may be NULL after init
    if (DUT->init                      == NULL ||
            DUT->uninit                == NULL ||
            DUT->configuration_get     == NULL ||
            DUT->configuration_set     == NULL ||
            DUT->data_get              == NULL ||
            DUT->dsp_get               == NULL ||
            DUT->dsp_set               == NULL ||
            DUT->fifo_enable           == NULL ||
            DUT->fifo_interrupt_enable == NULL ||
            DUT->fifo_read             == NULL ||
            DUT->level_interrupt_set   == NULL ||
            DUT->mode_get              == NULL ||
            DUT->mode_set              == NULL ||
            DUT->resolution_get        == NULL ||
            DUT->resolution_set        == NULL ||
            DUT->samplerate_get        == NULL ||
            DUT->samplerate_set        == NULL ||
            DUT->scale_get             == NULL ||
            DUT->scale_set             == NULL ||
            DUT->name                  == NULL ||
            DUT->provides.bitfield     == 0)
    {
        return true;
    }

    return false;
}

static bool validate_sensor_teardown (rd_sensor_t * DUT)
{
    bool failed = false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->init (NULL, 0, 0))   ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->uninit (NULL, 0, 0)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->configuration_get (NULL,
               NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->configuration_set (NULL,
               NULL))  ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->data_get (NULL))     ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->dsp_get (NULL, NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->dsp_set (NULL, NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->fifo_enable (false)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->fifo_interrupt_enable (
                   false))     ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->fifo_read (NULL,
               NULL))            ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->level_interrupt_set (false,
               NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->mode_get (NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->mode_set (NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->resolution_get (NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->resolution_set (NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->samplerate_get (NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->samplerate_set (NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->scale_get (NULL)) ? true : false;
    failed |= (RD_ERROR_NOT_INITIALIZED != DUT->scale_set (NULL)) ? true : false;
    failed |= (0 != DUT->provides.bitfield) ? true : false;
    return failed;
}

static bool validate_sensor_mode_after_init (rd_sensor_t * DUT)
{
    uint8_t mode;
    rd_status_t err_code = DUT->mode_get (&mode);

    if (RD_SUCCESS != err_code || RD_SENSOR_CFG_SLEEP != mode)
    {
        RD_ERROR_CHECK (RD_ERROR_SELFTEST, ~RD_ERROR_FATAL);
        return true;
    }

    return false;
}

static bool test_sensor_init_on_null (rd_sensor_t * DUT,
                                      const rd_sensor_init_fp init,
                                      const rd_bus_t bus, const uint8_t handle)
{
    rd_status_t err_init = DUT->init (NULL, bus, handle);
    rd_status_t err_uninit = DUT->uninit (NULL, bus, handle);

    if (RD_ERROR_NULL != err_init || RD_ERROR_NULL != err_uninit)
    {
        return true;
    }

    return false;
}

/**
 * @brief Test that sensor init and uninit work as expected.
 *
 * - Sensor must return RD_SUCCESS on first init.
 * - None of the sensor function pointers may be NULL after init
 * - Sensor should return RD_ERROR_INVALID_STATE when initializing sensor which is already init. May return other error if check for it triggers first.
 *   - Sensor structure must be not be altered in failed init.
 *   - This also means that a sensor is "locked" until first successful initializer uninitializes.
 * - Sensor must return RD_SUCCESS on first uninit
 * - All of sensor function pointers must return RD_ERROR_NOT_INITIALIZED after uninit
 * - Sensor configuration is not defined after init, however the sensor must be in lowest-power state available.
 * - Sensor mode_get must return RD_SENSOR_CFG_SLEEP after init.
 * - Sensor configuration is not defined after uninit, however the sensor must be in lowest-power state available.
 *   - Sensor power consumption is not tested by this test.
 * - Sensor initialization must be successful after uninit.
 * - Init and Uninit should return RD_ERROR_NULL if pointer to the sensor struct is NULL. May return other error if check for it triggers first.
 *
 *
 * @param[in] init   Function pointer to sensor initialization
 * @param[in] bus    Bus of the sensor, RD_BUS_NONE, _I2C or _SPI
 * @param[in] handle Handle of the sensor, such as SPI GPIO pin, I2C address or ADC channel.
 *
 * @return false if no errors occurred, true otherwise.
 */
static bool test_sensor_init (const rd_sensor_init_fp init,
                              const rd_bus_t bus, const uint8_t handle)
{
    rd_sensor_t DUT = {0};
    bool failed = false;
    // - Sensor must return RD_SUCCESS on first init.
    failed |= initialize_sensor_once (&DUT, init, bus, handle);
    RETURN_ON_ERROR (failed);
    // - None of the sensor function pointers may be NULL after init
    failed |= validate_sensor_setup (&DUT);
    // - Sensor must return RD_SUCCESS on first uninit
    failed |= uninitialize_sensor (&DUT, init, bus, handle);
    // - All of sensor function pointers must return RD_ERROR_NOT_INITIALIZED after uninit
    failed |= validate_sensor_teardown (&DUT);
    // - Sensor must return RD_ERROR_INVALID_STATE when initializing sensor which is already init
    failed |= initialize_sensor_twice (&DUT, init, bus, handle);
    // - Sensor must return RD_SUCCESS after uninit
    failed |= uninitialize_sensor (&DUT, init, bus, handle);
    // - Sensor initialization must be successful after uninit.
    failed |= initialize_sensor_once (&DUT, init, bus, handle);
    // - Sensor mode_get must return RD_SENSOR_CFG_SLEEP after init.
    failed |= validate_sensor_mode_after_init (&DUT);
    // - Init and Uninit must return RD_ERROR_NULL if pointer to sensor struct is NULL
    failed |= test_sensor_init_on_null (&DUT, init, bus, handle);
    // Uninitialise sensor after test
    DUT.uninit (&DUT, bus, handle);
    return failed;
}

static bool test_sensor_setup_set_get (const rd_sensor_t * DUT,
                                       const rd_sensor_setup_fp set, const rd_sensor_setup_fp get)
{
    rd_status_t err_code = RD_SUCCESS;
    bool failed = false;
    uint8_t config = 0;
    uint8_t original = 0;
    // Test constant values
    uint8_t cfg_constants[] = { RD_SENSOR_CFG_DEFAULT, RD_SENSOR_CFG_MAX, RD_SENSOR_CFG_MIN, RD_SENSOR_CFG_NO_CHANGE };

    for (size_t ii = 0; ii < sizeof (cfg_constants); ii++)
    {
        config  = cfg_constants[ii];
        err_code = set (&config);
        original = config;
        err_code |= get (&config);

        if (config != original ||
                RD_SUCCESS != err_code)
        {
            failed = true;
        }
    }

    if (RD_SUCCESS != err_code)
    {
        failed = true;
    }

    //  Sensor must return RD_ERROR_INVALID_STATE if sensor is not in SLEEP mode while being configured.
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    config = RD_SENSOR_CFG_DEFAULT;
    err_code =  DUT->mode_set (&mode);
    err_code |= set (&config);

    if (RD_ERROR_INVALID_STATE != err_code)
    {
        failed = true;
    }

    mode = RD_SENSOR_CFG_SLEEP;
    err_code = DUT->mode_set (&mode);

    // Test values 1 ... 200
    for (uint8_t ii = 1; ii < 200; ii++)
    {
        config  = ii;
        original = config;
        err_code = set (&config);

        // Set value must be at least requested value
        if (RD_SUCCESS == err_code &&
                original > config)
        {
            failed = true;
        }

        // Get must be as what was returned in set
        original = config;
        err_code |= get (&config);

        if (config != original &&
                RD_SUCCESS == err_code)
        {
            failed = true;
            break;
        }

        // Break on not supported
        if (RD_ERROR_NOT_SUPPORTED == err_code)
        {
            break;
        }

        // Return error on any other error code
        if (RD_SUCCESS != err_code)
        {
            return true;
        }
    }

    // Check NULL check
    err_code = set (NULL);

    if (RD_ERROR_NULL != err_code)
    {
        failed = true;
    }

    err_code = get (NULL);

    if (RD_ERROR_NULL != err_code)
    {
        failed = true;
    }

    return failed;
}

/**
 *  @brief Test that sensor sample rate, scale and resolution setters and getters work as expected.
 *
 * - MIN, MAX, DEFAULT and NO_CHANGE must always return RD_SUCCESS
 * - On any parameter set between 1 and 200, if driver returns SUCCESS, the returned value must be at least as much as what was set.
 * - GET must return same value as SET had as an output.
 * - Get and Set should return RD_ERROR_NULL if pointer to the value is NULL. May return other error if check for it triggers first.
 * - If setting up parameter is not supported, for example on sensor with fixed resolution or single-shot measurements only, return RD_SENSOR_CFG_DEFAULT
 * - Sensor must return RD_ERROR_INVALID_STATE if sensor is not in SLEEP mode while one of parameters is being set
 *
 * @param[in] init:   Function pointer to sensor initialization
 * @param[in] bus:    Bus of the sensor, RD_BUS_NONE, _I2C or _SPI
 * @param[in] handle: Handle of the sensor, such as SPI GPIO pin, I2C address or ADC channel.
 *
 * @return RD_SUCCESS if the tests passed, error code from the test otherwise.
 */
static bool test_sensor_setup (const rd_sensor_init_fp init,
                               const rd_bus_t bus, const uint8_t handle)
{
    // - Sensor must return RD_SUCCESS on first init.
    rd_sensor_t DUT;
    memset (&DUT, 0, sizeof (DUT));
    bool failed = false;
    failed |= init (&DUT, bus, handle);

    if (failed)
    {
        // Return to avoid calling NULL function pointers
        return failed;
    }

    // Test scale
    failed |= test_sensor_setup_set_get (&DUT, DUT.scale_set, DUT.scale_get);
    // Test samplerate
    failed |= test_sensor_setup_set_get (&DUT, DUT.samplerate_set, DUT.samplerate_get);
    // Test resolution
    failed |= test_sensor_setup_set_get (&DUT, DUT.resolution_set, DUT.resolution_get);
    // Uninitialise sensor after test
    DUT.uninit (&DUT, bus, handle);
    return failed;
}

/** @brief Copy new value into old value and return true if new is different from original old value. */
static inline bool value_has_changed (rd_sensor_data_t * old,
                                      const rd_sensor_data_t * const new_d)
{
    bool change = false;

    for (uint8_t ii = 0; ii < rd_sensor_data_fieldcount (old); ii++)
    {
        if (old->data[ii] != new_d->data[ii]) { change = true; }

        old->data[ii] = new_d->data[ii];
    }

    return change;
}

static bool sensor_sleeps_after_init (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode;
    err_code = DUT->mode_get (&mode);

    if (RD_SUCCESS != err_code || RD_SENSOR_CFG_SLEEP != mode)
    {
        return true;
    }

    return false;
}

static bool sensor_returns_invalid_before_sampling (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    float values_new[MAX_SENSOR_PROVIDED_FIELDS];
    rd_sensor_data_t new_data =
    {
        .fields = DUT->provides,
        .data = values_new
    };
    err_code = DUT->data_get (&new_data);

    if (RD_SUCCESS != err_code || new_data.valid.bitfield)
    {
        RD_ERROR_CHECK (RD_ERROR_INTERNAL, ~RD_ERROR_FATAL);
        return true;
    }

    return false;
}

static bool sensor_returns_to_sleep (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode;
    mode = RD_SENSOR_CFG_SINGLE;
    err_code = DUT->mode_set (&mode);

    if (RD_SUCCESS != err_code || RD_SENSOR_CFG_SLEEP != mode)
    {
        RD_ERROR_CHECK (RD_ERROR_INTERNAL, ~RD_ERROR_FATAL);
        return true;
    }

    return false;
}

static bool sensor_returns_valid_data (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode;
    rd_sensor_data_t data = {0};
    float values [MAX_SENSOR_PROVIDED_FIELDS];
    data.fields = DUT->provides;
    data.data = values;
    mode = RD_SENSOR_CFG_SINGLE;
    err_code = DUT->mode_set (&mode);
    err_code |= DUT->data_get (&data);

    if (RD_SUCCESS != err_code ||
            (DUT->provides.bitfield != data.valid.bitfield) ||
            RD_UINT64_INVALID == data.timestamp_ms)
    {
        RD_ERROR_CHECK (RD_ERROR_INTERNAL, ~RD_ERROR_FATAL);
        return true;
    }

    return false;
}

static bool single_sample_stays_valid (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    float old_values[MAX_SENSOR_PROVIDED_FIELDS] = {0};
    float new_values[MAX_SENSOR_PROVIDED_FIELDS] = {0};
    rd_sensor_data_t old_data = {.fields = DUT->provides,
                                 .data   = old_values
                                };
    rd_sensor_data_t new_data = {.fields = DUT->provides,
                                 .data   = new_values
                                };
    err_code = DUT->mode_set (&mode);
    err_code |= DUT->data_get (&old_data);
    ri_delay_ms (2); // wait 2 ms to ensure timestamp is not changed.
    err_code |= DUT->data_get (&new_data);

    if (RD_SUCCESS != err_code ||
            old_data.timestamp_ms != new_data.timestamp_ms ||
            0 != memcmp (old_values, new_values, sizeof (old_values)))
    {
        RD_ERROR_CHECK (RD_ERROR_INTERNAL, ~RD_ERROR_FATAL);
        return true;
    }

    return false;
}

static bool sensor_remains_continuous (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    mode = RD_SENSOR_CFG_CONTINUOUS;
    err_code = DUT->mode_set (&mode);
    ri_delay_ms (20);
    err_code |= DUT->mode_get (&mode);

    if (RD_SUCCESS != err_code || RD_SENSOR_CFG_CONTINUOUS != mode)
    {
        return true;
    }

    return false;
}

static bool sensor_rejects_single_on_continuous (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    err_code = DUT->mode_set (&mode);

    if (RD_ERROR_INVALID_STATE != err_code
            || RD_SENSOR_CFG_CONTINUOUS != mode)
    {
        return true;
    }

    return false;
}

static bool sensor_mode_cannot_be_null (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = DUT->mode_set (NULL);

    if (RD_ERROR_NULL != err_code)
    {
        return true;
    }

    err_code = DUT->mode_get (NULL);

    if (RD_ERROR_NULL != err_code)
    {
        return true;
    }

    return false;
}

static bool sensor_returns_continuous_data (const rd_sensor_t * const DUT)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    float old_values[MAX_SENSOR_PROVIDED_FIELDS];
    float new_values[MAX_SENSOR_PROVIDED_FIELDS];
    rd_sensor_data_t old_data = {.fields = DUT->provides,
                                 .data   = old_values
                                };
    rd_sensor_data_t new_data = {.fields = DUT->provides,
                                 .data   = new_values
                                };
    err_code = DUT->mode_set (&mode);
    err_code |= DUT->data_get (&old_data);
    ri_delay_ms (2); // wait 2 ms to ensure timestamp is not changed.
    err_code |= DUT->data_get (&new_data);
    err_code = DUT->mode_set (&mode);
    uint8_t samplerate = RD_SENSOR_CFG_MAX;
    err_code |= DUT->samplerate_set (&samplerate);
    mode = RD_SENSOR_CFG_CONTINUOUS;
    err_code |= DUT->mode_set (&mode);
    uint32_t interval = (1000 / (samplerate + 1));
    ri_delay_ms (2 * interval);
    err_code |= DUT->data_get (&old_data);
    int retries = 0;

    for (; retries < MAX_RETRIES; retries++)
    {
        ri_delay_ms (2 * interval);
        new_data.valid.bitfield = 0;
        new_data.timestamp_ms = RD_SENSOR_INVALID_TIMSTAMP;
        err_code |= DUT->data_get (&new_data);

        if (old_data.timestamp_ms == new_data.timestamp_ms || RD_SUCCESS != err_code)
        {
            if (RD_STATUS_MORE_AVAILABLE == err_code)
            {
                do
                {
                    err_code |= DUT->data_get (&new_data);
                } while (RD_STATUS_MORE_AVAILABLE == err_code);

                continue;
            }
            else
            {
                return true;
            }
        }

        if (value_has_changed (&old_data, &new_data)) { break; }
    }

    if (MAX_RETRIES == retries)
    {
        return true;
    }

    return false;
}

/**
 * @brief Test that sensor modes work as expected
 *
 * - Sensor must be in SLEEP mode after init
 * - Sensor must return all values as INVALID if sensor is read before first sample
 * - Sensor must be in SLEEP mode after mode has been set to SINGLE
 * - Sensor must have new data after setting mode to SINGLE returns
 * - Sensor must have same values, including timestamp, on successive calls to DATA_GET after SINGLE sample
 * - Sensor must stay in CONTINUOUS mode after being set to continuous
 * - Sensor must return RD_ERROR_INVALID_STATE if set to SINGLE while in continuous mode  and remain in continuous mode
 * - Sensor must return RD_ERROR_NULL if null mode is passed as a parameter
 * - Sensor must return updated data in CONTINUOUS mode, at least timestamp has to be updated after two ms wait.
 *   * Sensor is allowed to buffer data in CONTINUOUS mode.
 *   * if data is buffered and more samples are available, sensor must return RD_STATUS_MORE_AVAILABLE
 *
 * @param[in] init     Function pointer to sensor initialization
 * @param[in] bus      Bus of the sensor, RD_BUS_NONE, _I2C, _UART or _SPI
 * @param[in] handle   Handle of the sensor, such as SPI GPIO pin, I2C address or ADC channel.
 *
 * @return @c RD_SUCCESS if the tests passed, error code from the test otherwise.
 */
static bool test_sensor_modes (const rd_sensor_init_fp init,
                               const rd_bus_t bus, const uint8_t handle)
{
    bool failed = false;
    rd_sensor_t DUT;
    memset (&DUT, 0, sizeof (DUT));
    initialize_sensor_once (&DUT, init, bus, handle);
    // - Sensor must be in SLEEP mode after init
    failed |= sensor_sleeps_after_init (&DUT);
    // - Sensor must return all values as INVALID if sensor is read before first sample
    failed |= sensor_returns_invalid_before_sampling (&DUT);
    // - Sensor must be in SLEEP mode after mode has been set to SINGLE
    failed |= sensor_returns_to_sleep (&DUT);
    // - Sensor must have new data after setting mode to SINGLE returns
    failed |= sensor_returns_valid_data (&DUT);
    // - Sensor must have same values, including timestamp, on successive calls to DATA_GET after SINGLE sample
    failed |= single_sample_stays_valid (&DUT);
    // - Sensor must stay in CONTINUOUS mode after being set to continuous.
    failed |= sensor_remains_continuous (&DUT);
    // - Sensor must return RD_ERROR_INVALID_STATE if set to SINGLE while in continuous mode and remain in continuous mode
    failed |= sensor_rejects_single_on_continuous (&DUT);
    // - Sensor must return RD_ERROR_NULL if null mode is passed as a parameter
    failed |= sensor_mode_cannot_be_null (&DUT);
    // Sensor must return updated data in CONTINUOUS mode, at least timestamp has to be updated after two ms wait.
    failed |= sensor_returns_continuous_data (&DUT);
    // Uninitialise sensor after test
    failed |= DUT.uninit (&DUT, bus, handle);
    return failed;
}

static void on_fifo (const ri_gpio_evt_t evt)
{
    fifo_int = true;
}

static void on_level (const ri_gpio_evt_t evt)
{
    level_int = true;
}

/**
 *  @brief Prepare GPIOs and initialize sensor for tests.
 *
 *  @param[out] DUT Sensor to configure.
 *  @param[in]  init function to initialize sensor.
 *  @param[out] interrupt_table Table of function pointers to configure with interrupts.
 *  @param[in]  fifo_pin Pin to register FIFO interrupts.
 *  @param[in]  level_pin Pin to register level interrupts.
 *  @return RD_SUCCESS on successful initialization.
 *  @return RD_ERROR_SELFTEST if initialization fails.
 */
static bool test_sensor_interrupts_setup (rd_sensor_t * DUT,
        rd_sensor_init_fp const init,
        const rd_bus_t bus, const uint8_t handle,
        ri_gpio_interrupt_fp_t * const interrupt_table,
        const ri_gpio_id_t fifo_pin,
        const ri_gpio_id_t level_pin)
{
    rd_status_t err_code = RD_SUCCESS;

    if (ri_gpio_interrupt_is_init())
    {
        err_code |= ri_gpio_interrupt_uninit();
    }

    err_code |= ri_gpio_interrupt_init (interrupt_table,
                                        RI_GPIO_INTERRUPT_TEST_TABLE_SIZE);
    err_code |= ri_gpio_interrupt_enable (fifo_pin, RI_GPIO_SLOPE_LOTOHI,
                                          RI_GPIO_MODE_INPUT_PULLUP, on_fifo);
    err_code |= ri_gpio_interrupt_enable (level_pin, RI_GPIO_SLOPE_LOTOHI,
                                          RI_GPIO_MODE_INPUT_PULLUP, on_level);
    // - Sensor must return RD_SUCCESS on first init.
    memset (DUT, 0, sizeof (rd_sensor_t));
    err_code |= init (DUT, bus, handle);
    err_code |= DUT->fifo_interrupt_enable (true);
    return (RD_SUCCESS != err_code);
}

/** @brief Uninitialize GPIOs and sensor after tests
  *  @param[out] DUT Sensor to configure
  */
static void test_sensor_interrupts_teardown (rd_sensor_t * const DUT,
        rd_sensor_init_fp const init,
        const rd_bus_t bus, const uint8_t handle,
        const ri_gpio_id_t fifo_pin,
        const ri_gpio_id_t level_pin)
{
    DUT->fifo_interrupt_enable (false);
    ri_gpio_interrupt_disable (fifo_pin);
    ri_gpio_interrupt_disable (level_pin);
    DUT->uninit (DUT, bus, handle);
    ri_gpio_interrupt_uninit();
}

/** @brief  - LEVEL return status of interrupt occurance */
static rd_status_t test_sensor_level_enable (const rd_sensor_t * DUT)
{
    float threshold_g = APP_MOTION_THRESHOLD;
    DUT->level_interrupt_set (true, &threshold_g);
    rd_sensor_configuration_t config = {0};
    config.samplerate = 10;
    config.mode = RD_SENSOR_CFG_CONTINUOUS;
    DUT->configuration_set (DUT, &config);
    level_int = false;
    // Wait for LEVEL interrupt
    uint32_t timeout = 0;
    uint32_t max_time = 5U * 1000U * 1000U;

    while ( (!level_int) && (timeout < max_time))
    {
        timeout += 10;
        ri_delay_us (10);
    }

    if (timeout >= max_time)
    {
        return RD_ERROR_TIMEOUT;
    }

    return (level_int) ? false : true;
}

/** @brief  - FIFO read must return samples with different values (noise) */
static rd_status_t test_sensor_fifo_enable (const rd_sensor_t * DUT)
{
    DUT->fifo_enable (true);
    rd_sensor_configuration_t config = {0};
    config.samplerate = 10;
    config.mode = RD_SENSOR_CFG_CONTINUOUS;
    DUT->configuration_set (DUT, &config);
    fifo_int = false;
    ri_delay_ms (100);
    rd_sensor_data_t old;
    float old_values[MAX_SENSOR_PROVIDED_FIELDS];
    old.data = old_values;
    old.fields.bitfield = DUT->provides.bitfield;
    size_t num_samples = MAX_FIFO_DEPTH;
    rd_sensor_data_t data[MAX_FIFO_DEPTH] = { 0 };
    float values[num_samples][MAX_SENSOR_PROVIDED_FIELDS];
    uint32_t max_time = 4U * 1000U * 1000U;

    for (size_t ii = 0; ii < num_samples; ii++)
    {
        data[ii].data = values[ii];
        data[ii].fields.bitfield = DUT->provides.bitfield;
    }

    bool valid_data = false;
    // Wait for FIFO interrupt
    uint32_t timeout = 0;

    while ( (!fifo_int) && (timeout < max_time))
    {
        timeout += 10;
        ri_delay_us (10);
    }

    if (timeout >= max_time)
    {
        return RD_ERROR_TIMEOUT;
    }

    DUT->fifo_read (&num_samples, data);

    if (10U > num_samples)
    {
        return RD_ERROR_SELFTEST;
    }

    // Check that FIFO has new values
    value_has_changed (&old, & (data[0]));

    for (size_t iii = 1; iii < num_samples; iii++)
    {
        if (value_has_changed (&old,  & (data[iii])))
        {
            valid_data = true;
            break;
        }
    }

    return (valid_data) ? false : true;
}


/**
 * @brief Test that sensor interrupts work as expected
 *
 * Tests level and fifo interrupts.
 * Functions may return @c RD_ERROR_NOT_SUPPORTED, otherwise:
 *  - FIFO read must return samples with different values (noise)
 *  - FIFO full interrupt must trigger after some time when in FIFO mode
 *  - FIFO full interrupt must trigger again after FIFO has been read and filled again
 *  - FIFO full interrupt must not trigger if FIFO is read at fast enough interval
 *  - FIFO full interrupt must not
 *
 * @param[in] init   Function pointer to sensor initialization
 * @param[in] bus    Bus of the sensor, RD_BUS_NONE, _I2C, _UART or _SPI
 * @param[in] handle Handle of the sensor, such as SPI GPIO pin, I2C address or ADC channel.
 * @param[in] interactive True to enable interactive tests which require user to supply interrupt excitation.
 * @param[in] fifo_pin @ref ri_gpio_id_t identifying pin used to detect interrupts from FIFO
 * @param[in] fifo_pin @ref ri_gpio_id_t identifying pin used to detect interrupts from level.
 *
 * @return @c RD_SUCCESS if the tests passed, error code from the test otherwise.
 */
static bool test_sensor_interrupts (const rd_sensor_init_fp init,
                                    const rd_bus_t bus, const uint8_t handle,
                                    const bool interactive,
                                    const ri_gpio_id_t fifo_pin,
                                    const ri_gpio_id_t level_pin,
                                    const rd_test_print_fp printfp)
{
    ri_gpio_interrupt_fp_t
    interrupt_table[RI_GPIO_INTERRUPT_TEST_TABLE_SIZE];
    rd_sensor_t DUT;
    rd_status_t status;
    status = test_sensor_interrupts_setup (&DUT, init, bus, handle, interrupt_table, fifo_pin,
                                           level_pin);

    if (RD_SUCCESS == status)
    {
        printfp ("{\r\n\"level\":");
        status |= test_sensor_level_enable (&DUT);

        if (status)
        {
            printfp ("\"fail\",\r\n");
        }
        else
        {
            printfp ("\"pass\",\r\n");
        }

        test_sensor_interrupts_teardown (&DUT, init, bus, handle, fifo_pin,
                                         level_pin);
        status = test_sensor_interrupts_setup (&DUT, init, bus, handle, interrupt_table, fifo_pin,
                                               level_pin);
        printfp ("\"fifo\":");
        status |= test_sensor_fifo_enable (&DUT);

        if (status)
        {
            printfp ("\"fail\"\r\n");
        }
        else
        {
            printfp ("\"pass\"\r\n");
        }

        printfp ("},");
    }

    test_sensor_interrupts_teardown (&DUT, init, bus, handle, fifo_pin,
                                     level_pin);
    return status;
}

static bool sensor_returns_valid_data_print (const rd_sensor_t * const DUT,
        const rd_test_print_fp printfp)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode;
    rd_sensor_data_t data = {0};
    float values [MAX_SENSOR_PROVIDED_FIELDS];
    data.fields = DUT->provides;
    data.data = values;
    mode = RD_SENSOR_CFG_SINGLE;
    err_code = DUT->mode_set (&mode);
    err_code |= DUT->data_get (&data);

    if (RD_SUCCESS != err_code)
    {
        RD_ERROR_CHECK (RD_ERROR_INTERNAL, ~RD_ERROR_FATAL);
        return true;
    }

    rd_sensor_data_print (&data, printfp);
    return false;
}

static bool test_sensor_data_print (const rd_sensor_init_fp init,
                                    const rd_bus_t bus, const uint8_t handle,
                                    const rd_test_print_fp printfp)
{
    // - Sensor must return RD_SUCCESS on first init.
    rd_sensor_t DUT;
    memset (&DUT, 0, sizeof (DUT));
    bool failed = false;
    failed |= init (&DUT, bus, handle);

    if (failed)
    {
        // Return to avoid calling NULL function pointers
        printfp ("\"data\":\"fail\",\r\n");
        return failed;
    }

    failed |= sensor_returns_valid_data_print (&DUT, printfp);
    DUT.uninit (&DUT, bus, handle);
    return failed;
}

bool rd_sensor_run_integration_test (const rd_test_print_fp printfp,
                                     rt_sensor_ctx_t * p_sensor_ctx)
{
    bool status = false;
    rd_status_t err_code = RD_SUCCESS;
    printfp ("\"");
    printfp (p_sensor_ctx->sensor.name);
    printfp ("\": {\r\n");
    printfp ("\"init\":");

    if (RD_HANDLE_UNUSED == p_sensor_ctx->handle)
    {
        err_code |= RD_ERROR_NOT_FOUND;
    }
    else
    {
        err_code |= p_sensor_ctx->init (&p_sensor_ctx->sensor,
                                        p_sensor_ctx->bus,
                                        p_sensor_ctx->handle);
    }

    if (RD_ERROR_NOT_FOUND == err_code)
    {
        printfp ("\"skip\"\r\n");
        p_sensor_ctx->sensor.uninit (&p_sensor_ctx->sensor, p_sensor_ctx->bus,
                                     p_sensor_ctx->handle);
    }
    else
    {
        p_sensor_ctx->sensor.uninit (&p_sensor_ctx->sensor, p_sensor_ctx->bus,
                                     p_sensor_ctx->handle);
        status = test_sensor_init (p_sensor_ctx->init, p_sensor_ctx->bus, p_sensor_ctx->handle);

        if (status)
        {
            printfp ("\"fail\",\r\n");
        }
        else
        {
            printfp ("\"pass\",\r\n");
        }

        printfp ("\"modes\":");

        if (!status)
        {
            status = test_sensor_modes (p_sensor_ctx->init, p_sensor_ctx->bus, p_sensor_ctx->handle);
        }

        if (status)
        {
            printfp ("\"fail\",\r\n");
        }
        else
        {
            printfp ("\"pass\",\r\n");
        }

        printfp ("\"configuration\":");

        if (!status)
        {
            status = test_sensor_setup (p_sensor_ctx->init, p_sensor_ctx->bus, p_sensor_ctx->handle);
        }

        if (status)
        {
            printfp ("\"fail\",\r\n");
        }
        else
        {
            printfp ("\"pass\",\r\n");
        }

        printfp ("\"interrupts\":");

        if ( (RI_GPIO_ID_UNUSED != p_sensor_ctx->fifo_pin)
                && (RI_GPIO_ID_UNUSED != p_sensor_ctx->level_pin)
                && (!status))
        {
            status = test_sensor_interrupts (p_sensor_ctx->init, p_sensor_ctx->bus,
                                             p_sensor_ctx->handle, false,
                                             p_sensor_ctx->fifo_pin,
                                             p_sensor_ctx->level_pin,
                                             printfp);
        }
        else
        {
            printfp ("\"skipped\",\r\n");
        }

        status = test_sensor_data_print (p_sensor_ctx->init, p_sensor_ctx->bus,
                                         p_sensor_ctx->handle, printfp);
    }

    printfp ("}");
    return status;
}

void rd_sensor_data_print (const rd_sensor_data_t * const p_data,
                           const rd_test_print_fp printfp)
{
    uint8_t data_counter = 0;
    uint8_t data_available = 0;
    uint32_t data_check = p_data->fields.bitfield;
    char sensors_name[MAX_SENSORS][MAX_SENSOR_NAME_LEN] =
    {
        "acceleration_x_g",
        "acceleration_y_g",
        "acceleration_z_g",
        "co2_ppm",
        "gyro_x_dps",
        "gyro_y_dps",
        "gyro_z_dps",
        "humidity_rh",
        "luminosity",
        "magnetometer_x_g",
        "magnetometer_y_g",
        "magnetometer_z_g",
        "pm_1_ugm3",
        "pm_2_ugm3",
        "pm_4_ugm3",
        "pm_10_ugm3",
        "pressure_pa",
        "spl_dbz",
        "temperature_c",
        "voc_ppm",
        "voltage_v",
        "voltage_ratio",
    };

    /* Count enabled sensors */
    for (int i = 0; i < MAX_SENSORS; i++)
    {
        if (data_check & BITFIELD_MASK)
        { data_available++; }

        data_check = data_check >> 1;
    }

    if (NULL != p_data)
    {
        printfp ("\"data\":{\r\n");
        char msg[MAX_LOG_BUFFER_SIZE];

        if (RD_UINT64_INVALID == p_data->timestamp_ms)
        {
            snprintf (msg, sizeof (msg), "\"timestamp_ms\": \"RD_UINT64_INVALID\",\n");
        }
        else
        {
            // Cast to 32 bits, tests are run at boot so there is no danger of overflow.
            snprintf (msg, sizeof (msg), "\"timestamp_ms\": \"%ld\",\n",
                      (uint32_t) p_data->timestamp_ms);
        }

        printfp (msg);

        for (uint8_t i = 0; i < MAX_SENSORS; i++)
        {
            if ( (p_data->fields.bitfield >> i) &
                    BITFIELD_MASK)
            {
                char msg[MAX_LOG_BUFFER_SIZE];

                if ( (p_data->valid.bitfield >> i) &
                        BITFIELD_MASK)
                {
                    if (0 == isnan (* ( (float *) (&p_data->data[data_counter]))))
                    {
                        snprintf (msg, sizeof (msg),
                                  "\"%s\": \"%.2f\"",
                                  (char *) &sensors_name[i][0],
                                  * ( (float *) (&p_data->data[data_counter])));
                    }
                    else
                    {
                        snprintf (msg, sizeof (msg),
                                  "\"%s\": \"NAN\"",
                                  (char *) &sensors_name[i][0]);
                    }

                    data_counter++;
                }
                else
                {
                    snprintf (msg, sizeof (msg),
                              "\"%s\": \"NAN\"",
                              (char *) &sensors_name[i][0]);
                }

                if (data_counter == data_available)
                {
                    char * str = "\r\n";
                    strncat (msg, str, sizeof (str));
                }
                else
                {
                    char * str = ",\r\n";
                    strncat (msg, str, sizeof (str));
                }

                printfp (msg);
            }
        }

        printfp ("}\r\n");
    }
}

#else    //RUUVI_RUN_TESTS

// Dummy implementation
rd_status_t test_sensor_status (size_t * total, size_t * passed)
{
    return RD_SUCCESS;
}

// Dummy implementation
void test_sensor_run (void)
{}
#endif
