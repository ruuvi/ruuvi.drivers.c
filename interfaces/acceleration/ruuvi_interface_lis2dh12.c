/**
 * LIS2DH12 interface.
 * Requires STM lis2dh12 driver, available on GitHub under unknown locense.
 * Requires "application_config.h", will only get compiled if LIS2DH12_ACCELERATION is defined
 * Requires "boards.h" for slave select pin
 * Requires floats enabled in application
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_INTERFACE_ACCELERATION_LIS2DH12_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_acceleration.h"
#include "ruuvi_interface_lis2dh12.h"
#include "ruuvi_interface_spi_lis2dh12.h"
#include "ruuvi_interface_yield.h"

#include "lis2dh12_reg.h"

#include <stdlib.h>
#include <string.h>

// Macro for checking that sensor is in sleep mode before configuration
#define VERIFY_SENSOR_SLEEPS() do { \
          uint8_t MACRO_MODE = 0; \
          ruuvi_interface_lis2dh12_mode_get(&MACRO_MODE); \
          if(RUUVI_DRIVER_SENSOR_CFG_SLEEP != MACRO_MODE) { return RUUVI_DRIVER_ERROR_INVALID_STATE; } \
          } while(0)

/*!
 * @brief lis2dh12 sensor settings structure.
 */
static struct
{
  // resolution
  lis2dh12_op_md_t resolution;

  // scale
  lis2dh12_fs_t scale;

  // data rate
  lis2dh12_odr_t samplerate;

  // self-test
  lis2dh12_st_t selftest;

  // operating mode, handle
  uint8_t mode, handle;

  uint64_t tsample; // Time of last sample in single mode.

  // device control structure
  lis2dh12_ctx_t ctx;
} dev = {0};

// Check that self-test values differ enough
static ruuvi_driver_status_t lis2dh12_verify_selftest_difference(axis3bit16_t* new,
    axis3bit16_t* old)
{
  if(LIS2DH12_2g != dev.scale || LIS2DH12_NM_10bit != dev.resolution) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  // Calculate positive diffs of each axes and compare to expected change
  for(size_t ii = 0; ii < 3; ii++)
  {
    int16_t diff = new->i16bit[ii] - old->i16bit[ii];
    //Compensate justification, check absolute difference
    diff >>= 6;

    if(0 > diff) { diff = 0 - diff; }

    if(RUUVI_INTERFACE_LIS2DH12_SELFTEST_DIFF_MIN > diff) { return RUUVI_DRIVER_ERROR_SELFTEST; }

    if(RUUVI_INTERFACE_LIS2DH12_SELFTEST_DIFF_MAX < diff) { return RUUVI_DRIVER_ERROR_SELFTEST; }
  }

  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_init(ruuvi_driver_sensor_t*
    acceleration_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == acceleration_sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  if(NULL != dev.ctx.write_reg) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  // Initialize mems driver interface
  lis2dh12_ctx_t* dev_ctx = &(dev.ctx);

  switch(bus)
  {
    case RUUVI_DRIVER_BUS_SPI:
      dev_ctx->write_reg = ruuvi_interface_spi_lis2dh12_write;
      dev_ctx->read_reg = ruuvi_interface_spi_lis2dh12_read;
      break;

    case RUUVI_DRIVER_BUS_I2C:
      return RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED;

    default:
      return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }

  dev.handle = handle;
  dev_ctx->handle = &dev.handle;
  dev.mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
  // Check device ID
  uint8_t whoamI = 0;
  lis2dh12_device_id_get(dev_ctx, &whoamI);

  if(whoamI != LIS2DH12_ID) { return RUUVI_DRIVER_ERROR_NOT_FOUND; }

  // Disable FIFO, activity
  lis2dh12_fm_t mode = LIS2DH12_BYPASS_MODE;
  lis2dh12_fifo_mode_set(&(dev.ctx), mode);
  ruuvi_interface_lis2dh12_fifo_use(false);
  ruuvi_interface_lis2dh12_fifo_interrupt_use(false);
  float ths = 0;
  ruuvi_interface_lis2dh12_activity_interrupt_use(false, &ths);
  // Turn X-, Y-, Z-measurement on
  uint8_t enable_axes = 0x07;
  lis2dh12_write_reg(dev_ctx, LIS2DH12_CTRL_REG1, &enable_axes, 1);
  // Disable Block Data Update, allow values to update even if old is not read
  lis2dh12_block_data_update_set(dev_ctx, PROPERTY_DISABLE);
  // Set Output Data Rate for self-test
  dev.samplerate = LIS2DH12_ODR_400Hz;
  lis2dh12_data_rate_set(dev_ctx, dev.samplerate);
  // Set full scale to 2G for self-test
  dev.scale = LIS2DH12_2g;
  lis2dh12_full_scale_set(dev_ctx, dev.scale);
  // (Don't) Enable temperature sensor
  //lis2dh12_temperature_meas_set(&dev_ctx, LIS2DH12_TEMP_ENABLE);
  // Set device in 10 bit mode
  dev.resolution = LIS2DH12_NM_10bit;
  lis2dh12_operating_mode_set(dev_ctx, dev.resolution);
  // Run self-test
  // turn self-test off.
  dev.selftest = LIS2DH12_ST_DISABLE;
  err_code |= lis2dh12_self_test_set(dev_ctx, dev.selftest);
  // wait for valid sample to be available, 3 samples at 400 Hz = 2.5 ms / sample => 7.5 ms. Wait 9 ms.
  ruuvi_interface_delay_ms(9);
  // read accelerometer
  axis3bit16_t data_raw_acceleration_old;
  axis3bit16_t data_raw_acceleration_new;
  memset(data_raw_acceleration_old.u8bit, 0x00, 3 * sizeof(int16_t));
  memset(data_raw_acceleration_new.u8bit, 0x00, 3 * sizeof(int16_t));
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_old.u8bit);
  // self-test to positive direction
  dev.selftest = LIS2DH12_ST_POSITIVE;
  lis2dh12_self_test_set(dev_ctx, dev.selftest);
  // wait 2 samples in low power or normal mode for valid data.
  ruuvi_interface_delay_ms(9);
  // Check self-test result
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  err_code |= lis2dh12_verify_selftest_difference(&data_raw_acceleration_new,
              &data_raw_acceleration_old);
  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_SUCCESS);
  // turn self-test off, keep error code in case we "lose" sensor after self-test
  dev.selftest = LIS2DH12_ST_DISABLE;
  err_code |= lis2dh12_self_test_set(dev_ctx, dev.selftest);
  // wait 2 samples and read value
  ruuvi_interface_delay_ms(9);
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_old.u8bit);
  // self-test to negative direction
  dev.selftest = LIS2DH12_ST_NEGATIVE;
  lis2dh12_self_test_set(dev_ctx, dev.selftest);
  // wait 2 samples
  ruuvi_interface_delay_ms(9);
  // Check self-test result
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  err_code |= lis2dh12_verify_selftest_difference(&data_raw_acceleration_new,
              &data_raw_acceleration_old);
  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_SUCCESS);
  // turn self-test off, keep error code in case we "lose" sensor after self-test
  dev.selftest = LIS2DH12_ST_DISABLE;
  err_code |= lis2dh12_self_test_set(dev_ctx, dev.selftest);
  // Turn accelerometer off
  dev.samplerate = LIS2DH12_POWER_DOWN;
  err_code |= lis2dh12_data_rate_set(dev_ctx, dev.samplerate);
  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_SUCCESS);

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    acceleration_sensor->init                  = ruuvi_interface_lis2dh12_init;
    acceleration_sensor->uninit                = ruuvi_interface_lis2dh12_uninit;
    acceleration_sensor->samplerate_set        = ruuvi_interface_lis2dh12_samplerate_set;
    acceleration_sensor->samplerate_get        = ruuvi_interface_lis2dh12_samplerate_get;
    acceleration_sensor->resolution_set        = ruuvi_interface_lis2dh12_resolution_set;
    acceleration_sensor->resolution_get        = ruuvi_interface_lis2dh12_resolution_get;
    acceleration_sensor->scale_set             = ruuvi_interface_lis2dh12_scale_set;
    acceleration_sensor->scale_get             = ruuvi_interface_lis2dh12_scale_get;
    acceleration_sensor->dsp_set               = ruuvi_interface_lis2dh12_dsp_set;
    acceleration_sensor->dsp_get               = ruuvi_interface_lis2dh12_dsp_get;
    acceleration_sensor->mode_set              = ruuvi_interface_lis2dh12_mode_set;
    acceleration_sensor->mode_get              = ruuvi_interface_lis2dh12_mode_get;
    acceleration_sensor->data_get              = ruuvi_interface_lis2dh12_data_get;
    acceleration_sensor->configuration_set     = ruuvi_driver_sensor_configuration_set;
    acceleration_sensor->configuration_get     = ruuvi_driver_sensor_configuration_get;
    acceleration_sensor->fifo_enable           = ruuvi_interface_lis2dh12_fifo_use;
    acceleration_sensor->fifo_interrupt_enable = ruuvi_interface_lis2dh12_fifo_interrupt_use;
    acceleration_sensor->fifo_read             = ruuvi_interface_lis2dh12_fifo_read;
    acceleration_sensor->level_interrupt_set   =
      ruuvi_interface_lis2dh12_activity_interrupt_use;
    dev.tsample = RUUVI_DRIVER_UINT64_INVALID;
  }

  return err_code;
}

/*
 * Lis2dh12 does not have a proper softreset (boot does not reset all registers)
 * Therefore just stop sampling
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_uninit(ruuvi_driver_sensor_t* sensor,
    ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  dev.samplerate = LIS2DH12_POWER_DOWN;
  memset(sensor, 0, sizeof(ruuvi_driver_sensor_t));
  //LIS2DH12 function returns SPI write result which is ruuvi_driver_status_t
  ruuvi_driver_status_t err_code = lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
  memset(&dev, 0, sizeof(dev));
  return err_code;
}

/**
 * Set up samplerate. Powers down sensor on SAMPLERATE_STOP, writes value to
 * lis2dh12 only if mode is continous as writing samplerate to sensor starts sampling.
 * MAX is 200 Hz as it can be represented by the configuration format
 * Samplerate is rounded up, i.e. "Please give me at least samplerate F.", 5 is rounded to 10 Hz etc.
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_samplerate_set(uint8_t* samplerate)
{
  if(NULL == samplerate)                                { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *samplerate)     {}
  else if(RUUVI_DRIVER_SENSOR_CFG_MIN == *samplerate)      { dev.samplerate = LIS2DH12_ODR_1Hz;   }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *samplerate)      { dev.samplerate = LIS2DH12_ODR_1kHz344_NM_HP; }
  else if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *samplerate)  { dev.samplerate = LIS2DH12_ODR_1Hz;   }
  else if(1   == *samplerate)                              { dev.samplerate = LIS2DH12_ODR_1Hz;   }
  else if(10  >= *samplerate)                              { dev.samplerate = LIS2DH12_ODR_10Hz;  }
  else if(25  >= *samplerate)                              { dev.samplerate = LIS2DH12_ODR_25Hz;  }
  else if(50  >= *samplerate)                              { dev.samplerate = LIS2DH12_ODR_50Hz;  }
  else if(100 >= *samplerate)                              { dev.samplerate = LIS2DH12_ODR_100Hz; }
  else if(200 >= *samplerate)                              { dev.samplerate = LIS2DH12_ODR_200Hz; }
  else if(RUUVI_DRIVER_SENSOR_CFG_CUSTOM_1 == *samplerate) { dev.samplerate = LIS2DH12_ODR_400Hz; }
  else if(RUUVI_DRIVER_SENSOR_CFG_CUSTOM_2 == *samplerate) { dev.samplerate = LIS2DH12_ODR_1kHz620_LP; }
  // This is equal to LIS2DH12_ODR_5kHz376_LP
  else if(RUUVI_DRIVER_SENSOR_CFG_CUSTOM_3 == *samplerate) { dev.samplerate = LIS2DH12_ODR_1kHz344_NM_HP; }
  else { *samplerate = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED; err_code |= RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
    err_code |= ruuvi_interface_lis2dh12_samplerate_get(samplerate);
  }

  return err_code;
}

/*
 *. Read sample rate to pointer
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_samplerate_get(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= lis2dh12_data_rate_get(&(dev.ctx), &dev.samplerate);

  switch(dev.samplerate)
  {
    case LIS2DH12_ODR_1Hz:
      *samplerate = 1;
      break;

    case LIS2DH12_ODR_10Hz:
      *samplerate = 10;
      break;

    case LIS2DH12_ODR_25Hz:
      *samplerate = 25;
      break;

    case LIS2DH12_ODR_50Hz:
      *samplerate = 50;
      break;

    case LIS2DH12_ODR_100Hz:
      *samplerate = 100;
      break;

    case LIS2DH12_ODR_200Hz:
      *samplerate = 200;
      break;

    case LIS2DH12_ODR_400Hz:
      *samplerate = RUUVI_DRIVER_SENSOR_CFG_CUSTOM_1;
      break;

    case LIS2DH12_ODR_1kHz344_NM_HP:
      *samplerate = RUUVI_DRIVER_SENSOR_CFG_MAX;
      break;

    case LIS2DH12_ODR_1kHz620_LP:
      *samplerate = RUUVI_DRIVER_SENSOR_CFG_CUSTOM_2;
      break;

    default:
      *samplerate = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
      err_code |=  RUUVI_DRIVER_ERROR_INTERNAL;
  }

  return err_code;
}

/**
 * Setup resolution. Resolution is rounded up, i.e. "please give at least this many bits"
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_resolution_set(uint8_t* resolution)
{
  if(NULL == resolution)                               { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *resolution)    { }
  else if(RUUVI_DRIVER_SENSOR_CFG_MIN == *resolution)     { dev.resolution = LIS2DH12_LP_8bit;  }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *resolution)     { dev.resolution = LIS2DH12_HR_12bit; }
  else if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *resolution) { dev.resolution = LIS2DH12_NM_10bit; }
  else if(8 >= *resolution)                              { dev.resolution = LIS2DH12_LP_8bit;  }
  else if(10 >= *resolution)                             { dev.resolution = LIS2DH12_NM_10bit; }
  else if(12 >= *resolution)                             { dev.resolution = LIS2DH12_HR_12bit; }
  else { *resolution = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED; err_code |= RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    err_code |= lis2dh12_operating_mode_set(&(dev.ctx), dev.resolution);
    err_code |= ruuvi_interface_lis2dh12_resolution_get(resolution);
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_resolution_get(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= lis2dh12_operating_mode_get(&(dev.ctx), &dev.resolution);

  switch(dev.resolution)
  {
    case LIS2DH12_LP_8bit:
      *resolution = 8;
      break;

    case LIS2DH12_NM_10bit:
      *resolution = 10;
      break;

    case LIS2DH12_HR_12bit:
      *resolution = 12;
      break;

    default:
      *resolution = RUUVI_DRIVER_SENSOR_ERR_INVALID;
      err_code |= RUUVI_DRIVER_ERROR_INTERNAL;
      break;
  }

  return err_code;
}

/**
 * Setup lis2dh12 scale. Scale is rounded up, i.e. "at least this much"
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_scale_set(uint8_t* scale)
{
  if(NULL == scale)                               { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *scale)    { }
  else if(RUUVI_DRIVER_SENSOR_CFG_MIN == *scale)     { dev.scale = LIS2DH12_2g; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *scale)     { dev.scale = LIS2DH12_16g; }
  else if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *scale) { dev.scale = LIS2DH12_2g;  }
  else if(2  >= *scale)                              { dev.scale = LIS2DH12_2g;  }
  else if(4  >= *scale)                              { dev.scale = LIS2DH12_4g;  }
  else if(8  >= *scale)                              { dev.scale = LIS2DH12_8g;  }
  else if(16 >= *scale)                              { dev.scale = LIS2DH12_16g; }
  else
  {
    *scale = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
    err_code |= RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    err_code |= lis2dh12_full_scale_set(&(dev.ctx), dev.scale);
    err_code |= ruuvi_interface_lis2dh12_scale_get(scale);
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_scale_get(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= lis2dh12_full_scale_get(&(dev.ctx), &dev.scale);

  switch(dev.scale)
  {
    case LIS2DH12_2g:
      *scale = 2;
      break;

    case LIS2DH12_4g:
      *scale = 4;
      break;

    case LIS2DH12_8g:
      *scale = 8;
      break;

    case  LIS2DH12_16g:
      *scale = 16;
      break;

    default:
      *scale = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
      err_code |= RUUVI_DRIVER_ERROR_INTERNAL;
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_dsp_set(uint8_t* dsp, uint8_t* parameter)
{
  if(NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  // Read original values in case one is NO_CHANGE and other should be adjusted.
  uint8_t orig_dsp, orig_param;
  err_code |= ruuvi_interface_lis2dh12_dsp_get(&orig_dsp, &orig_param);

  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *dsp)       { *dsp       = orig_dsp; }

  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *parameter) { *parameter = orig_param; }

  if(RUUVI_DRIVER_SENSOR_DSP_HIGH_PASS == *dsp)
  {
    lis2dh12_hpcf_t hpcf;

    // Set default here to avoid duplicate value error in switch-case
    if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *parameter) { *parameter = 0; }

    switch(*parameter)
    {
      case RUUVI_DRIVER_SENSOR_CFG_MIN:
      case 0:
        hpcf = LIS2DH12_LIGHT;
        *parameter = 0;
        break;

      case 1:
        hpcf = LIS2DH12_MEDIUM;
        break;

      case 2:
        hpcf = LIS2DH12_STRONG;
        break;

      case RUUVI_DRIVER_SENSOR_CFG_MAX:
      case 3:
        hpcf = LIS2DH12_AGGRESSIVE;
        *parameter = 3;
        break;

      default :
        *parameter = RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
        return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
    }

    err_code |= lis2dh12_high_pass_bandwidth_set(&(dev.ctx), hpcf);
    err_code |= lis2dh12_high_pass_mode_set(&(dev.ctx), LIS2DH12_NORMAL);
    err_code |= lis2dh12_high_pass_on_outputs_set(&(dev.ctx), PROPERTY_ENABLE);
    return err_code;
  }

  if(RUUVI_DRIVER_SENSOR_DSP_LAST == *dsp ||
      RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *dsp)
  {
    err_code |= lis2dh12_high_pass_on_outputs_set(&(dev.ctx), PROPERTY_DISABLE);
    *dsp = RUUVI_DRIVER_SENSOR_DSP_LAST;
    return err_code;
  }

  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_dsp_get(uint8_t* dsp, uint8_t* parameter)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  uint8_t mode, hpcf;
  err_code |= lis2dh12_high_pass_bandwidth_get(&(dev.ctx), &hpcf);
  err_code |= lis2dh12_high_pass_on_outputs_get(&(dev.ctx), &mode);

  if(mode) { *dsp = RUUVI_DRIVER_SENSOR_DSP_HIGH_PASS; }
  else { *dsp = RUUVI_DRIVER_SENSOR_DSP_LAST; }

  switch(hpcf)
  {
    case LIS2DH12_LIGHT:
      *parameter = 0;
      break;

    case LIS2DH12_MEDIUM:
      *parameter = 1;
      break;

    case LIS2DH12_STRONG:
      *parameter = 2;
      break;

    case LIS2DH12_AGGRESSIVE:
      *parameter = 3;
      break;

    default:
      *parameter  = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
      return RUUVI_DRIVER_ERROR_INTERNAL;
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_mode_set(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_CFG_SINGLE == *mode)
  {
    // Do nothing if sensor is in continuous mode
    uint8_t current_mode;
    ruuvi_interface_lis2dh12_mode_get(&current_mode);

    if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == current_mode)
    {
      *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
      return RUUVI_DRIVER_ERROR_INVALID_STATE;
    }

    // Start sensor at 400 Hz (highest common samplerate)
    // and wait for 7/ODR ms for turn-on (?) NOTE: 7 s / 400 just to be on safe side.
    // Refer to LIS2DH12 datasheet p.16.
    dev.samplerate = LIS2DH12_ODR_400Hz;
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
    ruuvi_interface_delay_ms((7000 / 400) + 1);
    dev.tsample = ruuvi_driver_sensor_timestamp_get();
    dev.samplerate = LIS2DH12_POWER_DOWN;
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
    *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
    return err_code;
  }

  // Do not store power down mode to dev structure, so we can continue at previous data rate
  // when mode is set to continous.
  if(RUUVI_DRIVER_SENSOR_CFG_SLEEP == *mode)
  {
    dev.mode = *mode;
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), LIS2DH12_POWER_DOWN);
  }
  else if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == *mode)
  {
    dev.mode = *mode;
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
  }
  else { err_code |= RUUVI_DRIVER_ERROR_INVALID_PARAM; }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_mode_get(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  switch(dev.mode)
  {
    case RUUVI_DRIVER_SENSOR_CFG_SLEEP:
      *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
      break;

    case RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS:
      *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
      break;

    default:
      *mode = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
      return RUUVI_DRIVER_ERROR_INTERNAL;
  }

  return RUUVI_DRIVER_SUCCESS;
}

/**
 * Convert raw value to acceleration in mg
 *
 * parameter raw: Input. Raw values from LIS2DH12
 * parameter acceleration: Output. Acceleration values in mg
 *
 */
static ruuvi_driver_status_t rawToMg(const axis3bit16_t* raw_acceleration,
                                     float* acceleration)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  for(size_t ii = 0; ii < 3; ii++)
  {
    switch(dev.scale)
    {
      case LIS2DH12_2g:
        switch(dev.resolution)
        {
          case LIS2DH12_LP_8bit:
            acceleration[ii] = LIS2DH12_FROM_FS_2g_LP_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          case LIS2DH12_NM_10bit:
            acceleration[ii] = LIS2DH12_FROM_FS_2g_NM_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          case LIS2DH12_HR_12bit:
            acceleration[ii] = LIS2DH12_FROM_FS_2g_HR_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          default:
            acceleration[ii] = RUUVI_INTERFACE_ACCELERATION_INVALID;
            err_code |= RUUVI_DRIVER_ERROR_INTERNAL;
            break;
        }

        break;

      case LIS2DH12_4g:
        switch(dev.resolution)
        {
          case LIS2DH12_LP_8bit:
            acceleration[ii] = LIS2DH12_FROM_FS_4g_LP_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          case LIS2DH12_NM_10bit:
            acceleration[ii] = LIS2DH12_FROM_FS_4g_NM_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          case LIS2DH12_HR_12bit:
            acceleration[ii] = LIS2DH12_FROM_FS_4g_HR_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          default:
            acceleration[ii] = RUUVI_INTERFACE_ACCELERATION_INVALID;
            err_code |= RUUVI_DRIVER_ERROR_INTERNAL;
            break;
        }

        break;

      case LIS2DH12_8g:
        switch(dev.resolution)
        {
          case LIS2DH12_LP_8bit:
            acceleration[ii] = LIS2DH12_FROM_FS_8g_LP_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          case LIS2DH12_NM_10bit:
            acceleration[ii] = LIS2DH12_FROM_FS_8g_NM_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          case LIS2DH12_HR_12bit:
            acceleration[ii] = LIS2DH12_FROM_FS_8g_HR_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          default:
            acceleration[ii] = RUUVI_INTERFACE_ACCELERATION_INVALID;
            err_code |= RUUVI_DRIVER_ERROR_INTERNAL;
            break;
        }

        break;

      case LIS2DH12_16g:
        switch(dev.resolution)
        {
          case LIS2DH12_LP_8bit:
            acceleration[ii] = LIS2DH12_FROM_FS_16g_LP_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          case LIS2DH12_NM_10bit:
            acceleration[ii] = LIS2DH12_FROM_FS_16g_NM_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          case LIS2DH12_HR_12bit:
            acceleration[ii] = LIS2DH12_FROM_FS_16g_HR_TO_mg(raw_acceleration->i16bit[ii]);
            break;

          default:
            acceleration[ii] = RUUVI_INTERFACE_ACCELERATION_INVALID;
            err_code |= RUUVI_DRIVER_ERROR_INTERNAL;
            break;
        }

        break;

      default:
        acceleration[ii] = RUUVI_INTERFACE_ACCELERATION_INVALID;
        err_code |= RUUVI_DRIVER_ERROR_INTERNAL;
        break;
    }
  }

  return err_code;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_data_get(void* data)
{
  if(NULL == data) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  axis3bit16_t raw_acceleration;
  memset(raw_acceleration.u8bit, 0x00, 3 * sizeof(int16_t));
  err_code |= lis2dh12_acceleration_raw_get(&(dev.ctx), raw_acceleration.u8bit);
  ruuvi_interface_acceleration_data_t* p_acceleration =
    (ruuvi_interface_acceleration_data_t*)data;
  float acceleration[3] = {0};
  p_acceleration->timestamp_ms = RUUVI_DRIVER_UINT64_INVALID;
  p_acceleration->x_g = RUUVI_INTERFACE_ACCELERATION_INVALID;
  p_acceleration->y_g = RUUVI_INTERFACE_ACCELERATION_INVALID;
  p_acceleration->z_g = RUUVI_INTERFACE_ACCELERATION_INVALID;
  // Compensate data with resolution, scale
  err_code |= rawToMg(&raw_acceleration, acceleration);
  uint8_t mode;
  err_code |= ruuvi_interface_lis2dh12_mode_get(&mode);

  if(RUUVI_DRIVER_SENSOR_CFG_SLEEP == mode)           {  p_acceleration->timestamp_ms   = dev.tsample; }
  else if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == mode) {  p_acceleration->timestamp_ms   = ruuvi_driver_sensor_timestamp_get(); }
  else { RUUVI_DRIVER_ERROR_CHECK(RUUVI_DRIVER_ERROR_INTERNAL, ~RUUVI_DRIVER_ERROR_FATAL); }

  // If we have valid data, return it.
  if(RUUVI_DRIVER_UINT64_INVALID != p_acceleration->timestamp_ms
      && RUUVI_DRIVER_SUCCESS == err_code)
  {
    //Convert mG to G
    p_acceleration->x_g = acceleration[0] / 1000.0;
    p_acceleration->y_g = acceleration[1] / 1000.0;
    p_acceleration->z_g = acceleration[2] / 1000.0;
  }

  return err_code;
}

// TODO: State checks
ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_use(const bool enable)
{
  lis2dh12_fm_t mode;

  if(enable) { mode = LIS2DH12_DYNAMIC_STREAM_MODE; }
  else { mode = LIS2DH12_BYPASS_MODE; }

  lis2dh12_fifo_set(&(dev.ctx), enable);
  return lis2dh12_fifo_mode_set(&(dev.ctx),  mode);
}

//TODO * return: RUUVI_DRIVER_INVALID_STATE if FIFO is not in use
ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_read(size_t* num_elements,
    ruuvi_driver_sensor_data_t* p_data)
{
  if(NULL == num_elements || NULL == p_data) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_interface_acceleration_data_t* data = (ruuvi_interface_acceleration_data_t*)p_data;
  uint8_t elements = 0;
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  err_code |= lis2dh12_fifo_data_level_get(&(dev.ctx), &elements);

  if(!elements)
  {
    *num_elements = 0;
    return RUUVI_DRIVER_SUCCESS;
  }

  // 31 FIFO + latest
  elements++;

  // Do not read more than buffer size
  if(elements > *num_elements) { elements = *num_elements; }

  // get current time
  uint64_t now = ruuvi_driver_sensor_timestamp_get();
  // Get samplerate
  uint8_t rate;
  ruuvi_interface_lis2dh12_samplerate_get(&rate);
  // Calculate sample interval
  uint32_t interval = 1000 / rate;
  // Read all elements
  axis3bit16_t raw_acceleration;
  float acceleration[3];

  for(size_t ii = 0; ii < elements; ii++)
  {
    err_code |= lis2dh12_acceleration_raw_get(&(dev.ctx), raw_acceleration.u8bit);
    // Compensate data with resolution, scale
    err_code |= rawToMg(&raw_acceleration, acceleration);
    data[ii].timestamp_ms = now - ((elements - ii - 1) * interval);
    data[ii].x_g = acceleration[0] / 1000;
    data[ii].y_g = acceleration[1] / 1000;
    data[ii].z_g = acceleration[2] / 1000;
  }

  *num_elements = elements;
  return err_code;
}


ruuvi_driver_status_t ruuvi_interface_lis2dh12_fifo_interrupt_use(const bool enable)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  lis2dh12_ctrl_reg3_t ctrl = { 0 };

  if(true == enable)
  {
    // Setting the FTH [4:0] bit in the FIFO_CTRL_REG (2Eh) register to an N value,
    // the number of X, Y and Z data samples that should be read at the rise of the watermark interrupt is up to (N+1).
    err_code |= lis2dh12_fifo_watermark_set(&(dev.ctx), 31);
    ctrl.i1_wtm = PROPERTY_ENABLE;
  }

  err_code |= lis2dh12_pin_int1_config_set(&(dev.ctx), &ctrl);
  return err_code;
}

/**
 * Enable activity interrupt on LIS2DH12
 * Triggers as ACTIVE HIGH interrupt while detected movement is above threshold limit_g
 * Axes are high-passed for this interrupt, i.e. gravity won't trigger the interrupt
 * Axes are examined individually, compound acceleration won't trigger the interrupt.
 *
 * parameter enable:  True to enable interrupt, false to disable interrupt
 * parameter limit_g: Desired acceleration to trigger the interrupt.
 *                    Is considered as "at least", the acceleration is rounded up to next value.
 *                    Is written with value that was set to interrupt
 * returns: RUUVI_DRIVER_SUCCESS on success
 * returns: RUUVI_DRIVER_ERROR_INVALID_STATE if acceleration limit is higher than maximum scale
 * returns: error code from stack on error.
 *
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_activity_interrupt_use(const bool enable,
    float* limit_g)
{
  if(NULL == limit_g) { return RUUVI_DRIVER_ERROR_NULL; }

  if(0 > *limit_g)    { return RUUVI_DRIVER_ERROR_INVALID_PARAM; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  lis2dh12_hp_t high_pass = LIS2DH12_ON_INT1_GEN;
  lis2dh12_ctrl_reg6_t ctrl6 = { 0 };
  lis2dh12_int1_cfg_t  cfg = { 0 };
  ctrl6.i2_ia1 = PROPERTY_ENABLE;

  if(enable)
  {
    cfg.xhie     = PROPERTY_ENABLE;
    cfg.yhie     = PROPERTY_ENABLE;
    cfg.zhie     = PROPERTY_ENABLE;
  }

  /*
  cfg.xlie     = PROPERTY_ENABLE;
  cfg.ylie     = PROPERTY_ENABLE;
  cfg.zlie     = PROPERTY_ENABLE;
  */
  // Adjust for scale
  // 1 LSb = 16 mg @ FS = 2 g
  // 1 LSb = 32 mg @ FS = 4 g
  // 1 LSb = 62 mg @ FS = 8 g
  // 1 LSb = 186 mg @ FS = 16 g
  uint8_t  scale;
  uint32_t threshold;
  float divisor;
  err_code |= ruuvi_interface_lis2dh12_scale_get(&scale);

  switch(scale)
  {
    case 2:
      divisor = 0.016f;
      break;

    case 4:
      divisor = 0.032f;
      break;

    case 8:
      divisor = 0.062f;
      break;

    case 16:
      divisor = 0.186f;
      break;

    default:
      divisor = 0.016f;
      break;
  }

  threshold = (uint32_t)(*limit_g / divisor) + 1;

  if(threshold > 0x7F) { return RUUVI_DRIVER_ERROR_INVALID_PARAM; }

  *limit_g = threshold * divisor;
  // Configure highpass on INTERRUPT 1
  err_code |= lis2dh12_high_pass_int_conf_set(&(dev.ctx), high_pass);
  // Configure INTERRUPT 1 Threshold
  err_code |= lis2dh12_int1_gen_threshold_set(&(dev.ctx), threshold);
  // Configure INTERRUPT 1 ON ZHI, ZLO, YHI, YLO, XHI, XLO
  err_code |= lis2dh12_int1_gen_conf_set(&(dev.ctx), &cfg);
  // Route INTERRUPT 1 to PIN 2
  err_code |= lis2dh12_pin_int2_config_set(&(dev.ctx), &ctrl6);
  return err_code;
}

#endif