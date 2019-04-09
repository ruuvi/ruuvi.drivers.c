/**
 * BME280 interface.
 * Requires Bosch BME280_driver, available under BSD-3 on GitHub.
 * Will only get compiled if RUUVI_INTERFACE_ENVIRONMENTAL_BME280_ENABLED is defined as true
 * Requires BME280_FLOAT_ENABLE defined in makefile.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ENVIRONMENTAL_BME280_ENABLED
// Ruuvi headers
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_bme280.h"
#include "ruuvi_interface_environmental.h"
#include "ruuvi_interface_spi.h"
#include "ruuvi_interface_spi_bme280.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_i2c_bme280.h"
#include "ruuvi_interface_yield.h"

#include <string.h>

// Bosch driver.
#include "bme280.h"
#include "bme280_defs.h"
#include "bme280_selftest.h"
#if !(BME280_FLOAT_ENABLE || DOXYGEN)
  #error "Please #define BME280_FLOAT_ENABLE in makefile CFLAGS"
#endif

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
          ruuvi_interface_bme280_mode_get(&MACRO_MODE); \
          if(RUUVI_DRIVER_SENSOR_CFG_SLEEP != MACRO_MODE) { return RUUVI_DRIVER_ERROR_INVALID_STATE; } \
          } while(0)


/** State variables **/
static struct bme280_dev dev = {0};
static uint64_t tsample;

/**
 * Convert error from BME280 driver to appropriate NRF ERROR
 */
static ruuvi_driver_status_t BME_TO_RUUVI_ERROR(int8_t rslt)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(BME280_E_DEV_NOT_FOUND == rslt)  { err_code = RUUVI_DRIVER_ERROR_NOT_FOUND; }
  else if(BME280_E_NULL_PTR == rslt)  { err_code = RUUVI_DRIVER_ERROR_NULL; }
  else if(BME280_E_COMM_FAIL == rslt) { err_code = RUUVI_DRIVER_ERROR_BUSY; }
  else if(BME280_OK == rslt) { return RUUVI_DRIVER_SUCCESS; }

  return err_code;
}

void bosch_delay_ms(uint32_t time_ms)
{
  ruuvi_interface_delay_ms(time_ms);
}

// BME280 datasheet Appendix B.
static uint32_t bme280_max_meas_time(uint8_t oversampling)
{
  // Time
  float time = 1.25f + \
               2.3 * 3 * oversampling + \
               2 * 0.575;
  // Roundoff + margin
  return (uint32_t)(2 + (uint32_t) time);
}

/** Initialize BME280 into low-power mode **/
ruuvi_driver_status_t ruuvi_interface_bme280_init(ruuvi_driver_sensor_t*
    environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == environmental_sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  // dev is NULL at boot, if function pointers have been set sensor is initialized
  if(NULL != dev.write) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  switch(bus)
  {
    #if RUUVI_INTERFACE_ENVIRONMENTAL_BME280_SPI_ENABLED
    case RUUVI_DRIVER_BUS_SPI:
      /* Sensor_0 interface over SPI with native chip select line */
      dev.dev_id = handle;
      dev.intf = BME280_SPI_INTF;
      dev.read = ruuvi_interface_spi_bme280_read;
      dev.write = ruuvi_interface_spi_bme280_write;
      dev.delay_ms = bosch_delay_ms;
      err_code |= BME_TO_RUUVI_ERROR(bme280_init(&dev));

      if(err_code != RUUVI_DRIVER_SUCCESS) { return err_code; }
      break;
    #endif

    #if RUUVI_INTERFACE_ENVIRONMENTAL_BME280_I2C_ENABLED
    case RUUVI_DRIVER_BUS_I2C:
      dev.dev_id = handle;
      dev.intf = BME280_I2C_INTF;
      dev.read = ruuvi_interface_i2c_bme280_read;
      dev.write = ruuvi_interface_i2c_bme280_write;
      dev.delay_ms = bosch_delay_ms;
      err_code |= BME_TO_RUUVI_ERROR(bme280_init(&dev));

      if(err_code != RUUVI_DRIVER_SUCCESS) { return err_code; }
      break;
    #endif

    case RUUVI_DRIVER_BUS_NONE:
    default:
      return  RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  err_code |= BME_TO_RUUVI_ERROR(bme280_crc_selftest(&dev));
  err_code |= BME_TO_RUUVI_ERROR(bme280_soft_reset(&dev));
  // Setup Oversampling 1 to enable sensor
  uint8_t dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
  uint8_t dsp_parameter = 1;
  err_code |= ruuvi_interface_bme280_dsp_set(&dsp, &dsp_parameter);

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    environmental_sensor->init              = ruuvi_interface_bme280_init;
    environmental_sensor->uninit            = ruuvi_interface_bme280_uninit;
    environmental_sensor->samplerate_set    = ruuvi_interface_bme280_samplerate_set;
    environmental_sensor->samplerate_get    = ruuvi_interface_bme280_samplerate_get;
    environmental_sensor->resolution_set    = ruuvi_interface_bme280_resolution_set;
    environmental_sensor->resolution_get    = ruuvi_interface_bme280_resolution_get;
    environmental_sensor->scale_set         = ruuvi_interface_bme280_scale_set;
    environmental_sensor->scale_get         = ruuvi_interface_bme280_scale_get;
    environmental_sensor->dsp_set           = ruuvi_interface_bme280_dsp_set;
    environmental_sensor->dsp_get           = ruuvi_interface_bme280_dsp_get;
    environmental_sensor->mode_set          = ruuvi_interface_bme280_mode_set;
    environmental_sensor->mode_get          = ruuvi_interface_bme280_mode_get;
    environmental_sensor->data_get          = ruuvi_interface_bme280_data_get;
    environmental_sensor->configuration_set = ruuvi_driver_sensor_configuration_set;
    environmental_sensor->configuration_get = ruuvi_driver_sensor_configuration_get;
    tsample = RUUVI_DRIVER_UINT64_INVALID;
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_bme280_uninit(ruuvi_driver_sensor_t* sensor,
    ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = BME_TO_RUUVI_ERROR(bme280_soft_reset(&dev));

  if(RUUVI_DRIVER_SUCCESS != err_code) { return err_code; }

  memset(sensor, 0, sizeof(ruuvi_driver_sensor_t));
  memset(&dev, 0, sizeof(dev));
  tsample = RUUVI_DRIVER_UINT64_INVALID;
  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_bme280_samplerate_set(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *samplerate)  { dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
  else if(*samplerate == 1)                           { dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
  else if(*samplerate == 2)                           { dev.settings.standby_time = BME280_STANDBY_TIME_500_MS; }
  else if(*samplerate <= 8)                           { dev.settings.standby_time = BME280_STANDBY_TIME_125_MS; }
  else if(*samplerate <= 16)                          { dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS; }
  else if(*samplerate <= 50)                          { dev.settings.standby_time = BME280_STANDBY_TIME_20_MS; }
  else if(*samplerate <= 100)                         { dev.settings.standby_time = BME280_STANDBY_TIME_10_MS; }
  else if(*samplerate <= 200)                         { dev.settings.standby_time = BME280_STANDBY_TIME_1_MS; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MIN == *samplerate) { dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *samplerate) { dev.settings.standby_time = BME280_STANDBY_TIME_1_MS; }
  else if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *samplerate) {} // do nothing
  else { *samplerate = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED; err_code |= RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    // BME 280 must be in standby while configured
    err_code |=  BME_TO_RUUVI_ERROR(bme280_set_sensor_settings(BME280_STANDBY_SEL, &dev));
    err_code |= ruuvi_interface_bme280_samplerate_get(samplerate);
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_bme280_samplerate_get(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = BME_TO_RUUVI_ERROR(bme280_get_sensor_settings(&dev));

  if(RUUVI_DRIVER_SUCCESS != err_code) { return err_code; }

  if(BME280_STANDBY_TIME_1000_MS == dev.settings.standby_time)      { *samplerate = 1;   }
  else if(BME280_STANDBY_TIME_500_MS == dev.settings.standby_time)  { *samplerate = 2;   }
  else if(BME280_STANDBY_TIME_125_MS == dev.settings.standby_time)  { *samplerate = 8;   }
  else if(BME280_STANDBY_TIME_62_5_MS == dev.settings.standby_time) { *samplerate = 16;  }
  else if(BME280_STANDBY_TIME_20_MS == dev.settings.standby_time)   { *samplerate = 50;  }
  else if(BME280_STANDBY_TIME_10_MS == dev.settings.standby_time)   { *samplerate = 100;  }
  else if(BME280_STANDBY_TIME_1_MS == dev.settings.standby_time)    { *samplerate = 200; }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_bme280_resolution_set(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  uint8_t original = *resolution;
  *resolution = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  RETURN_SUCCESS_ON_VALID(original);
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_bme280_resolution_get(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

  *resolution = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_bme280_scale_set(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  uint8_t original = *scale;
  *scale = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  RETURN_SUCCESS_ON_VALID(original);
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_bme280_scale_get(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

  *scale = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_bme280_dsp_set(uint8_t* dsp, uint8_t* parameter)
{
  if(NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();

  // Validate configuration
  if(1  != *parameter
      && 2  != *parameter
      && 4  != *parameter
      && 8  != *parameter
      && 16 != *parameter
      && RUUVI_DRIVER_SENSOR_CFG_DEFAULT != *parameter
      && RUUVI_DRIVER_SENSOR_CFG_MIN     != *parameter
      && RUUVI_DRIVER_SENSOR_CFG_MAX     != *parameter
      && RUUVI_DRIVER_SENSOR_DSP_LAST != *dsp)
  {
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }

  // Error if DSP is not last, and if dsp is something else than IIR or OS
  if((RUUVI_DRIVER_SENSOR_DSP_LAST != *dsp) &&
      ~(RUUVI_DRIVER_SENSOR_DSP_LOW_PASS | RUUVI_DRIVER_SENSOR_DSP_OS) & (*dsp))
  {
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }

  // Clear setup
  uint8_t settings_sel = 0;
  // Always 1x oversampling to keep sensing element enabled
  dev.settings.osr_h = BME280_OVERSAMPLING_1X;
  dev.settings.osr_p = BME280_OVERSAMPLING_1X;
  dev.settings.osr_t = BME280_OVERSAMPLING_1X;
  dev.settings.filter = BME280_FILTER_COEFF_OFF;
  settings_sel |= BME280_OSR_PRESS_SEL;
  settings_sel |= BME280_OSR_TEMP_SEL;
  settings_sel |= BME280_OSR_HUM_SEL;
  settings_sel |= BME280_FILTER_SEL;

  // Setup IIR
  if(RUUVI_DRIVER_SENSOR_DSP_LOW_PASS & *dsp)
  {
    if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *parameter || \
        RUUVI_DRIVER_SENSOR_CFG_MIN     == *parameter || \
        1 == *parameter
      )
    {
      dev.settings.filter = BME280_FILTER_COEFF_OFF;
      *parameter = 1;
    }
    else if(2 == *parameter)
    {
      dev.settings.filter = BME280_FILTER_COEFF_2;
      *parameter = 2;
    }
    else if(4 >= *parameter)
    {
      dev.settings.filter = BME280_FILTER_COEFF_4;
      *parameter = 4;
    }
    else if(8 >= *parameter)
    {
      dev.settings.filter = BME280_FILTER_COEFF_8;
      *parameter = 8;
    }
    else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *parameter || \
            16 >= *parameter)
    {
      dev.settings.filter = BME280_FILTER_COEFF_16;
      *parameter = 16;
    }
    else
    {
      *parameter = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
      return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
    }
  }

  // Setup Oversampling
  if(RUUVI_DRIVER_SENSOR_DSP_OS & *dsp)
  {
    if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *parameter || \
        RUUVI_DRIVER_SENSOR_CFG_MIN     == *parameter || \
        1 == *parameter)
    {
      dev.settings.osr_h = BME280_OVERSAMPLING_1X;
      dev.settings.osr_p = BME280_OVERSAMPLING_1X;
      dev.settings.osr_t = BME280_OVERSAMPLING_1X;
      *parameter = 1;
    }
    else if(2 == *parameter)
    {
      dev.settings.osr_h = BME280_OVERSAMPLING_2X;
      dev.settings.osr_p = BME280_OVERSAMPLING_2X;
      dev.settings.osr_t = BME280_OVERSAMPLING_2X;
      *parameter = 2;
    }
    else if(4 >= *parameter)
    {
      dev.settings.osr_h = BME280_OVERSAMPLING_4X;
      dev.settings.osr_p = BME280_OVERSAMPLING_4X;
      dev.settings.osr_t = BME280_OVERSAMPLING_4X;
      *parameter = 4;
    }
    else if(8 >= *parameter)
    {
      dev.settings.osr_h = BME280_OVERSAMPLING_8X;
      dev.settings.osr_p = BME280_OVERSAMPLING_8X;
      dev.settings.osr_t = BME280_OVERSAMPLING_8X;
      *parameter = 8;
    }
    else if(16 >= *parameter || \
            RUUVI_DRIVER_SENSOR_CFG_MAX)
    {
      dev.settings.osr_h = BME280_OVERSAMPLING_16X;
      dev.settings.osr_p = BME280_OVERSAMPLING_16X;
      dev.settings.osr_t = BME280_OVERSAMPLING_16X;
      *parameter = 16;
    }
    else
    {
      *parameter = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
      return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
    }
  }

  //Write configuration
  return BME_TO_RUUVI_ERROR(bme280_set_sensor_settings(settings_sel, &dev));
}

// Read configuration
ruuvi_driver_status_t ruuvi_interface_bme280_dsp_get(uint8_t* dsp, uint8_t* parameter)
{
  if(NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= BME_TO_RUUVI_ERROR(bme280_get_sensor_settings(&dev));

  if(RUUVI_DRIVER_SUCCESS != err_code) { return err_code; }

  // Assume default / 0,
  *dsp       = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  *parameter = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;

  // Check if IIR has been set. If yes, read DSP param from there.
  if(BME280_FILTER_COEFF_OFF != dev.settings.filter)
  {
    *dsp |= RUUVI_DRIVER_SENSOR_DSP_LOW_PASS;

    switch(dev.settings.filter)
    {
      case BME280_FILTER_COEFF_2:
        *parameter = 2;
        break;

      case BME280_FILTER_COEFF_4:
        *parameter = 4;
        break;

      case BME280_FILTER_COEFF_8:
        *parameter = 8;
        break;

      case BME280_FILTER_COEFF_16:
        *parameter = 16;
        break;
    }
  }

  // Check if OS has been set. If yes, read DSP param from there.
  // Param should be same for OS and IIR if it is >1.
  // OSR is same for every element.
  if(BME280_NO_OVERSAMPLING != dev.settings.osr_h
      && BME280_OVERSAMPLING_1X != dev.settings.osr_h)
  {
    *dsp |= RUUVI_DRIVER_SENSOR_DSP_OS;

    switch(dev.settings.osr_h)
    {
      case BME280_OVERSAMPLING_2X:
        *parameter = 2;
        break;

      case BME280_OVERSAMPLING_4X:
        *parameter = 4;
        break;

      case BME280_OVERSAMPLING_8X:
        *parameter = 8;
        break;

      case BME280_OVERSAMPLING_16X:
        *parameter = 16;
        break;
    }
  }

  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_bme280_mode_set(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t current_mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;

  switch(*mode)
  {
    case RUUVI_DRIVER_SENSOR_CFG_SLEEP:
      err_code = BME_TO_RUUVI_ERROR(bme280_set_sensor_mode(BME280_SLEEP_MODE, &dev));
      break;

    case RUUVI_DRIVER_SENSOR_CFG_SINGLE:
      // Do nothing if sensor is in continuous mode
      ruuvi_interface_bme280_mode_get(&current_mode);

      if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == current_mode)
      {
        *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
        return RUUVI_DRIVER_ERROR_INVALID_STATE;
      }

      err_code = BME_TO_RUUVI_ERROR(bme280_set_sensor_mode(BME280_FORCED_MODE, &dev));
      // We assume that dev struct is in sync with the state of the BME280 and underlying interface
      // which has the number of settings as 2^OSR is not changed.
      // We also assume that each element runs same OSR
      uint8_t samples = 1 << (dev.settings.osr_h - 1);
      ruuvi_interface_delay_ms(bme280_max_meas_time(samples));
      tsample = ruuvi_driver_sensor_timestamp_get();
      // BME280 returns to SLEEP after forced sample
      *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
      break;

    case RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS:
      err_code = BME_TO_RUUVI_ERROR(bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev));
      break;

    default:
      err_code = RUUVI_DRIVER_ERROR_INVALID_PARAM;
      break;
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_bme280_mode_get(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t bme_mode = 0;
  err_code = BME_TO_RUUVI_ERROR(bme280_get_sensor_mode(&bme_mode, &dev));

  if(RUUVI_DRIVER_SUCCESS != err_code) { return err_code; }

  switch(bme_mode)
  {
    case BME280_SLEEP_MODE:
      *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
      break;

    case BME280_FORCED_MODE:
      *mode = RUUVI_DRIVER_SENSOR_CFG_SINGLE;
      break;

    case BME280_NORMAL_MODE:
      *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
      break;

    default:
      *mode = RUUVI_DRIVER_SENSOR_ERR_INVALID;
      break;
  }

  return err_code;
}


ruuvi_driver_status_t ruuvi_interface_bme280_data_get(void* data)
{
  if(NULL == data) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_interface_environmental_data_t* p_data = (ruuvi_interface_environmental_data_t*)
      data;
  struct bme280_data comp_data;
  p_data->timestamp_ms   = 0;
  p_data->temperature_c  = RUUVI_INTERFACE_ENVIRONMENTAL_INVALID;
  p_data->humidity_rh    = RUUVI_INTERFACE_ENVIRONMENTAL_INVALID;
  p_data->pressure_pa    = RUUVI_INTERFACE_ENVIRONMENTAL_INVALID;
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code = BME_TO_RUUVI_ERROR(bme280_get_sensor_data(BME280_ALL, &comp_data, &dev));

  if(RUUVI_DRIVER_SUCCESS != err_code) { return err_code; }

  // Write tsample if we're in single mode, current time if we're in continuous mode
  // Leave sample time as invalid if forced mode is ongoing.
  uint8_t mode = 0;
  err_code |= ruuvi_interface_bme280_mode_get(&mode);

  if(RUUVI_DRIVER_SENSOR_CFG_SLEEP == mode)           { p_data->timestamp_ms = tsample; }
  else if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == mode) { p_data->timestamp_ms = ruuvi_driver_sensor_timestamp_get(); }
  else { RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL); }

  // If we have valid data, return it.
  if(RUUVI_DRIVER_UINT64_INVALID != p_data->timestamp_ms)
  {
    p_data->temperature_c  = (float) comp_data.temperature;
    p_data->humidity_rh    = (float) comp_data.humidity;
    p_data->pressure_pa    = (float) comp_data.pressure;
  }

  return err_code;
}




#endif