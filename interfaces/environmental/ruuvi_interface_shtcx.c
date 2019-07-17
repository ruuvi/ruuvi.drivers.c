/**
 * SHTCX interface.
 * Requires Sensirion SHTCx, available under BSD-3 on GitHub.
 * Will only get compiled if RUUVI_INTERFACE_ENVIRONMENTAL_BME280_ENABLED is defined as true
 * Requires BME280_FLOAT_ENABLE defined in makefile.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ENVIRONMENTAL_SHTCX_ENABLED || DOXYGEN
// Ruuvi headers
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_shtcx.h"
#include "ruuvi_interface_environmental.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_shtcx.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_yield.h"

#include <string.h>

// Sensirion driver.
#include "shtc1.h"

// Macro for checking "ignored" parameters NO_CHANGE, MIN, MAX, DEFAULT
#define RETURN_SUCCESS_ON_VALID(param) do {\
            if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT   == param ||\
               RUUVI_DRIVER_SENSOR_CFG_MIN       == param ||\
               RUUVI_DRIVER_SENSOR_CFG_MAX       == param ||\
               RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == param   \
             ) return RUUVI_DRIVER_SUCCESS;\
           } while(0)

// Macro for checking that sensor is in sleep mode before configuration
#define VERIFY_SENSOR_SLEEPS() do { \
          uint8_t MACRO_MODE = 0; \
          ruuvi_interface_shtcx_mode_get(&MACRO_MODE); \
          if(RUUVI_DRIVER_SENSOR_CFG_SLEEP != MACRO_MODE) { return RUUVI_DRIVER_ERROR_INVALID_STATE; } \
          } while(0)


/** State variables **/
static uint64_t m_tsample;
static bool m_autorefresh;
static int32_t m_temperature;
static int32_t m_humidity;
static bool m_is_init;

#define STATUS_OK 0
#define STATUS_ERR_BAD_DATA (-1)
#define STATUS_CRC_FAIL (-2)
#define STATUS_UNKNOWN_DEVICE (-3)
#define STATUS_WAKEUP_FAILED (-4)
#define STATUS_SLEEP_FAILED (-5)

/**
 * Convert error from SHTCX driver to appropriate NRF ERROR
 */
static ruuvi_driver_status_t SHTCX_TO_RUUVI_ERROR(int16_t rslt)
{
  if(STATUS_OK == rslt)                 { return RUUVI_DRIVER_SUCCESS; }
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_ERROR_INTERNAL;

  if(STATUS_UNKNOWN_DEVICE == rslt)     { err_code = RUUVI_DRIVER_ERROR_NOT_FOUND; }
  else if(STATUS_ERR_BAD_DATA == rslt)  { err_code = RUUVI_DRIVER_ERROR_INVALID_DATA; }
  else if(STATUS_CRC_FAIL == rslt)      { err_code = RUUVI_DRIVER_ERROR_INVALID_DATA; }
  else if(STATUS_WAKEUP_FAILED == rslt) { err_code = RUUVI_DRIVER_ERROR_INTERNAL; }
  else if(STATUS_SLEEP_FAILED == rslt)  { err_code = RUUVI_DRIVER_ERROR_INTERNAL; }

  return err_code;
}

/** Initialize SHTCx into low-power mode **/
ruuvi_driver_status_t ruuvi_interface_shtcx_init(ruuvi_driver_sensor_t*
    environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == environmental_sensor) { return RUUVI_DRIVER_ERROR_NULL; }
  if(m_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  size_t retries = 0;

  switch(bus)
  {
    case RUUVI_DRIVER_BUS_I2C:
      do{
      err_code = SHTCX_TO_RUUVI_ERROR(shtc1_probe());
      retries++;
      }while(RUUVI_DRIVER_ERROR_INVALID_DATA == err_code && retries < 5);
      break;

    default:
      return  RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }
  if(RUUVI_DRIVER_SUCCESS != err_code) { err_code = RUUVI_DRIVER_ERROR_NOT_FOUND; }

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    // Sensirion driver delays high-power mode time in any case.
    shtc1_enable_low_power_mode(0);
    environmental_sensor->init              = ruuvi_interface_shtcx_init;
    environmental_sensor->uninit            = ruuvi_interface_shtcx_uninit;
    environmental_sensor->samplerate_set    = ruuvi_interface_shtcx_samplerate_set;
    environmental_sensor->samplerate_get    = ruuvi_interface_shtcx_samplerate_get;
    environmental_sensor->resolution_set    = ruuvi_interface_shtcx_resolution_set;
    environmental_sensor->resolution_get    = ruuvi_interface_shtcx_resolution_get;
    environmental_sensor->scale_set         = ruuvi_interface_shtcx_scale_set;
    environmental_sensor->scale_get         = ruuvi_interface_shtcx_scale_get;
    environmental_sensor->dsp_set           = ruuvi_interface_shtcx_dsp_set;
    environmental_sensor->dsp_get           = ruuvi_interface_shtcx_dsp_get;
    environmental_sensor->mode_set          = ruuvi_interface_shtcx_mode_set;
    environmental_sensor->mode_get          = ruuvi_interface_shtcx_mode_get;
    environmental_sensor->data_get          = ruuvi_interface_shtcx_data_get;
    environmental_sensor->configuration_set = ruuvi_driver_sensor_configuration_set;
    environmental_sensor->configuration_get = ruuvi_driver_sensor_configuration_get;
    m_tsample = RUUVI_DRIVER_UINT64_INVALID;
    m_is_init = true;
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_uninit(ruuvi_driver_sensor_t* sensor,
    ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  shtc1_enable_low_power_mode(1);

  memset(sensor, 0, sizeof(ruuvi_driver_sensor_t));
  m_tsample = RUUVI_DRIVER_UINT64_INVALID;
  m_temperature = RUUVI_DRIVER_INT32_INVALID;
  m_humidity = RUUVI_DRIVER_INT32_INVALID;
  m_is_init = false;
  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_samplerate_set(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }
  VERIFY_SENSOR_SLEEPS();
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *samplerate)  { *samplerate = RUUVI_DRIVER_SENSOR_CFG_DEFAULT; }
  else if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *samplerate) { *samplerate = RUUVI_DRIVER_SENSOR_CFG_DEFAULT; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MIN == *samplerate) { *samplerate = RUUVI_DRIVER_SENSOR_CFG_DEFAULT; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *samplerate) {*samplerate = RUUVI_DRIVER_SENSOR_CFG_DEFAULT; }
  else { *samplerate = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED; err_code |= RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_samplerate_get(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  *samplerate = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_resolution_set(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }
  
  VERIFY_SENSOR_SLEEPS();
  uint8_t original = *resolution;
  *resolution = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  RETURN_SUCCESS_ON_VALID(original);
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_resolution_get(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

  *resolution = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_scale_set(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }
  VERIFY_SENSOR_SLEEPS();
  uint8_t original = *scale;
  *scale = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  RETURN_SUCCESS_ON_VALID(original);
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_scale_get(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

  *scale = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_dsp_set(uint8_t* dsp, uint8_t* parameter)
{
  if(NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }
  VERIFY_SENSOR_SLEEPS();
  // Validate configuration
  if((RUUVI_DRIVER_SENSOR_CFG_DEFAULT  != *parameter
      && RUUVI_DRIVER_SENSOR_CFG_MIN   != *parameter
      && RUUVI_DRIVER_SENSOR_CFG_MAX   != *parameter) || 
      (RUUVI_DRIVER_SENSOR_DSP_LAST  != *dsp))
  {
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }

  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_dsp_get(uint8_t* dsp, uint8_t* parameter)
{
  if(NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }

  // Only default is available
  *dsp       = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  *parameter = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;

  return RUUVI_DRIVER_SUCCESS;
}

// Start single on command, mark autorefresh with continuous
ruuvi_driver_status_t ruuvi_interface_shtcx_mode_set(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  // Enter sleep by default and by explicit sleep commmand
  if(RUUVI_DRIVER_SENSOR_CFG_SLEEP == *mode || RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *mode)
  {
    m_autorefresh = false;
    *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
    return RUUVI_DRIVER_SUCCESS;
  }

  if(RUUVI_DRIVER_SENSOR_CFG_SINGLE == *mode)
  {
    // Do nothing if sensor is in continuous mode
    uint8_t current_mode;
    ruuvi_interface_shtcx_mode_get(&current_mode);

    if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == current_mode)
    {
      *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
      // Start first measurement
      shtc1_measure();
      return RUUVI_DRIVER_ERROR_INVALID_STATE;
    }

    // Enter sleep after measurement
    m_autorefresh = false;
    *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
    m_tsample = ruuvi_interface_rtc_millis();

    return SHTCX_TO_RUUVI_ERROR(shtc1_measure_blocking_read(&m_temperature, &m_humidity));
    
  }

  if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == *mode)
  {
    m_autorefresh = true;
    return RUUVI_DRIVER_SUCCESS;
  }

  return RUUVI_DRIVER_ERROR_INVALID_PARAM;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_mode_get(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  if(m_autorefresh)
  {
    *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
  }

  if(!m_autorefresh)
  {
    *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
  }

  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_shtcx_data_get(void* data)
{
  if(NULL == data) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_interface_environmental_data_t* environmental =
    (ruuvi_interface_environmental_data_t*) data;
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  if(m_autorefresh) 
  { 
    /* XXX Sensor sleep clears measured values, blocking read required.
    // read sensor values
    err_code |= SHTCX_TO_RUUVI_ERROR(shtc1_read(&m_temperature, &m_humidity));
    // Start next measurement
    err_code |= SHTCX_TO_RUUVI_ERROR(shtc1_measure()); 
    */
    err_code |= SHTCX_TO_RUUVI_ERROR(shtc1_measure_blocking_read(&m_temperature, &m_humidity));
    m_tsample = ruuvi_interface_rtc_millis();
  }

  environmental->timestamp_ms  = RUUVI_DRIVER_UINT64_INVALID;
  environmental->temperature_c = RUUVI_INTERFACE_ENVIRONMENTAL_INVALID;
  environmental->pressure_pa   = RUUVI_INTERFACE_ENVIRONMENTAL_INVALID;
  environmental->humidity_rh   = RUUVI_INTERFACE_ENVIRONMENTAL_INVALID;

  if(RUUVI_DRIVER_SUCCESS == err_code && RUUVI_DRIVER_UINT64_INVALID != m_tsample)
  {
    environmental->timestamp_ms  = m_tsample;
    environmental->temperature_c = m_temperature/1000.0f;
    environmental->humidity_rh   = m_humidity/1000.0f;
  }

  return err_code;
}





#endif