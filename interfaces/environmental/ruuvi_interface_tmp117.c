#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_tmp117.h"
#include "ruuvi_interface_i2c_tmp117.h"
#include "ruuvi_interface_yield.h"

#if (RUUVI_INTERFACE_ENVIRONMENTAL_TMP117_ENABLED || DOXYGEN)

/**
 * @addtogroup TMP117
 */
/*@{*/

/**
 * @file ruuvi_interface_tmp117.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-11-13
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * TMP117 temperature sensor driver.
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

static uint8_t  m_address;
static uint16_t ms_per_sample;
static uint16_t ms_per_cc;
static float    m_temperature;
static uint64_t m_timestamp;
static const char m_sensor_name[] = "TMP117";
static bool m_continuous = false;

static ruuvi_driver_status_t tmp117_soft_reset(void)
{
  uint16_t reset = TMP117_MASK_RESET & 0xFFFF;
  return ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION, reset);
}

static ruuvi_driver_status_t tmp117_validate_id(void)
{
  uint16_t id;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_DEVICE_ID, &id);
  id &= TMP117_MASK_ID;
  return (TMP117_VALUE_ID == id) ? err_code : err_code | RUUVI_DRIVER_ERROR_NOT_FOUND;
}

static ruuvi_driver_status_t tmp117_oversampling_set(const uint8_t num_os)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_OS;

  switch(num_os)
  {
    case TMP117_VALUE_OS_1:
      reg_val |= TMP117_VALUE_OS_1;

      if(16 > ms_per_cc) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      ms_per_sample = 16;
      break;

    case TMP117_VALUE_OS_8:
      reg_val |= TMP117_VALUE_OS_8;

      if(125 > ms_per_cc) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      ms_per_sample = 125;
      break;

    case TMP117_VALUE_OS_32:
      reg_val |= TMP117_VALUE_OS_32;

      if(500 > ms_per_cc) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      ms_per_sample = 500;
      break;

    case TMP117_VALUE_OS_64:
      reg_val |= TMP117_VALUE_OS_64;

      if(1000 > ms_per_cc) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      ms_per_sample = 1000;
      break;

    default:
      return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION,
              reg_val);
  return err_code;
}

static ruuvi_driver_status_t tmp117_samplerate_set(const uint16_t num_os)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_CC;

  switch(num_os)
  {
    case TMP117_VALUE_CC_16_MS:
      if(16 < ms_per_sample) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      reg_val |= TMP117_VALUE_CC_16_MS;
      ms_per_cc = 16;
      break;

    case TMP117_VALUE_CC_125_MS:
      if(125 < ms_per_sample) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      reg_val |= TMP117_VALUE_CC_125_MS;
      ms_per_cc = 125;
      break;

    case TMP117_VALUE_CC_250_MS:
      if(250 < ms_per_sample) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      reg_val |= TMP117_VALUE_CC_250_MS;
      ms_per_cc = 250;
      break;

    case TMP117_VALUE_CC_500_MS:
      if(500 < ms_per_sample) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      reg_val |= TMP117_VALUE_CC_500_MS;
      ms_per_cc = 500;
      break;

    case TMP117_VALUE_CC_1000_MS:
      if(1000 < ms_per_sample) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      reg_val |= TMP117_VALUE_CC_1000_MS;
      ms_per_cc = 1000;
      break;

    case TMP117_VALUE_CC_4000_MS:
      if(4000 < ms_per_sample) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      reg_val |= TMP117_VALUE_CC_4000_MS;
      ms_per_cc = 4000;
      break;

    case TMP117_VALUE_CC_8000_MS:
      if(8000 < ms_per_sample) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      reg_val |= TMP117_VALUE_CC_8000_MS;
      ms_per_cc = 8000;
      break;

    case TMP117_VALUE_CC_16000_MS:
      if(16000 < ms_per_sample) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

      reg_val |= TMP117_VALUE_CC_16000_MS;
      ms_per_cc = 16000;
      break;

    default:
      return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION,
              reg_val);
  return err_code;
}

static ruuvi_driver_status_t tmp117_sleep(void)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_MODE;
  reg_val |= TMP117_VALUE_MODE_SLEEP;
  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION,
              reg_val);
  return  err_code;
}

static ruuvi_driver_status_t tmp117_sample(void)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_MODE;
  reg_val |= TMP117_VALUE_MODE_SINGLE;
  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION,
              reg_val);
  m_timestamp = ruuvi_driver_sensor_timestamp_get();
  return  err_code;
}

static ruuvi_driver_status_t tmp117_continuous(void)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_MODE;
  reg_val |= TMP117_VALUE_MODE_CONT;
  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION,
              reg_val);
  return  err_code;
}

static float tmp117_read(void)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_TEMP_RESULT, &reg_val);
  int16_t dec_temperature = (reg_val > 32767) ? reg_val - 65535 : reg_val;
  float temperature = (0.0078125 * dec_temperature);

  if(TMP117_VALUE_TEMP_NA == reg_val || RUUVI_DRIVER_SUCCESS != err_code) { temperature = NAN; }

  return temperature;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_init(ruuvi_driver_sensor_t*
    environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == environmental_sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  if(m_address) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ruuvi_driver_sensor_initialize(environmental_sensor);
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  m_address = handle;
  size_t retries = 0;

  switch(bus)
  {
    case RUUVI_DRIVER_BUS_I2C:
      do
      {
        err_code |= tmp117_validate_id();
        retries++;
      } while(RUUVI_DRIVER_ERROR_TIMEOUT == err_code && retries < 5);

      break;

    default:
      return  RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  if(RUUVI_DRIVER_SUCCESS != err_code) { err_code = RUUVI_DRIVER_ERROR_NOT_FOUND; }

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    err_code |= tmp117_soft_reset();
    environmental_sensor->init              = ruuvi_interface_tmp117_init;
    environmental_sensor->uninit            = ruuvi_interface_tmp117_uninit;
    environmental_sensor->samplerate_set    = ruuvi_interface_tmp117_samplerate_set;
    environmental_sensor->samplerate_get    = ruuvi_interface_tmp117_samplerate_get;
    environmental_sensor->resolution_set    = ruuvi_interface_tmp117_resolution_set;
    environmental_sensor->resolution_get    = ruuvi_interface_tmp117_resolution_get;
    environmental_sensor->scale_set         = ruuvi_interface_tmp117_scale_set;
    environmental_sensor->scale_get         = ruuvi_interface_tmp117_scale_get;
    environmental_sensor->dsp_set           = ruuvi_interface_tmp117_dsp_set;
    environmental_sensor->dsp_get           = ruuvi_interface_tmp117_dsp_get;
    environmental_sensor->mode_set          = ruuvi_interface_tmp117_mode_set;
    environmental_sensor->mode_get          = ruuvi_interface_tmp117_mode_get;
    environmental_sensor->data_get          = ruuvi_interface_tmp117_data_get;
    environmental_sensor->configuration_set = ruuvi_driver_sensor_configuration_set;
    environmental_sensor->configuration_get = ruuvi_driver_sensor_configuration_get;
    environmental_sensor->name              = m_sensor_name;
    environmental_sensor->provides.datas.temperature_c = 1;
    m_timestamp = RUUVI_DRIVER_UINT64_INVALID;
    m_temperature = NAN;
    ms_per_cc = 1000;
    ms_per_sample = 16;
    m_continuous = false;
    err_code |= tmp117_sleep();
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_uninit(ruuvi_driver_sensor_t* sensor,
    ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  tmp117_sleep();
  ruuvi_driver_sensor_uninitialize(sensor);
  m_timestamp = RUUVI_DRIVER_UINT64_INVALID;
  m_temperature = NAN;
  m_address = 0;
  m_continuous = false;
  return err_code;
}


ruuvi_driver_status_t ruuvi_interface_tmp117_samplerate_set(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  if(m_continuous) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *samplerate)
  {
    return ruuvi_interface_tmp117_samplerate_get(samplerate);
  }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *samplerate ||
      1 >= *samplerate)
  {
    *samplerate = 1;
    err_code |= tmp117_samplerate_set(TMP117_VALUE_CC_1000_MS);
  }
  else if(2 >= *samplerate)
  {
    *samplerate = 2;
    err_code |= tmp117_samplerate_set(TMP117_VALUE_CC_500_MS);
  }
  else if(4 >= *samplerate)
  {
    *samplerate = 4;
    err_code |= tmp117_samplerate_set(TMP117_VALUE_CC_250_MS);
  }
  else if(8 >= *samplerate)
  {
    *samplerate = 8;
    err_code |= tmp117_samplerate_set(TMP117_VALUE_CC_125_MS);
  }
  else if(64 >= *samplerate ||
          RUUVI_DRIVER_SENSOR_CFG_MAX == *samplerate)
  {
    *samplerate = 64;
    err_code |= tmp117_samplerate_set(TMP117_VALUE_CC_16_MS);
  }
  else if(RUUVI_DRIVER_SENSOR_CFG_CUSTOM_1 == *samplerate)
  {
    err_code |= tmp117_samplerate_set(TMP117_VALUE_CC_4000_MS);
  }
  else if(RUUVI_DRIVER_SENSOR_CFG_CUSTOM_2 == *samplerate)
  {
    err_code |= tmp117_samplerate_set(TMP117_VALUE_CC_8000_MS);
  }
  else if(RUUVI_DRIVER_SENSOR_CFG_CUSTOM_3 == *samplerate ||
          RUUVI_DRIVER_SENSOR_CFG_MIN == *samplerate)
  {
    *samplerate = RUUVI_DRIVER_SENSOR_CFG_CUSTOM_3;
    err_code |= tmp117_samplerate_set(TMP117_VALUE_CC_16000_MS);
  }
  else { err_code |= RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }

  return  err_code;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_samplerate_get(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint16_t reg_val;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= TMP117_MASK_CC;

  switch(reg_val)
  {
    case TMP117_VALUE_CC_16_MS:
      *samplerate = 64;
      break;

    case TMP117_VALUE_CC_125_MS:
      *samplerate = 8;
      break;

    case TMP117_VALUE_CC_250_MS:
      *samplerate = 4;
      break;

    case TMP117_VALUE_CC_500_MS:
      *samplerate = 2;
      break;

    case TMP117_VALUE_CC_1000_MS:
      *samplerate = 1;
      break;

    case TMP117_VALUE_CC_4000_MS:
      *samplerate = RUUVI_DRIVER_SENSOR_CFG_CUSTOM_1;
      break;

    case TMP117_VALUE_CC_8000_MS:
      *samplerate = RUUVI_DRIVER_SENSOR_CFG_CUSTOM_2;
      break;

    case TMP117_VALUE_CC_16000_MS:
      *samplerate = RUUVI_DRIVER_SENSOR_CFG_CUSTOM_3;
      break;

    default:
      return RUUVI_DRIVER_ERROR_INTERNAL;
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_resolution_set(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

  if(m_continuous) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  uint8_t original = *resolution;
  *resolution = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  RETURN_SUCCESS_ON_VALID(original);
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_resolution_get(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

  *resolution = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_scale_set(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

  if(m_continuous) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  uint8_t original = *scale;
  *scale = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  RETURN_SUCCESS_ON_VALID(original);
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_scale_get(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

  *scale = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_dsp_set(uint8_t* dsp, uint8_t* parameter)
{
  if(NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }

  if(m_continuous) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == * dsp)
  {
    return ruuvi_interface_tmp117_dsp_get(dsp, parameter);
  }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_DSP_LAST == *dsp ||
      RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *dsp)
  {
    err_code |= tmp117_oversampling_set(TMP117_VALUE_OS_1);
    *parameter = 1;
  }
  else if(RUUVI_DRIVER_SENSOR_DSP_OS == *dsp)
  {
    if(1 >= *parameter)
    {
      *parameter = 1;
      err_code |= tmp117_oversampling_set(TMP117_VALUE_OS_1);
    }
    else if(8 >= *parameter)
    {
      *parameter = 8;
      err_code |= tmp117_oversampling_set(TMP117_VALUE_OS_8);
    }
    else if(32 >= *parameter)
    {
      *parameter = 32;
      err_code |= tmp117_oversampling_set(TMP117_VALUE_OS_32);
    }
    else if(64 >= *parameter)
    {
      *parameter = 64;
      err_code |= tmp117_oversampling_set(TMP117_VALUE_OS_64);
    }
    else { err_code |= RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }
  }
  else { err_code |= RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_dsp_get(uint8_t* dsp, uint8_t* parameter)
{
  if(NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }

  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= TMP117_MASK_OS;

  switch(reg_val)
  {
    case TMP117_VALUE_OS_1:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_LAST;
      *parameter = 1;
      break;

    case TMP117_VALUE_OS_8:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 8;
      break;

    case TMP117_VALUE_OS_32:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 32;
      break;

    case TMP117_VALUE_OS_64:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 64;
      break;
  }
  return err_code;
}


ruuvi_driver_status_t ruuvi_interface_tmp117_mode_set(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  switch(*mode)
  {
    case RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS:
      err_code |= tmp117_continuous();
      m_continuous = true;
      break;

    case RUUVI_DRIVER_SENSOR_CFG_SINGLE:
      if(m_continuous)
      {
        *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
        return RUUVI_DRIVER_ERROR_INVALID_STATE;
      }

      err_code |= tmp117_sample();
      ruuvi_interface_delay_ms(ms_per_sample);
      *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
      break;

    case RUUVI_DRIVER_SENSOR_CFG_SLEEP:
      err_code |= tmp117_sleep();
      m_continuous = false;
      break;

    default:
      err_code |= RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_mode_get(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  *mode = m_continuous ? RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS : RUUVI_DRIVER_SENSOR_CFG_SLEEP;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_tmp117_data_get(ruuvi_driver_sensor_data_t* const
    data)
{
  if(NULL == data) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(m_continuous)
  {
    m_temperature = tmp117_read();
    m_timestamp = ruuvi_driver_sensor_timestamp_get();
  }

  if(RUUVI_DRIVER_SUCCESS == err_code && RUUVI_DRIVER_UINT64_INVALID != m_timestamp)
  {
    ruuvi_driver_sensor_data_fields_t env_fields = {.bitfield = 0};
    env_fields.datas.temperature_c = 1;
    ruuvi_driver_sensor_data_set(data,
                                 env_fields,
                                 m_temperature);
    data->timestamp_ms = m_timestamp;
  }

  return err_code;
}
/*@}*/
#endif