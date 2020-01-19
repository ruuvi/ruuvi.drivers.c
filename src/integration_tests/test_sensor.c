#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include "ruuvi_interface_gpio_interrupt_test.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_yield.h"
#include "test_sensor.h"

#include <float.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#if RUUVI_RUN_TESTS

#define RETURN_ON_ERROR(status) if(RUUVI_DRIVER_SUCCESS != status) {return status;}
#define MAX_RETRIES 50 //!< brief Number of times to run test on statistics-dependent tests, such as sampling noise.
static size_t m_total  = 0;
static size_t m_passed = 0;
static volatile bool fifo_int  = false;
static volatile bool level_int = false;

static bool initialize_sensor_once(ruuvi_driver_sensor_t* DUT, const ruuvi_driver_sensor_init_fp init,
                                       const ruuvi_driver_bus_t bus, const uint8_t handle)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code = init(DUT, bus, handle);
  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool uninitialize_sensor(ruuvi_driver_sensor_t* DUT, const ruuvi_driver_sensor_init_fp init,
                                       const ruuvi_driver_bus_t bus, const uint8_t handle)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code = DUT->uninit(DUT, bus, handle);
  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool initialize_sensor_twice(ruuvi_driver_sensor_t* DUT, const ruuvi_driver_sensor_init_fp init,
                                       const ruuvi_driver_bus_t bus, const uint8_t handle)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code = init(DUT, bus, handle);
  err_code = init(DUT, bus, handle);
  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool validate_sensor_setup(ruuvi_driver_sensor_t* DUT)
{
  // - None of the sensor function pointers may be NULL after init
  if( DUT->init                  == NULL ||
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
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool validate_sensor_teardown(ruuvi_driver_sensor_t* DUT)
{
  bool failed = false;
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->init(NULL, 0, 0))                 ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->uninit(NULL, 0, 0))               ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->configuration_get(NULL, NULL))    ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->configuration_set(NULL, NULL))    ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->data_get(NULL))                   ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->dsp_get(NULL, NULL))              ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->dsp_set(NULL, NULL))              ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->fifo_enable(false))               ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->fifo_interrupt_enable(false))     ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->fifo_read(NULL, NULL))            ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->level_interrupt_set(false, NULL)) ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->mode_get(NULL))                   ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->mode_set(NULL))                   ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->resolution_get(NULL))             ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->resolution_set(NULL))             ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->samplerate_get(NULL))             ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->samplerate_set(NULL))             ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->scale_get(NULL))                  ? true : false; 
  failed |= (RUUVI_DRIVER_ERROR_NOT_INITIALIZED != DUT->scale_set(NULL))                  ? true : false; 
  failed |= (0                                  != DUT->provides.bitfield)                ? true : false; 
  if(failed) { RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL); }

  return !failed;
}

static bool validate_sensor_mode_after_init(ruuvi_driver_sensor_t* DUT)
{
  uint8_t mode;
  ruuvi_driver_status_t err_code = DUT->mode_get(&mode);

  if(RUUVI_DRIVER_SUCCESS != err_code || RUUVI_DRIVER_SENSOR_CFG_SLEEP != mode)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool test_sensor_init_on_null(ruuvi_driver_sensor_t* DUT, const ruuvi_driver_sensor_init_fp init,
                                       const ruuvi_driver_bus_t bus, const uint8_t handle)
{
  ruuvi_driver_status_t err_init = DUT->init(NULL, bus, handle);
  ruuvi_driver_status_t err_uninit = DUT->uninit(NULL, bus, handle);
  if(RUUVI_DRIVER_ERROR_NULL != err_init || RUUVI_DRIVER_ERROR_NULL != err_uninit)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_SELFTEST, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

ruuvi_driver_status_t test_sensor_init(const ruuvi_driver_sensor_init_fp init,
                                       const ruuvi_driver_bus_t bus, const uint8_t handle)
{
  ruuvi_driver_sensor_t DUT;
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  // - Sensor must return RUUVI_DRIVER_SUCCESS on first init.
  err_code |= test_sensor_register(initialize_sensor_once(&DUT, init, bus, handle));
  RETURN_ON_ERROR(err_code);
  // - None of the sensor function pointers may be NULL after init
  err_code |= test_sensor_register(validate_sensor_setup(&DUT));
  // - Sensor must return RUUVI_DRIVER_SUCCESS on first uninit
  err_code |= test_sensor_register(uninitialize_sensor(&DUT, init, bus, handle));
  // - All of sensor function pointers must return RUUVI_DRIVER_ERROR_NOT_INITIALIZED after uninit
  err_code |= test_sensor_register(validate_sensor_teardown(&DUT));
  // - Sensor must return RUUVI_DRIVER_ERROR_INVALID_STATE when initializing sensor which is already init
  err_code |= test_sensor_register(initialize_sensor_twice(&DUT, init, bus, handle));
  // - Sensor must return RUUVI_DRIVER_SUCCESS after uninit
  err_code |= test_sensor_register(uninitialize_sensor(&DUT, init, bus, handle));
  // - Sensor initialization must be successful after uninit.
  err_code |= test_sensor_register(initialize_sensor_once(&DUT, init, bus, handle));
  // - Sensor mode_get must return RUUVI_DRIVER_SENSOR_CFG_SLEEP after init.
  err_code |= test_sensor_register(validate_sensor_mode_after_init(&DUT));
  // - Init and Uninit must return RUUVI_DRIVER_ERROR_NULL if pointer to sensor struct is NULL
  err_code |= test_sensor_register(test_sensor_init_on_null(&DUT, init, bus, handle));
  // Uninitialise sensor after test
  DUT.uninit(&DUT, bus, handle);

  return err_code;
}

static ruuvi_driver_status_t test_sensor_setup_set_get(const ruuvi_driver_sensor_t* DUT,
    const ruuvi_driver_sensor_setup_fp set, const ruuvi_driver_sensor_setup_fp get)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t config = 0;
  uint8_t original = 0;
  bool failed = false;
  // Test constant values
  uint8_t cfg_constants[] = { RUUVI_DRIVER_SENSOR_CFG_DEFAULT, RUUVI_DRIVER_SENSOR_CFG_MAX, RUUVI_DRIVER_SENSOR_CFG_MIN, RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE };

  for(size_t ii = 0; ii < sizeof(cfg_constants); ii++)
  {
    config  = cfg_constants[ii];
    err_code = set(&config);
    original = config;
    err_code |= get(&config);

    if(config != original ||
        RUUVI_DRIVER_SUCCESS != err_code)
    {
      failed = true;
      RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    }
  }

  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    failed = true;
  }

  //  Sensor must return RUUVI_DRIVER_ERROR_INVALID_STATE if sensor is not in SLEEP mode while being configured.
  uint8_t mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
  config = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  err_code =  DUT->mode_set(&mode);
  err_code |= set(&config);

  if(RUUVI_DRIVER_ERROR_INVALID_STATE != err_code)
  {
    failed = true;
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
  }

  mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
  err_code = DUT->mode_set(&mode);

  // Test values 1 ... 200
  for(uint8_t ii = 1; ii < 200; ii++)
  {
    config  = ii;
    original = config;
    err_code = set(&config);

    // Set value must be at least requested value
    if(RUUVI_DRIVER_SUCCESS == err_code &&
        original > config)
    {
      failed = true;
      RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    }

    // Get must be as what was returned in set
    original = config;
    err_code |= get(&config);

    if(config != original &&
        RUUVI_DRIVER_SUCCESS == err_code)
    {
      failed = true;
      RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
      break;
    }

    // Break on not supported
    if(RUUVI_DRIVER_ERROR_NOT_SUPPORTED == err_code)
    {
      break;
    }

    // Return error on any other error code
    if(RUUVI_DRIVER_SUCCESS != err_code)
    {
      return RUUVI_DRIVER_ERROR_SELFTEST;
    }
  }

  // Check NULL check
  err_code = set(NULL);

  if(RUUVI_DRIVER_ERROR_NULL != err_code)
  {
    failed = true;
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
  }

  err_code = get(NULL);

  if(RUUVI_DRIVER_ERROR_NULL != err_code)
  {
    failed = true;
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
  }

  if(failed)
  {
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t test_sensor_setup(const ruuvi_driver_sensor_init_fp init,
                                        const ruuvi_driver_bus_t bus, const uint8_t handle)
{
  // - Sensor must return RUUVI_DRIVER_SUCCESS on first init.
  ruuvi_driver_sensor_t DUT;
  memset(&DUT, 0, sizeof(DUT));
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  bool failed = false;
  bool test_ok = true;
  err_code = init(&DUT, bus, handle);

  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    failed = true;
    // Init is test elsewhere, do not register result. return to avoid calling NULL function pointers
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  // Test scale
  err_code = test_sensor_setup_set_get(&DUT, DUT.scale_set, DUT.scale_get);

  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    failed = true;
    test_ok = false;
  }

  test_sensor_register(test_ok);
  test_ok = true;
  // Test samplerate
  err_code = test_sensor_setup_set_get(&DUT, DUT.samplerate_set, DUT.samplerate_get);

  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    failed = true;
    test_ok = false;
  }

  test_sensor_register(test_ok);
  test_ok = true;
  // Test resolution
  err_code = test_sensor_setup_set_get(&DUT, DUT.resolution_set, DUT.resolution_get);

  if(RUUVI_DRIVER_SUCCESS != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
    failed = true;
    test_ok = false;
  }

  test_sensor_register(test_ok);
  test_ok = true;
  // Uninitialise sensor after test
  DUT.uninit(&DUT, bus, handle);

  if(failed)
  {
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  return RUUVI_DRIVER_SUCCESS;
}

/** @brief copy new value into old value and return true if new is different from original old value*/
static inline bool value_has_changed(ruuvi_driver_sensor_data_t* old,
                                     const ruuvi_driver_sensor_data_t* const new_d)
{
  bool change = false;
  for(uint8_t ii = 0; ii < ruuvi_driver_sensor_data_fieldcount(old); ii++)
  {
    if(old->data[ii] != new_d->data[ii]) { change = true; }
    old->data[ii] = new_d->data[ii];
  }
  return change;
}

static bool sensor_sleeps_after_init(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t mode;
  err_code = DUT->mode_get(&mode);

  if(RUUVI_DRIVER_SUCCESS != err_code || RUUVI_DRIVER_SENSOR_CFG_SLEEP != mode)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool sensor_returns_invalid_before_sampling(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  float values_new[32]; // conserve excess space for simplicity.
  ruuvi_driver_sensor_data_t new_data = {.fields = DUT->provides,
                                         .data = values_new};
  err_code = DUT->data_get(&new_data);

  if(RUUVI_DRIVER_SUCCESS != err_code || new_data.valid.bitfield)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool sensor_returns_to_sleep(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t mode;
  mode = RUUVI_DRIVER_SENSOR_CFG_SINGLE;
  err_code = DUT->mode_set(&mode);

  if(RUUVI_DRIVER_SUCCESS != err_code || RUUVI_DRIVER_SENSOR_CFG_SLEEP != mode)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool sensor_returns_valid_data(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t mode;
  ruuvi_driver_sensor_data_t data = {0};
  float values [32];
  data.fields = DUT->provides;
  data.data = values;
  mode = RUUVI_DRIVER_SENSOR_CFG_SINGLE;
  err_code = DUT->mode_set(&mode);
  err_code |= DUT->data_get(&data);

  if(RUUVI_DRIVER_SUCCESS != err_code || 
     (DUT->provides.bitfield != data.valid.bitfield) ||
     RUUVI_DRIVER_UINT64_INVALID == data.timestamp_ms)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool single_sample_stays_valid(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t mode = RUUVI_DRIVER_SENSOR_CFG_SINGLE;
  float old_values[32] = {0};
  float new_values[32] = {0};
  ruuvi_driver_sensor_data_t old_data = {.fields = DUT->provides,
                                         .data   = old_values};
  ruuvi_driver_sensor_data_t new_data = {.fields = DUT->provides,
                                         .data   = new_values};
  err_code = DUT->mode_set(&mode);
  err_code |= DUT->data_get(&old_data);
  ruuvi_interface_delay_ms(2); // wait 2 ms to ensure timestamp is not changed.
  err_code |= DUT->data_get(&new_data);

  if(RUUVI_DRIVER_SUCCESS != err_code ||
     old_data.timestamp_ms != new_data.timestamp_ms ||
     0 != memcmp(old_values, new_values, sizeof(old_values)))
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool sensor_remains_continuous(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t mode = RUUVI_DRIVER_SENSOR_CFG_SINGLE;
  mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
  err_code = DUT->mode_set(&mode);
  ruuvi_interface_delay_ms(20);
  err_code |= DUT->mode_get(&mode);

  if(RUUVI_DRIVER_SUCCESS != err_code || RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS != mode)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool sensor_rejects_single_on_continuous(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t mode = RUUVI_DRIVER_SENSOR_CFG_SINGLE;
  err_code = DUT->mode_set(&mode);

  if(RUUVI_DRIVER_ERROR_INVALID_STATE != err_code
      || RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS != mode)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return true;
}

static bool sensor_mode_cannot_be_null(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code = DUT->mode_set(NULL);
  if(RUUVI_DRIVER_ERROR_NULL != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }

  err_code = DUT->mode_get(NULL);
  if(RUUVI_DRIVER_ERROR_NULL != err_code)
  {
    RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
    return false;
  }
  return  true;
}

static bool sensor_returns_continuous_data(const ruuvi_driver_sensor_t* const DUT)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
  float old_values[32];
  float new_values[32];
  ruuvi_driver_sensor_data_t old_data = {.fields = DUT->provides,
                                         .data   = old_values};
  ruuvi_driver_sensor_data_t new_data = {.fields = DUT->provides,
                                         .data   = new_values};
  err_code = DUT->mode_set(&mode);
  err_code |= DUT->data_get(&old_data);
  ruuvi_interface_delay_ms(2); // wait 2 ms to ensure timestamp is not changed.
  err_code |= DUT->data_get(&new_data);
  err_code = DUT->mode_set(&mode);
  uint8_t samplerate = RUUVI_DRIVER_SENSOR_CFG_MAX;
  err_code |= DUT->samplerate_set(&samplerate);
  mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
  err_code |= DUT->mode_set(&mode);
  uint32_t interval = (1000 / (samplerate + 1));
  ruuvi_interface_delay_ms(2 * interval);
  err_code |= DUT->data_get(&old_data);
  int retries = 0;

  for(; retries < MAX_RETRIES; retries++)
  {
    ruuvi_interface_delay_ms(2 * interval);
    err_code |= DUT->data_get(&new_data);

    if(old_data.timestamp_ms == new_data.timestamp_ms || RUUVI_DRIVER_SUCCESS != err_code)
    {
      if(RUUVI_DRIVER_STATUS_MORE_AVAILABLE == err_code)
      {
        do
        {
          err_code |= DUT->data_get(&new_data);
        } while(RUUVI_DRIVER_STATUS_MORE_AVAILABLE == err_code);

        continue;
      }
      else
      {
        RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL);
        return false;
      }
    }

    if(value_has_changed(&old_data, &new_data)) { break; }
  }

  if(MAX_RETRIES == retries)
  {
    return false;
  }
  return true;
}

ruuvi_driver_status_t test_sensor_modes(const ruuvi_driver_sensor_init_fp init,
                                        const ruuvi_driver_bus_t bus, const uint8_t handle)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  ruuvi_driver_sensor_t DUT;
  memset(&DUT, 0, sizeof(DUT));
  initialize_sensor_once(&DUT, init, bus, handle);
  // - Sensor must be in SLEEP mode after init
  err_code = test_sensor_register(sensor_sleeps_after_init(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // - Sensor must return all values as INVALID if sensor is read before first sample
  err_code = test_sensor_register(sensor_returns_invalid_before_sampling(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // - Sensor must be in SLEEP mode after mode has been set to SINGLE
  err_code = test_sensor_register(sensor_returns_to_sleep(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // - Sensor must have new data after setting mode to SINGLE returns
  err_code = test_sensor_register(sensor_returns_valid_data(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // - Sensor must have same values, including timestamp, on successive calls to DATA_GET after SINGLE sample
  err_code = test_sensor_register(single_sample_stays_valid(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // - Sensor must stay in CONTINUOUS mode after being set to continuous.
  err_code = test_sensor_register(sensor_remains_continuous(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // - Sensor must return RUUVI_DRIVER_ERROR_INVALID_STATE if set to SINGLE while in continuous mode and remain in continuous mode
  err_code = test_sensor_register(sensor_rejects_single_on_continuous(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // - Sensor must return RUUVI_DRIVER_ERROR_NULL if null mode is passed as a parameter
  err_code = test_sensor_register(sensor_mode_cannot_be_null(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // Sensor must return updated data in CONTINUOUS mode, at least timestamp has to be updated after two ms wait.
  err_code = test_sensor_register(sensor_returns_continuous_data(&DUT));
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  // Uninitialise sensor after test
  err_code = DUT.uninit(&DUT, bus, handle);
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);

  return err_code;
}

static void on_fifo(const ruuvi_interface_gpio_evt_t evt)
{
  fifo_int = true;
}

static void on_level(const ruuvi_interface_gpio_evt_t evt)
{
  level_int = true;
}

/** @brief prepare GPIOs and initialize sensor for tests
 *
 *  @param[out] DUT Sensor to configure
 *  @param[in]  init function to initialize sensor
 *  @param[out] interrupt_table Table of function pointers to configure with interrupts
 *  @param[in]  fifo_pin Pin to register FIFO interrupts
 *  @param[in]  levelo_pin Pin to register level interrupts
 *  @return RUUVI_DRIVER_SUCCESS on successful initialization.
 *  @return RUUVI_DRIVER_ERROR_SELFTEST if initialization fails.
 */
static ruuvi_driver_status_t test_sensor_interrupts_setup(ruuvi_driver_sensor_t* DUT,
    ruuvi_driver_sensor_init_fp const init,
    const ruuvi_driver_bus_t bus, const uint8_t handle,
    ruuvi_interface_gpio_interrupt_fp_t* const interrupt_table,
    const ruuvi_interface_gpio_id_t fifo_pin,
    const ruuvi_interface_gpio_id_t level_pin)
{
  ruuvi_interface_gpio_interrupt_init(interrupt_table,
                                      RUUVI_INTERFACE_GPIO_INTERRUPT_TEST_TABLE_SIZE);
  ruuvi_interface_gpio_interrupt_enable(fifo_pin, RUUVI_INTERFACE_GPIO_SLOPE_LOTOHI,
                                        RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP, on_fifo);
  ruuvi_interface_gpio_interrupt_enable(level_pin, RUUVI_INTERFACE_GPIO_SLOPE_LOTOHI,
                                        RUUVI_INTERFACE_GPIO_MODE_INPUT_PULLUP, on_level);
  // - Sensor must return RUUVI_DRIVER_SUCCESS on first init.
  memset(DUT, 0, sizeof(ruuvi_driver_sensor_t));
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code = init(DUT, bus, handle);
  RUUVI_DRIVER_ERROR_CHECK(err_code, ~RUUVI_DRIVER_ERROR_FATAL);
  return  err_code;
}

/** @brief Uninitialize GPIOs and sensor after tests
  *  @param[out] DUT Sensor to configure
  */
static void test_sensor_interrupts_teardown(ruuvi_driver_sensor_t* const DUT,
    ruuvi_driver_sensor_init_fp const init,
    const ruuvi_driver_bus_t bus, const uint8_t handle)
{
  DUT->uninit(DUT, bus, handle);
  ruuvi_interface_gpio_interrupt_uninit();
}

/* @brief  - FIFO read must return samples with different values (noise) */
static ruuvi_driver_status_t test_sensor_fifo_enable(const ruuvi_driver_sensor_t* DUT)
{
  DUT->fifo_enable(true);
  ruuvi_driver_sensor_configuration_t config = {0};
  config.samplerate = RUUVI_DRIVER_SENSOR_CFG_MAX;
  config.mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
  DUT->configuration_set(DUT, &config);
  ruuvi_interface_delay_ms(100);
  size_t num_samples = 32;
  ruuvi_driver_sensor_data_t data[32] = { 0 };
  bool valid_data = false;
  DUT->fifo_read(&num_samples, data);

  if(10 > num_samples)
  {
    return RUUVI_DRIVER_ERROR_SELFTEST;
  }

  // Initialize has changed with data
  ruuvi_driver_sensor_data_t old;
  value_has_changed(&old, &(data[0]));

  for(size_t iii = 1; iii < num_samples; iii++)
  {
    if(value_has_changed(&old,  &(data[iii])))
    {
      valid_data = true;
      break;
    }
  }

  test_sensor_register(valid_data);
  return (valid_data) ? RUUVI_DRIVER_SUCCESS : RUUVI_DRIVER_ERROR_SELFTEST;
}


/**
 * Functions may return @c RUUVI_DRIVER_ERROR_NOT_SUPPORTED, otherwise:
 *  - FIFO read must return samples with different values (noise)
 *  - FIFO full interrupt must trigger after some time when in FIFO mode
 *  - FIFO full interrupt must trigger again after FIFO has been read and filled again
 *  - FIFO full interrupt must not trigger if FIFO is read at fast enough interval
 *  - FIFO full interrupt must not
 */
ruuvi_driver_status_t test_sensor_interrupts(const ruuvi_driver_sensor_init_fp init,
    const ruuvi_driver_bus_t bus, const uint8_t handle,
    const bool interactive,
    const ruuvi_interface_gpio_id_t fifo_pin,
    const ruuvi_interface_gpio_id_t level_pin)
{
  ruuvi_interface_gpio_interrupt_fp_t
  interrupt_table[RUUVI_INTERFACE_GPIO_INTERRUPT_TEST_TABLE_SIZE];
  ruuvi_driver_sensor_t DUT;
  ruuvi_driver_status_t status;
  status = test_sensor_interrupts_setup(&DUT, init, bus, handle, interrupt_table, fifo_pin,
                                        level_pin);

  if(RUUVI_DRIVER_SUCCESS == status)
  {
    status |= test_sensor_fifo_enable(&DUT);
  }

  test_sensor_interrupts_teardown(&DUT, init, bus, handle);
  return status;
}

ruuvi_driver_status_t test_sensor_status(size_t* tests_total, size_t* tests_passed)
{
  *tests_total =  m_total;
  *tests_passed = m_passed;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t test_sensor_register(bool passed)
{
  m_total++;

  if(passed) { m_passed++; }

  return passed ? RUUVI_DRIVER_SUCCESS : RUUVI_DRIVER_ERROR_SELFTEST;
}

#else
// Dummy implementation
ruuvi_driver_status_t test_sensor_status(size_t* total, size_t* passed)
{
  *total = 0;
  *passed = 0;
  return RUUVI_DRIVER_SUCCESS;
}
void test_sensor_run(void)
{}
#endif