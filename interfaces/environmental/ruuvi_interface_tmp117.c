#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_tmp117.h"
#include "ruuvi_interface_i2c_tmp117.h"

static uint8_t  m_address;
static uint16_t ms_per_sample; 

static ruuvi_driver_status_t tmp117_soft_reset(void)
{
  uint16_t reset = TMP117_MASK_RESET & 0xFFFF;
  return ruuvi_interface_i2c_tmp117_write(m_address, TMP117_REG_CONFIGURATION, reset);
}

static ruuvi_driver_status_t tmp117_validate_id(void)
{
  uint16_t id;
  ruuvi_interface_i2c_tmp117_read(m_address, TMP117_REG_DEVICE_ID, &id);
  id &= TMP117_MASK_ID;
  return (TMP117_VALUE_ID == id)? RUUVI_DRIVER_SUCCESS : RUUVI_DRIVER_ERROR_NOT_FOUND;
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
  int16_t temperature = (reg_val > 32767)? reg_val - 65535 : reg_val;
  float temperature = (0.0078125 * temperature);
  if(TMP117_VALUE_TEMP_NA == reg_val || RUUVI_DRIVER_SUCCESS != err_code) { temperature = NAN; }
  return temperature;
}
