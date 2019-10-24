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

/**
 * @defgroup Environmental Environmental sensing
 * @brief Interface and implementations for different, temperature, humidity and 
 *        barometric pressure sensors. 
 */
/*@{*/

/**
 * @file ruuvi_interface_shtcx.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-08-10
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Environmental dataformat definition
 *
 */

/** @brief Macro for checking "ignored" parameters NO_CHANGE, MIN, MAX, DEFAULT */
#define RETURN_SUCCESS_ON_VALID(param) do {\
            if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT   == param ||\
               RUUVI_DRIVER_SENSOR_CFG_MIN       == param ||\
               RUUVI_DRIVER_SENSOR_CFG_MAX       == param ||\
               RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == param   \
             ) return RUUVI_DRIVER_SUCCESS;\
           } while(0)

/** @brief Macro for checking that sensor is in sleep mode before configuration */
#define VERIFY_SENSOR_SLEEPS() do { \
          uint8_t MACRO_MODE = 0; \
          ruuvi_interface_shtcx_mode_get(&MACRO_MODE); \
          if(RUUVI_DRIVER_SENSOR_CFG_SLEEP != MACRO_MODE) { return RUUVI_DRIVER_ERROR_INVALID_STATE; } \
          } while(0)



static uint64_t m_tsample;           //!< Timestamp of sample.
static bool m_autorefresh;           //!< Flag to refresh data on data_get.
static int32_t m_temperature;        //!< Last measured temperature.
static int32_t m_humidity;           //!< Last measured humidity.
static bool m_is_init;               //!< Flag, is sensor init.
static const char m_sensor_name[] = "SHTCX"; //!< Human-readable name of the sensor. 

#define STATUS_OK 0                  //!< SHTC driver ok
#define STATUS_ERR_BAD_DATA (-1)     //!< SHTC driver data invald
#define STATUS_CRC_FAIL (-2)         //!< SHTC driver CRC error
#define STATUS_UNKNOWN_DEVICE (-3)   //!< Invalid WHOAMI
#define STATUS_WAKEUP_FAILED (-4)    //!< Device didn't wake up
#define STATUS_SLEEP_FAILED (-5)     //!< Device didn't go to sleep

/**
 * @brief Convert error from SHTCX driver to appropriate NRF ERROR
 *
 * @param[in] rslt error code from SHTCX driver
 * @return    Ruuvi error code corresponding to SHTCX error code
 */
static ruuvi_driver_status_t SHTCX_TO_RUUVI_ERROR(const int16_t rslt)
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

ruuvi_driver_status_t ruuvi_interface_shtcx_init(ruuvi_driver_sensor_t*
    environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == environmental_sensor) { return RUUVI_DRIVER_ERROR_NULL; }
  if(m_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ruuvi_driver_sensor_initialize(environmental_sensor);
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
    // Explicitly entering low-power mode has no effect.
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
    environmental_sensor->name              = m_sensor_name;
    environmental_sensor->provides.datas.temperature_c = 1;
    environmental_sensor->provides.datas.humidity_rh = 1;
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

  ruuvi_driver_sensor_uninitialize(sensor);
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
    m_tsample = ruuvi_driver_sensor_timestamp_get();

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

ruuvi_driver_status_t ruuvi_interface_shtcx_data_get(ruuvi_driver_sensor_data_t* const p_data)
{
  if(NULL == p_data) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  if(m_autorefresh) 
  { 
    /* Sensor sleep clears measured values, blocking read required.
    // read sensor values
    err_code |= SHTCX_TO_RUUVI_ERROR(shtc1_read(&m_temperature, &m_humidity));
    // Start next measurement
    err_code |= SHTCX_TO_RUUVI_ERROR(shtc1_measure()); 
    */
    err_code |= SHTCX_TO_RUUVI_ERROR(shtc1_measure_blocking_read(&m_temperature, &m_humidity));
    m_tsample = ruuvi_driver_sensor_timestamp_get();
  }

  if(RUUVI_DRIVER_SUCCESS == err_code && RUUVI_DRIVER_UINT64_INVALID != m_tsample)
  {
    ruuvi_driver_sensor_data_t d_environmental;
    ruuvi_driver_sensor_data_fields_t env_fields = {.bitfield = 0};
    float env_values[2];
    env_values[0] = m_humidity/1000.0f;
    env_values[1] = m_temperature/1000.0f;
    env_fields.datas.humidity_rh = 1;
    env_fields.datas.temperature_c = 1;
    d_environmental.data = env_values;
    d_environmental.valid  = env_fields;
    d_environmental.fields = env_fields;
    ruuvi_driver_sensor_data_populate(p_data,
                                      &d_environmental,
                                      p_data->fields);
    p_data->timestamp_ms = m_tsample;
  }

  return err_code;
}

/**
 * @brief Implement sleep function for SHTC driver. 
 *
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * If delay is at least millisecond,
 * The function sleeps given number of milliseconds, rounded up, 
 * to benefit from low-power sleep in millisecond delay.
 *
 * @param[in] useconds the sleep time in microseconds
 * @note      sensirion interface signature isn't const, can't be const here.
 */
void sensirion_sleep_usec(uint32_t useconds) {
  if(useconds < 1000)
  {
    ruuvi_interface_delay_us(useconds);
  }
  else
  {
    ruuvi_interface_delay_ms((useconds/1000) + 1);
  }
}



#endif