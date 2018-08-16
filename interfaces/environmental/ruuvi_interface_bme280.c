/**
 * BME280 interface.
 * Requires Bosch BME280_driver, available under BSD-3 on GitHub.
 * Will only get compiled if RUUVI_INTERFACE_ENVIRONMENTAL_BME280_ENABLED is defined as true
 * Requires BME280_FLOAT_ENABLE defined in makefile.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_platform_external_includes.h"
#if RUUVI_INTERFACE_ENVIRONMENTAL_BME280_ENABLED
// Ruuvi headers
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_bme280.h"
#include "ruuvi_interface_environmental.h"
#include "ruuvi_interface_spi.h"
#include "ruuvi_interface_yield.h"

#include <string.h>

// Bosch driver.
#include "bme280.h"
#include "bme280_defs.h"
#include "bme280_selftest.h"
#ifndef BME280_FLOAT_ENABLE
  #error "Please #define BME280_FLOAT_ENABLE in makefile CFLAGS"
#endif
// XXX Find out why RUUVI_DRIVER_UINT64_INVALID is not defined here
#ifndef RUUVI_DRIVER_UINT64_INVALID
  #define RUUVI_DRIVER_UINT64_INVALID UINT64_MAX
#endif


/** State variables **/
static struct bme280_dev dev = {0};

/**
 * Convert error from BME280 driver to appropriate NRF ERROR
 */
static ruuvi_driver_status_t BME_TO_RUUVI_ERROR(int8_t rslt)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  if(BME280_E_DEV_NOT_FOUND == rslt)  { err_code = RUUVI_DRIVER_ERROR_NOT_FOUND; }
  else if(BME280_E_NULL_PTR == rslt)  { err_code = RUUVI_DRIVER_ERROR_NULL; }
  else if(BME280_E_COMM_FAIL == rslt) { err_code = RUUVI_DRIVER_ERROR_BUSY; }
  else if(BME280_OK == rslt) return RUUVI_DRIVER_SUCCESS;
  return err_code;
}

static void bosch_delay_ms(uint32_t time)
{
  ruuvi_platform_delay_ms(time);
}

/** Initialize BME280 into low-power mode **/
ruuvi_driver_status_t ruuvi_interface_bme280_init(ruuvi_driver_sensor_t* environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  if(NULL == environmental_sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  switch(bus)
  {
    case RUUVI_DRIVER_BUS_SPI:
      /* Sensor_0 interface over SPI with native chip select line */
      dev.dev_id = handle;
      dev.intf = BME280_SPI_INTF;
      dev.read = ruuvi_platform_spi_bosch_write;
      dev.write = ruuvi_platform_spi_bosch_write;
      dev.delay_ms = bosch_delay_ms;

      err_code |= BME_TO_RUUVI_ERROR(bme280_init(&dev));
      err_code |= BME_TO_RUUVI_ERROR(bme280_crc_selftest(&dev));
      err_code |= BME_TO_RUUVI_ERROR(bme280_soft_reset(&dev));

      // Setup Oversampling 1 to enable sensor
      uint8_t dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      uint8_t dsp_parameter = 1;
      err_code |= ruuvi_interface_bme280_dsp_set(&dsp, &dsp_parameter);
      break;

    case RUUVI_DRIVER_BUS_I2C:
      return RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED;

    case RUUVI_DRIVER_BUS_NONE:
    default:
      return  RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  if (RUUVI_DRIVER_SUCCESS == err_code)
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
  }
  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_bme280_uninit(ruuvi_driver_sensor_t* sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  memset(sensor, 0, sizeof(ruuvi_driver_sensor_t));
  return BME_TO_RUUVI_ERROR(bme280_soft_reset(&dev));
}

ruuvi_driver_status_t ruuvi_interface_bme280_samplerate_set(uint8_t* samplerate)
{
  if(0 == *samplerate ) { return RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }
  else if(*samplerate == 1)   { dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
  else if(*samplerate == 2)   { dev.settings.standby_time = BME280_STANDBY_TIME_500_MS; }
  else if(*samplerate <= 8)   { dev.settings.standby_time = BME280_STANDBY_TIME_125_MS; }
  else if(*samplerate <= 16)  { dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS; }
  else if(*samplerate <= 50)  { dev.settings.standby_time = BME280_STANDBY_TIME_20_MS; }
  else if(*samplerate <= 100) { dev.settings.standby_time = BME280_STANDBY_TIME_10_MS; }
  else if(*samplerate <= 200) { dev.settings.standby_time = BME280_STANDBY_TIME_1_MS; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MIN       == *samplerate) { dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX       == *samplerate) { dev.settings.standby_time = BME280_STANDBY_TIME_1_MS; }
  else { return RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }

  return BME_TO_RUUVI_ERROR(bme280_set_sensor_settings(BME280_STANDBY_SEL, &dev));

}

ruuvi_driver_status_t ruuvi_interface_bme280_samplerate_get(uint8_t* samplerate)
{
  ruuvi_driver_status_t err_code = BME_TO_RUUVI_ERROR(bme280_get_sensor_settings(&dev));

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
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}
ruuvi_driver_status_t ruuvi_interface_bme280_resolution_get(uint8_t* resolution)
{
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_bme280_scale_set(uint8_t* scale)
{
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_bme280_scale_get(uint8_t* scale)
{
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_bme280_dsp_set(uint8_t* dsp, uint8_t* parameter)
{
  // Validate configuration
  if(   1  != *parameter
     && 2  != *parameter
     && 4  != *parameter
     && 8  != *parameter
     && 16 != *parameter
     && RUUVI_DRIVER_SENSOR_DSP_LAST != *dsp)
  {
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }

  // Error if DSP is not last, and if dsp is something else than IIR or OS
  if((RUUVI_DRIVER_SENSOR_DSP_LAST != *dsp) &&
     (~RUUVI_DRIVER_SENSOR_DSP_IIR | RUUVI_DRIVER_SENSOR_DSP_OS) & (*dsp) )
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
  if(RUUVI_DRIVER_SENSOR_DSP_IIR & *dsp)
  {
    switch(*parameter)
    {
      case 1:
        dev.settings.filter = BME280_FILTER_COEFF_OFF;
        break;
      case 2:
        dev.settings.filter = BME280_FILTER_COEFF_2;
        break;
      case 4:
        dev.settings.filter = BME280_FILTER_COEFF_4;
        break;
      case 8:
        dev.settings.filter = BME280_FILTER_COEFF_8;
        break;
      case 16:
        dev.settings.filter = BME280_FILTER_COEFF_16;
        break;
      default:
        return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
        break;
    }
  }
  // Setup Oversampling
  if(RUUVI_DRIVER_SENSOR_DSP_OS & *dsp)
  {
    switch(*parameter)
    {
      case 1:
          dev.settings.osr_h = BME280_OVERSAMPLING_1X;
          dev.settings.osr_p = BME280_OVERSAMPLING_1X;
          dev.settings.osr_t = BME280_OVERSAMPLING_1X;
        break;
      case 2:
          dev.settings.osr_h = BME280_OVERSAMPLING_2X;
          dev.settings.osr_p = BME280_OVERSAMPLING_2X;
          dev.settings.osr_t = BME280_OVERSAMPLING_2X;
        break;
      case 4:
          dev.settings.osr_h = BME280_OVERSAMPLING_4X;
          dev.settings.osr_p = BME280_OVERSAMPLING_4X;
          dev.settings.osr_t = BME280_OVERSAMPLING_4X;
        break;
      case 8:
          dev.settings.osr_h = BME280_OVERSAMPLING_8X;
          dev.settings.osr_p = BME280_OVERSAMPLING_8X;
          dev.settings.osr_t = BME280_OVERSAMPLING_8X;
        break;
      case 16:
          dev.settings.osr_h = BME280_OVERSAMPLING_16X;
          dev.settings.osr_p = BME280_OVERSAMPLING_16X;
          dev.settings.osr_t = BME280_OVERSAMPLING_16X;
        break;
      default:
        return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
        break;
    }
  }
  //Write configuration
  return BME_TO_RUUVI_ERROR(bme280_set_sensor_settings(settings_sel, &dev));
}

// TODO
ruuvi_driver_status_t ruuvi_interface_bme280_dsp_get(uint8_t* dsp, uint8_t* parameter)
{
  return RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED;
}

ruuvi_driver_status_t ruuvi_interface_bme280_mode_set(uint8_t* mode)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  switch(*mode)
  {
    case RUUVI_DRIVER_SENSOR_CFG_SLEEP:
      err_code = BME_TO_RUUVI_ERROR(bme280_set_sensor_mode(BME280_SLEEP_MODE, &dev));
      break;

    case RUUVI_DRIVER_SENSOR_CFG_SINGLE:
      err_code = BME_TO_RUUVI_ERROR(bme280_set_sensor_mode(BME280_FORCED_MODE, &dev));
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
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t bme_mode;
  err_code = BME_TO_RUUVI_ERROR(bme280_get_sensor_mode(&bme_mode, &dev));
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
  ruuvi_interface_environmental_data_t* p_data = (ruuvi_interface_environmental_data_t*)data;
  struct bme280_data comp_data;
  int8_t rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
  p_data->timestamp_ms   = RUUVI_DRIVER_UINT64_INVALID;
  p_data->temperature_c  = (float) comp_data.temperature;
  p_data->humidity_rh    = (float) comp_data.humidity;
  p_data->pressure_pa    = (float) comp_data.pressure;
  return BME_TO_RUUVI_ERROR(rslt);
}




#endif