#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_tmp117.h"
#include "ruuvi_interface_i2c_tmp117.h"

static uint8_t  m_address;
static uint16_t ms_per_sample; 
static float    m_temperature;
static uint64_t m_timestamp;
static const char m_sensor_name[] = "TMP117";

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
  return (TMP117_VALUE_ID == id)? err_code : err_code | RUUVI_DRIVER_ERROR_NOT_FOUND;
}

static ruuvi_driver_status_t tmp117_oversampling_set(const uint8_t num_os)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_OS;
  switch(num_os)
  {
    case 1:
      reg_val |= TMP117_VALUE_OS_1;
      ms_per_sample = 16;
      break;
      
    case 8:
      reg_val |= TMP117_VALUE_OS_8;
      ms_per_sample = 125;
      break;

    case 32:
      reg_val |= TMP117_VALUE_OS_32;
      ms_per_sample = 500;
      break;

    case 64:
      reg_val |= TMP117_VALUE_OS_64;
      ms_per_sample = 1000;
      break;

    default:
      return RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }
  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION, reg_val);
  return err_code;
}

static ruuvi_driver_status_t tmp117_sleep(void)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_MODE;
  reg_val |= TMP117_VALUE_MODE_SLEEP;
  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION, reg_val);
  return  err_code;
}

static ruuvi_driver_status_t tmp117_sample(void)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_MODE;
  reg_val |= TMP117_VALUE_MODE_SINGLE;
  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION, reg_val);
  return  err_code;
}

static ruuvi_driver_status_t tmp117_continuous(void)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_CONFIGURATION, &reg_val);
  reg_val &= ~TMP117_MASK_MODE;
  reg_val |= TMP117_VALUE_MODE_CONT;
  err_code |= ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION, reg_val);
  return  err_code;
}

static float tmp117_read(void)
{
  uint16_t reg_val;
  ruuvi_driver_status_t err_code;
  err_code = ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_TEMP_RESULT, &reg_val);
  int16_t dec_temperature = (reg_val > 32767)? reg_val - 65535 : reg_val;
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
      do{
      err_code |= tmp117_validate_id();
      retries++;
      }while(RUUVI_DRIVER_ERROR_TIMEOUT == err_code && retries < 5);
      break;

    default:
      return  RUUVI_DRIVER_ERROR_INVALID_PARAM;
  }
  if(RUUVI_DRIVER_SUCCESS != err_code) { err_code = RUUVI_DRIVER_ERROR_NOT_FOUND; }

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
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
    tmp117_sleep();
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
  return err_code;
}
