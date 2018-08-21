/**
 * LIS2DH12 interface.
 * Requires STM lis2dh12 driver, available on GitHub under unknown locense.
 * Requires "application_config.h", will only get compiled if LIS2DH12_ACCELERATION is defined
 * Requires "boards.h" for slave select pin
 * Requires floats enabled in application
 */

#include "ruuvi_platform_external_includes.h"
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

// XXX Find out why RUUVI_DRIVER_UINT64_INVALID is not defined here
#ifndef RUUVI_DRIVER_UINT64_INVALID
  #define RUUVI_DRIVER_UINT64_INVALID UINT64_MAX
#endif

/*!
 * @brief lis2dh12 sensor settings structure.
 */
static struct {
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

  // device control structure
  lis2dh12_ctx_t ctx;
}dev;

// Check that self-test values differ enough
static ruuvi_driver_status_t lis2dh12_verify_selftest_difference(axis3bit16_t* new, axis3bit16_t* old)
{
  if(LIS2DH12_2g != dev.scale || LIS2DH12_NM_10bit != dev.resolution) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  // Calculate positive diffs of each axes and compare to expected change
  for(size_t ii = 0; ii < 3; ii++)
  {
    int16_t diff = new->i16bit[ii] - old->i16bit[ii];
    //Compensate justification
    diff >>=6;
    if(0 > diff) { diff = 0 - diff; }
    // Difference should be more than 17 and less than 360
    if(diff < 17)  { return RUUVI_DRIVER_ERROR_SELFTEST; }
    if(diff > 360) { return RUUVI_DRIVER_ERROR_SELFTEST; }
  }
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_init(ruuvi_driver_sensor_t* acceleration_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == acceleration_sensor) { return RUUVI_DRIVER_ERROR_NULL; }
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
  if ( whoamI != LIS2DH12_ID ) { return RUUVI_DRIVER_ERROR_NOT_FOUND; }

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
  // wait for sample to be available
  ruuvi_platform_delay_ms(3);

  // read accelerometer
  axis3bit16_t data_raw_acceleration_old;
  axis3bit16_t data_raw_acceleration_new;
  memset(data_raw_acceleration_old.u8bit, 0x00, 3*sizeof(int16_t));
  memset(data_raw_acceleration_new.u8bit, 0x00, 3*sizeof(int16_t));
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_old.u8bit);

  // self-test to positive direction
  dev.selftest = LIS2DH12_ST_POSITIVE;
  lis2dh12_self_test_set(dev_ctx, dev.selftest);

  // wait 2 samples - LP, normal mode
  ruuvi_platform_delay_ms(9);

  // Check self-test result
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  err_code |= lis2dh12_verify_selftest_difference(&data_raw_acceleration_new, &data_raw_acceleration_old);

  // self-test to negative direction
  dev.selftest = LIS2DH12_ST_NEGATIVE;
  lis2dh12_self_test_set(dev_ctx, dev.selftest);

  // wait 2 samples
  ruuvi_platform_delay_ms(9);

  // Check self-test result
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  err_code |= lis2dh12_verify_selftest_difference(&data_raw_acceleration_new, &data_raw_acceleration_old);

  // turn self-test off, keep error code in case we "lose" sensor after self-test
  dev.selftest = LIS2DH12_ST_DISABLE;
  err_code |= lis2dh12_self_test_set(dev_ctx, dev.selftest);

  // Turn accelerometer off
  dev.samplerate = LIS2DH12_POWER_DOWN;
  err_code |= lis2dh12_data_rate_set(dev_ctx, dev.samplerate);

  if(RUUVI_DRIVER_SUCCESS == err_code)
  {
    acceleration_sensor->init           = ruuvi_interface_lis2dh12_init;
    acceleration_sensor->uninit         = ruuvi_interface_lis2dh12_uninit;
    acceleration_sensor->samplerate_set = ruuvi_interface_lis2dh12_samplerate_set;
    acceleration_sensor->samplerate_get = ruuvi_interface_lis2dh12_samplerate_get;
    acceleration_sensor->resolution_set = ruuvi_interface_lis2dh12_resolution_set;
    acceleration_sensor->resolution_get = ruuvi_interface_lis2dh12_resolution_get;
    acceleration_sensor->scale_set      = ruuvi_interface_lis2dh12_scale_set;
    acceleration_sensor->scale_get      = ruuvi_interface_lis2dh12_scale_get;
    acceleration_sensor->dsp_set        = ruuvi_interface_lis2dh12_dsp_set;
    acceleration_sensor->dsp_get        = ruuvi_interface_lis2dh12_dsp_get;
    acceleration_sensor->mode_set       = ruuvi_interface_lis2dh12_mode_set;
    acceleration_sensor->mode_get       = ruuvi_interface_lis2dh12_mode_get;
    acceleration_sensor->data_get       = ruuvi_interface_lis2dh12_data_get;
 }

  return err_code;
}

/*
 * Lis2dh12 does not have a proper softreset (boot does not reset all registers)
 * Therefore just stop sampling
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_uninit(ruuvi_driver_sensor_t* sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == sensor) { return RUUVI_DRIVER_ERROR_NULL; }
  dev.samplerate = LIS2DH12_POWER_DOWN;
  memset(sensor, 0, sizeof(ruuvi_driver_sensor_t));
  //LIS2DH12 function returns SPI write result which is ruuvi_status_t
  return lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
}

/**
 * Set up samplerate. Powers down sensor on SAMPLERATE_STOP, writes value to
 * lis2dh12 only if mode is continous as writing samplerate to sensor starts sampling.
 * MAX is 400 Hz as it is valid for all power settings
 * Samplerate is rounded up, i.e. "Please give me at least samplerate F.", 5 is rounded to 10 Hz etc.
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_samplerate_set(uint8_t* samplerate)
{
  if(NULL == samplerate)                                { return RUUVI_DRIVER_ERROR_NULL; }
  if(RUUVI_DRIVER_SENSOR_CFG_SINGLE == *samplerate)     { return RUUVI_DRIVER_ERROR_NOT_SUPPORTED; } //HW does not support.
  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *samplerate)  { return RUUVI_DRIVER_SUCCESS; }
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

  if(RUUVI_DRIVER_SENSOR_CFG_SLEEP == *samplerate)
  {
    dev.samplerate = LIS2DH12_POWER_DOWN;
    *samplerate = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
  }
  else if(RUUVI_DRIVER_SENSOR_CFG_MIN == *samplerate)    { dev.samplerate = LIS2DH12_ODR_1Hz;   *samplerate = 1; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *samplerate)
  {
    dev.samplerate = LIS2DH12_ODR_400Hz;
    *samplerate = RUUVI_DRIVER_SENSOR_CFG_MAX;
  }
  else if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *samplerate){ dev.samplerate = LIS2DH12_ODR_1Hz;   *samplerate = 1; }
  else if(1   == *samplerate)                            { dev.samplerate = LIS2DH12_ODR_1Hz;   *samplerate = 1; }
  else if(10  >= *samplerate)                            { dev.samplerate = LIS2DH12_ODR_10Hz;  *samplerate = 10; }
  else if(25  >= *samplerate)                            { dev.samplerate = LIS2DH12_ODR_25Hz;  *samplerate = 25; }
  else if(50  >= *samplerate)                            { dev.samplerate = LIS2DH12_ODR_50Hz;  *samplerate = 50; }
  else if(100 >= *samplerate)                            { dev.samplerate = LIS2DH12_ODR_100Hz; *samplerate = 100; }
  else if(200 >= *samplerate)                            { dev.samplerate = LIS2DH12_ODR_200Hz; *samplerate = 200; }
  else { return RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }

  // Write samplerate to lis if we're in continous mode or if sample rate is 0.
  // If we're in sleep mode the sampling starts once device goes to continous mode.
  if(dev.mode == RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS ||
     dev.samplerate == LIS2DH12_POWER_DOWN)
  {
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
  }
  return err_code;
}

/*
 *. Read samplerate to pointer
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_samplerate_get(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  switch(dev.samplerate)
  {
    case LIS2DH12_POWER_DOWN:
      *samplerate = 0;
      break;

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
      *samplerate = RUUVI_DRIVER_SENSOR_CFG_MAX;
      break;

    default:
      *samplerate = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
      break;
  }
  return RUUVI_DRIVER_SUCCESS;
}

/**
 * Setup resolution. Resolution is rounded up, i.e. "please give at least this many bits"
 */
ruuvi_driver_status_t ruuvi_interface_lis2dh12_resolution_set(uint8_t* resolution)
{
  if(NULL == resolution)                               { return RUUVI_DRIVER_ERROR_NULL; }
  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *resolution) { return RUUVI_DRIVER_SUCCESS; }

  if     (RUUVI_DRIVER_SENSOR_CFG_MIN == *resolution)     { dev.resolution = LIS2DH12_LP_8bit;  *resolution = 8; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *resolution)     { dev.resolution = LIS2DH12_HR_12bit; *resolution = 12; }
  else if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *resolution) { dev.samplerate = LIS2DH12_NM_10bit; *resolution = 10; }
  else if(8 >= *resolution )                              { dev.resolution = LIS2DH12_LP_8bit;  *resolution = 8; }
  else if(10 >= *resolution )                             { dev.resolution = LIS2DH12_NM_10bit; *resolution = 10; }
  else if(12 >= *resolution )                             { dev.resolution = LIS2DH12_HR_12bit; *resolution = 12; }
  else
  {
    *resolution = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }

  //TODO LIS to ruuvi error
  return lis2dh12_operating_mode_set(&(dev.ctx), dev.resolution);
}
ruuvi_driver_status_t ruuvi_interface_lis2dh12_resolution_get(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

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
    return RUUVI_DRIVER_ERROR_INTERNAL;
    break;
  }
  return RUUVI_DRIVER_SUCCESS;
}

/**
 * Setup lis2dh12 scale. Scale is rounded up, i.e. "at least this much"
 */
ruuvi_driver_status_t ruuvi_ionterface_lis2dh12_scale_set(uint8_t* scale)
{
  if(NULL == scale)                               { return RUUVI_DRIVER_ERROR_NULL; }
  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *scale) { return RUUVI_DRIVER_SUCCESS;    }

  if     (RUUVI_DRIVER_SENSOR_CFG_MIN == *scale)     { dev.scale = LIS2DH12_2g;  *scale = 2; }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *scale)     { dev.scale = LIS2DH12_16g; *scale = 16; }
  else if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *scale) { dev.scale = LIS2DH12_2g;  *scale = 2; }
  else if(2  >= *scale)                              { dev.scale = LIS2DH12_2g;  *scale = 2; }
  else if(4  >= *scale)                              { dev.scale = LIS2DH12_4g;  *scale = 4; }
  else if(8  >= *scale)                              { dev.scale = LIS2DH12_8g;  *scale = 8; }
  else if(16 >= *scale)                              { dev.scale = LIS2DH12_16g; *scale = 16; }
  else
  {
    *scale = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }
  return lis2dh12_full_scale_set(&(dev.ctx), dev.scale);
}

ruuvi_driver_status_t ruuvi_ionterface_lis2dh12_scale_get(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

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
       *scale = RUUVI_DRIVER_SENSOR_ERR_INVALID;
      return RUUVI_DRIVER_ERROR_INTERNAL;
  }
  return RUUVI_DRIVER_SUCCESS;
}

// TODO
ruuvi_driver_status_t ruuvi_interface_lis2dh12_dsp_set(uint8_t* dsp, uint8_t* parameter)
{
  *dsp = RUUVI_DRIVER_SENSOR_ERR_NOT_IMPLEMENTED;
  *parameter = RUUVI_DRIVER_SENSOR_ERR_NOT_IMPLEMENTED;
  return RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED;
}

// TODO
ruuvi_driver_status_t ruuvi_interface_lis2dh12_dsp_get(uint8_t* dsp, uint8_t* parameter)
{
  *dsp = RUUVI_DRIVER_SENSOR_ERR_NOT_IMPLEMENTED;
  *parameter = RUUVI_DRIVER_SENSOR_ERR_NOT_IMPLEMENTED;
  return RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_mode_set(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }
  if(RUUVI_DRIVER_SENSOR_CFG_SINGLE == *mode)
  {
    *mode = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
    return RUUVI_DRIVER_ERROR_NULL;
  }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;

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
      *mode = RUUVI_DRIVER_SENSOR_ERR_INVALID;
      return RUUVI_DRIVER_ERROR_INTERNAL;
  }
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_lis2dh12_data_get(void* data)
{
  if(NULL == data) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  axis3bit16_t raw_acceleration;
  memset(raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
  err_code |= lis2dh12_acceleration_raw_get(&(dev.ctx), raw_acceleration.u8bit);

  ruuvi_interface_acceleration_data_t* p_acceleration = (ruuvi_interface_acceleration_data_t*)data;
  float acceleration[3] = {0};

    // Compensate data with resolution, scale
  for(size_t ii = 0; ii < 3; ii++)
  {
    switch(dev.scale)
    {
      case LIS2DH12_2g:
        switch(dev.resolution)
        {
          case LIS2DH12_LP_8bit:
            acceleration[ii] = LIS2DH12_FROM_FS_2g_LP_TO_mg(raw_acceleration.i16bit[ii]);
            break;

          case LIS2DH12_NM_10bit:
            acceleration[ii] = LIS2DH12_FROM_FS_2g_NM_TO_mg(raw_acceleration.i16bit[ii]);
            break;

          case LIS2DH12_HR_12bit:
            acceleration[ii] = LIS2DH12_FROM_FS_2g_HR_TO_mg(raw_acceleration.i16bit[ii]);
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
            acceleration[ii] = LIS2DH12_FROM_FS_4g_LP_TO_mg(raw_acceleration.i16bit[ii]);
            break;

          case LIS2DH12_NM_10bit:
            acceleration[ii] = LIS2DH12_FROM_FS_4g_NM_TO_mg(raw_acceleration.i16bit[ii]);
            break;

          case LIS2DH12_HR_12bit:
            acceleration[ii] = LIS2DH12_FROM_FS_4g_HR_TO_mg(raw_acceleration.i16bit[ii]);
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
            acceleration[ii] = LIS2DH12_FROM_FS_8g_LP_TO_mg(raw_acceleration.i16bit[ii]);
            break;

          case LIS2DH12_NM_10bit:
            acceleration[ii] = LIS2DH12_FROM_FS_8g_NM_TO_mg(raw_acceleration.i16bit[ii]);
            break;

          case LIS2DH12_HR_12bit:
            acceleration[ii] = LIS2DH12_FROM_FS_8g_HR_TO_mg(raw_acceleration.i16bit[ii]);
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
            acceleration[ii] = LIS2DH12_FROM_FS_16g_LP_TO_mg(raw_acceleration.i16bit[ii]);
            break;

          case LIS2DH12_NM_10bit:
            acceleration[ii] = LIS2DH12_FROM_FS_16g_NM_TO_mg(raw_acceleration.i16bit[ii]);
            break;

          case LIS2DH12_HR_12bit:
            acceleration[ii] = LIS2DH12_FROM_FS_16g_HR_TO_mg(raw_acceleration.i16bit[ii]);
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
  p_acceleration->timestamp_ms = RUUVI_DRIVER_UINT64_INVALID;
  p_acceleration->x_mg = acceleration[0];
  p_acceleration->y_mg = acceleration[1];
  p_acceleration->z_mg = acceleration[2];
  return err_code;
}

#endif