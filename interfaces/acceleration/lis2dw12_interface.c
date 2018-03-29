/**
 * LIS2DW12 interface. 
 * Requires STM lis2dw12 driver, available on GitHub under unknown locense.
 * Requires "application_config.h", will only get compiled if LIS2DW12_ACCELERATION is defined
 * Requires "boards.h" for slave select pin
 * Requires floats enabled in application
 */

#include "application_config.h" //TODO: write default header on driver repository
#include "boards.h"
#if LIS2DW12_ACCELERATION
#include "acceleration.h"
#include "ruuvi_error.h"
#include "ruuvi_sensor.h"
#include "lis2dw12_interface.h"
#include "yield.h"
#include "spi.h"

#include "lis2dw12_reg.h"

#include <stdlib.h>
#include <string.h>

#define PLATFORM_LOG_MODULE_NAME lis2dw12_iface
#if LIS2DW12_INTERFACE_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       LIS2DW12_INTERFACE_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  LIS2DW12_INTERFACE_INFO_COLOR
#else 
#define PLATFORM_LOG_LEVEL       0
#endif 
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

#ifndef APPLICATION_FLOAT_USE
  #error "LIS2DW12 interface requires floats, define APPLICATION_FLOAT_USE in makefile cflags"
#endif

static uint8_t lis2dw12_ss_pin = SPIM0_SS_ACCELERATION_PIN;

/*!
 * @brief lis2dh12 sensor settings structure.
 */
typedef struct {
  /*! resolution */
  lis2dw12_mode_t mode;

  /*! scale */
  lis2dw12_fs_t scale;

  /*! data rate */
  lis2dw12_odr_t samplerate;

  /*! self-test */
  lis2dw12_st_t selftest;

  /*! device control structure */
  lis2dw12_ctx_t ctx;

  ruuvi_sensor_mode_t opmode;
}lis2dw12;

static lis2dw12 dev;

// Check that self-test values differ enough
static ruuvi_status_t lis2dw12_verify_selftest_difference(axis3bit16_t* new, axis3bit16_t* old, bool negative)
{
  if(LIS2DW12_2g != dev.scale || LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4 != dev.mode) { return RUUVI_ERROR_INVALID_STATE; }

  // Calculate positive diffs of each axes and compare to expected change
  for(size_t ii = 0; ii < 3; ii++)
  {
    float diff = LIS2DW12_FROM_FS_2g_TO_mg((new->i16bit[ii] - old->i16bit[ii]));

    if(negative) { diff = 0 - diff; }

    PLATFORM_LOG_INFO("Self-test diff: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(diff));
    if(diff < 70.0)   { return RUUVI_ERROR_SELFTEST; }
    if(diff > 1500.0) { return RUUVI_ERROR_SELFTEST; } 
  }
  return RUUVI_SUCCESS;
}

ruuvi_status_t lis2dw12_interface_init(ruuvi_sensor_t* acceleration_sensor)
{
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  // Initialize mems driver interface
  lis2dw12_ctx_t* dev_ctx = &(dev.ctx);
  dev_ctx->write_reg = spi_lis2dw12_platform_write;
  dev_ctx->read_reg = spi_lis2dw12_platform_read;
  dev_ctx->handle = &lis2dw12_ss_pin;
  dev.opmode = RUUVI_SENSOR_MODE_SLEEP;

  // Check device ID
  uint8_t whoamI = 0;
  PLATFORM_LOG_INFO("Getting WHOAMI");
  err_code |= lis2dw12_device_id_get(dev_ctx, &whoamI);
  PLATFORM_LOG_INFO("Checking WHOAMI");
  if ( whoamI != LIS2DW12_ID ) { PLATFORM_LOG_ERROR("WHOAMI fail"); return RUUVI_ERROR_NOT_FOUND; }
  PLATFORM_LOG_INFO("WHOAMI Ok");
  // uint8_t enable_axes = 0x07;
  // lis2dh12_write_reg(dev_ctx, LIS2DH12_CTRL_REG1, &enable_axes, 1);

  // Disable Block Data Update, allow values to update even if old is not read
  PLATFORM_LOG_INFO("Status before setting up %d", err_code);
  err_code |= lis2dw12_block_data_update_set(dev_ctx, PROPERTY_DISABLE);
  PLATFORM_LOG_INFO("Status after data block setup %d", err_code);

    // Set Output Data Rate
  dev.samplerate = LIS2DW12_XL_ODR_200Hz;
  err_code |= lis2dw12_data_rate_set(dev_ctx, dev.samplerate);
  PLATFORM_LOG_INFO("Status after data rate %d", err_code);

  // Set full scale
  dev.scale = LIS2DW12_2g;
  err_code |= lis2dw12_full_scale_set(dev_ctx, dev.scale);
  PLATFORM_LOG_INFO("Status after scale %d", err_code);

  // (Don't) Enable temperature sensor
  //lis2dh12_temperature_meas_set(&dev_ctx, LIS2DH12_TEMP_ENABLE);

  // Set device in 14bit, low-noise mode 4
  dev.mode = LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4;
  err_code |= lis2dw12_power_mode_set(dev_ctx, dev.mode);
  PLATFORM_LOG_INFO("Status before self-test %d", err_code);



  // Run self-test
  // wait for sample to be available
  platform_delay_ms(6);

  // read accelerometer
  axis3bit16_t data_raw_acceleration_old;
  axis3bit16_t data_raw_acceleration_new;
  memset(data_raw_acceleration_old.u8bit, 0x00, 3*sizeof(int16_t));
  memset(data_raw_acceleration_new.u8bit, 0x00, 3*sizeof(int16_t));
  lis2dw12_acceleration_raw_get(dev_ctx, data_raw_acceleration_old.u8bit);
  PLATFORM_LOG_DEBUG("Read acceleration");

  // self-test to positive direction
  dev.selftest = LIS2DW12_XL_ST_POSITIVE;
  lis2dw12_self_test_set(dev_ctx, dev.selftest);

  // wait 2 samples
  platform_delay_ms(11);

  // Check self-test result
  lis2dw12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  PLATFORM_LOG_DEBUG("Read acceleration");
  err_code |= lis2dw12_verify_selftest_difference(&data_raw_acceleration_new, &data_raw_acceleration_old, false);

  // self-test to negative direction
  dev.selftest = LIS2DW12_XL_ST_NEGATIVE;
  lis2dw12_self_test_set(dev_ctx, dev.selftest);

  // wait 2 samples
  platform_delay_ms(11);

  // Check self-test result
  lis2dw12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  err_code |= lis2dw12_verify_selftest_difference(&data_raw_acceleration_new, &data_raw_acceleration_old, true);
  
  // turn self-test off, keep error code in case we "lose" sensor after self-test
  dev.selftest = LIS2DW12_XL_ST_DISABLE;
  err_code |= lis2dw12_self_test_set(dev_ctx, dev.selftest);
  PLATFORM_LOG_INFO("Status after self-test %d", err_code);

  // Turn accelerometer off
  dev.samplerate = LIS2DW12_XL_ODR_OFF;
  err_code |= lis2dw12_data_rate_set(dev_ctx, dev.samplerate);

  if(RUUVI_SUCCESS == err_code)
  {
    acceleration_sensor->init           = lis2dw12_interface_init;
    acceleration_sensor->uninit         = lis2dw12_interface_uninit;
    acceleration_sensor->samplerate_set = lis2dw12_interface_samplerate_set;
    acceleration_sensor->samplerate_get = lis2dw12_interface_samplerate_get;
    acceleration_sensor->resolution_set = lis2dw12_interface_resolution_set;
    acceleration_sensor->resolution_get = lis2dw12_interface_resolution_get;
    acceleration_sensor->scale_set      = lis2dw12_interface_scale_set;
    acceleration_sensor->scale_get      = lis2dw12_interface_scale_get;
    acceleration_sensor->dsp_set        = lis2dw12_interface_dsp_set;
    acceleration_sensor->dsp_get        = lis2dw12_interface_dsp_get;
    acceleration_sensor->mode_set       = lis2dw12_interface_mode_set;
    acceleration_sensor->mode_get       = lis2dw12_interface_mode_get;
    acceleration_sensor->interrupt_set  = lis2dw12_interface_interrupt_set;
    acceleration_sensor->interrupt_get  = lis2dw12_interface_interrupt_get;
    acceleration_sensor->data_get       = lis2dw12_interface_data_get;
 }
  
  return err_code;
}

/*
 * Lis2dh12 does not have a proper softreset (boot does not reset all registers)
 * Therefore just stop sampling
 */
ruuvi_status_t lis2dw12_interface_uninit(ruuvi_sensor_t* sensor)
{
  dev.samplerate = LIS2DW12_XL_ODR_OFF;
  //LIS2DH12 function returns SPI write result which is ruuvi_status_t
  return lis2dw12_data_rate_set(&(dev.ctx), dev.samplerate);
}

/**
 * Set up samplerate. Powers down sensor on SAMPLERATE_STOP, writes value to 
 * lis2dw12 only if mode is continous as writing samplerate to sensor starts sampling.
 * MAX is 200 Hz as it is valid for all power settings
 * Samplerate is rounded up, i.e. "Please give me at least samplerate F.", 5 is rounded to 10 Hz etc.
 */
ruuvi_status_t lis2dw12_interface_samplerate_set(ruuvi_sensor_samplerate_t* samplerate)
{
  if(NULL == samplerate)                                { return RUUVI_ERROR_NULL; }
  if(RUUVI_SENSOR_SAMPLERATE_NO_CHANGE == *samplerate)  { return RUUVI_SUCCESS; }
  ruuvi_status_t err_code = RUUVI_SUCCESS;

  if(RUUVI_SENSOR_SAMPLERATE_STOP == *samplerate)        { dev.samplerate = LIS2DW12_XL_ODR_OFF; }
  else if(RUUVI_SENSOR_SAMPLERATE_MIN == *samplerate)    { dev.samplerate = LIS2DW12_XL_ODR_1Hz6_LP_ONLY; }
  else if(RUUVI_SENSOR_SAMPLERATE_MAX == *samplerate)    { dev.samplerate = LIS2DW12_XL_ODR_200Hz; }
  else if(RUUVI_SENSOR_SAMPLERATE_SINGLE == *samplerate) { return RUUVI_ERROR_NOT_IMPLEMENTED; }
  else if(1   == *samplerate)                            { dev.samplerate = LIS2DW12_XL_ODR_1Hz6_LP_ONLY; }
  else if(12  >= *samplerate)                            { dev.samplerate = LIS2DW12_XL_ODR_12Hz5; }
  else if(25  >= *samplerate)                            { dev.samplerate = LIS2DW12_XL_ODR_25Hz;  }
  else if(50  >= *samplerate)                            { dev.samplerate = LIS2DW12_XL_ODR_50Hz;  }
  else if(100 >= *samplerate)                            { dev.samplerate = LIS2DW12_XL_ODR_100Hz; }
  else if(200 >= *samplerate)                            { dev.samplerate = LIS2DW12_XL_ODR_200Hz; }

  else { return RUUVI_ERROR_NOT_SUPPORTED; }

  // Write samplerate to lis if we're in continous mode or if sample rate is 0.
  if(RUUVI_SENSOR_MODE_CONTINOUS == dev.opmode
    || LIS2DW12_XL_ODR_OFF == dev.samplerate)
  {
    err_code |= lis2dw12_data_rate_set(&(dev.ctx), dev.samplerate);
  }
return err_code;
}

/*
 *  Read samplerate to pointer
 */
ruuvi_status_t lis2dw12_interface_samplerate_get(ruuvi_sensor_samplerate_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_ERROR_NULL; }

  switch(dev.samplerate)
  {
    case LIS2DW12_XL_ODR_OFF:
    *samplerate = 0;
    break;

    case LIS2DW12_XL_ODR_1Hz6_LP_ONLY:
    *samplerate = 1;
    break;

    case LIS2DW12_XL_ODR_12Hz5:
    *samplerate = 12;
    break;

    case LIS2DW12_XL_ODR_25Hz:
    *samplerate = 25;
    break;

    case LIS2DW12_XL_ODR_50Hz:
    *samplerate = 50;
    break;

    case LIS2DW12_XL_ODR_100Hz:
    *samplerate = 100;
    break;

    case LIS2DW12_XL_ODR_200Hz:
    *samplerate = 200;
    break;

    default:
    *samplerate = RUUVI_SENSOR_SAMPLERATE_NOT_SUPPORTED;
    break;
  }
  return RUUVI_SUCCESS;
}

/**
 * Setup resolution. Resolution is rounded up, i.e. "please give at least this many bits"
 */
ruuvi_status_t lis2dw12_interface_resolution_set(ruuvi_sensor_resolution_t* resolution)
{
  if(NULL == resolution)                               { return RUUVI_ERROR_NULL; }
  if(RUUVI_SENSOR_RESOLUTION_NO_CHANGE == *resolution) { return RUUVI_SUCCESS; }
  
  if     (RUUVI_SENSOR_RESOLUTION_MIN == *resolution) { dev.mode = LIS2DW12_CONT_LOW_PWR_12bit;  }
  else if(RUUVI_SENSOR_RESOLUTION_MAX == *resolution) { dev.mode = LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4; }
  else if(12 >= *resolution ) { dev.mode = LIS2DW12_CONT_LOW_PWR_12bit; }
  else if(14 >= *resolution ) { dev.mode = LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4; }
  else { return RUUVI_ERROR_NOT_SUPPORTED; }

  return lis2dw12_power_mode_set(&(dev.ctx), dev.mode);
}
ruuvi_status_t lis2dw12_interface_resolution_get(ruuvi_sensor_resolution_t* resolution)
{
  if(NULL == resolution) { return RUUVI_ERROR_NULL; }

  switch(dev.mode)
  {
    case LIS2DW12_CONT_LOW_PWR_12bit:
    *resolution = 12;
    break;

    case LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4:
    *resolution = 14;
    break;

    default:
    return RUUVI_ERROR_INTERNAL;
    break;
  }
  return RUUVI_SUCCESS;
}

/**
 * Setup lis2dh12 scale. Scale is rounded up, i.e. "at least tjis much"
 */
ruuvi_status_t lis2dw12_interface_scale_set(ruuvi_sensor_scale_t* scale)
{
  if(NULL == scale)                          { return RUUVI_ERROR_NULL; }
  if(RUUVI_SENSOR_SCALE_NO_CHANGE == *scale) { return RUUVI_SUCCESS;    }

  if     (RUUVI_SENSOR_SCALE_MIN == *scale)  { dev.scale = LIS2DW12_2g;  }
  else if(RUUVI_SENSOR_SCALE_MAX == *scale)  { dev.scale = LIS2DW12_16g; }
  else if(2  >= *scale)                      { dev.scale = LIS2DW12_2g;  } 
  else if(4  >= *scale)                      { dev.scale = LIS2DW12_4g;  } 
  else if(8  >= *scale)                      { dev.scale = LIS2DW12_8g;  } 
  else if(16 >= *scale)                      { dev.scale = LIS2DW12_16g; } 
  else                                       { return RUUVI_ERROR_NOT_SUPPORTED; }

  return lis2dw12_full_scale_set(&(dev.ctx), dev.scale);
}

ruuvi_status_t lis2dw12_interface_scale_get(ruuvi_sensor_scale_t* scale)
{
  if(NULL == scale) { return RUUVI_ERROR_NULL; }

  switch(dev.scale)
  {
    case LIS2DW12_2g:
    *scale = 2;
    break;

    case LIS2DW12_4g:
    *scale = 4;
    break;

    case LIS2DW12_8g:
    *scale = 8;
    break;

    case  LIS2DW12_16g:
    *scale = 16;
    break;

    default:
    return RUUVI_ERROR_INTERNAL;
  }
  return RUUVI_SUCCESS;
}


ruuvi_status_t lis2dw12_interface_dsp_set(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t lis2dw12_interface_dsp_get(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t lis2dw12_interface_mode_set(ruuvi_sensor_mode_t* mode)
{
  if(NULL == mode) { return RUUVI_ERROR_NULL; }
  if(RUUVI_SENSOR_MODE_SINGLE_ASYNCHRONOUS == *mode) { return RUUVI_ERROR_NOT_SUPPORTED; }
  if(RUUVI_SENSOR_MODE_SINGLE_BLOCKING     == *mode) { return RUUVI_ERROR_NOT_SUPPORTED; }

  ruuvi_status_t err_code = RUUVI_SUCCESS;

  // Do not store power down mode to dev strucrture, so we can continue at previous data rate 
  // when mode is set to continous.
  if(RUUVI_SENSOR_MODE_SLEEP == *mode) 
  { 
    dev.opmode = *mode;
    err_code |= lis2dw12_data_rate_set(&(dev.ctx), LIS2DW12_XL_ODR_OFF);
  }
  else if(RUUVI_SENSOR_MODE_CONTINOUS == *mode) 
  { 
    dev.opmode = *mode;
    err_code |= lis2dw12_data_rate_set(&(dev.ctx), dev.samplerate);
  }
  else { err_code |= RUUVI_ERROR_INVALID_PARAM; }
  return err_code;
}

ruuvi_status_t lis2dw12_interface_mode_get(ruuvi_sensor_mode_t* mode)
{
  if(NULL == mode) { return RUUVI_ERROR_NULL; }

  switch(dev.opmode)  
  {
    case RUUVI_SENSOR_MODE_SLEEP:
    *mode = RUUVI_SENSOR_MODE_SLEEP;
    break;

    case RUUVI_SENSOR_MODE_CONTINOUS:
    *mode = RUUVI_SENSOR_MODE_CONTINOUS;
    break;

    default:
    return RUUVI_ERROR_INTERNAL;
  }
  return RUUVI_SUCCESS;
}

ruuvi_status_t lis2dw12_interface_interrupt_set(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}
ruuvi_status_t lis2dw12_interface_interrupt_get(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t lis2dw12_interface_data_get(void* data)
{
  if(NULL == data) { return RUUVI_ERROR_NULL; }
  PLATFORM_LOG_DEBUG("Getting data");

  ruuvi_status_t err_code = RUUVI_SUCCESS;
  axis3bit16_t raw_acceleration;
  memset(raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
  err_code |= lis2dw12_acceleration_raw_get(&(dev.ctx), raw_acceleration.u8bit);

  ruuvi_acceleration_data_t* p_acceleration = (ruuvi_acceleration_data_t*)data;
  float acceleration[3] = {0};
  PLATFORM_LOG_DEBUG("SPI Read");

    // Compensate data with resolution, scale
  for(size_t ii = 0; ii < 3; ii++)
  {
    switch(dev.scale)
    {
      case LIS2DW12_2g:
      switch(dev.mode)
      {
        case LIS2DW12_CONT_LOW_PWR_12bit:
        acceleration[ii] = LIS2DW12_FROM_FS_2g_LP1_TO_mg(raw_acceleration.i16bit[ii]);
        break;
        case LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4:
        acceleration[ii] = LIS2DW12_FROM_FS_2g_TO_mg(raw_acceleration.i16bit[ii]);
        break;
        default:
        acceleration[ii] = ACCELERATION_INVALID;
        err_code |= RUUVI_ERROR_INTERNAL;
        break;
      }
      break;

      case LIS2DW12_4g:
      switch(dev.mode)
      {
        acceleration[ii] = LIS2DW12_FROM_FS_4g_LP1_TO_mg(raw_acceleration.i16bit[ii]);
        break;
        case LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4:
        acceleration[ii] = LIS2DW12_FROM_FS_4g_TO_mg(raw_acceleration.i16bit[ii]);
        break;
        default:
        acceleration[ii] = ACCELERATION_INVALID;
        err_code |= RUUVI_ERROR_INTERNAL;
        break;
      }
      break;

      case LIS2DW12_8g:
      switch(dev.mode)
      {
        acceleration[ii] = LIS2DW12_FROM_FS_8g_LP1_TO_mg(raw_acceleration.i16bit[ii]);
        break;
        case LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4:
        acceleration[ii] = LIS2DW12_FROM_FS_8g_TO_mg(raw_acceleration.i16bit[ii]);
        break;
        default:
        acceleration[ii] = ACCELERATION_INVALID;
        err_code |= RUUVI_ERROR_INTERNAL;
        break;
      }
      break;

      case LIS2DW12_16g:
      switch(dev.mode)
      {
        acceleration[ii] = LIS2DW12_FROM_FS_16g_LP1_TO_mg(raw_acceleration.i16bit[ii]);
        break;
        case LIS2DW12_CONT_LOW_PWR_LOW_NOISE_4:
        acceleration[ii] = LIS2DW12_FROM_FS_16g_TO_mg(raw_acceleration.i16bit[ii]);
        break;
        default:
        acceleration[ii] = ACCELERATION_INVALID;
        err_code |= RUUVI_ERROR_INTERNAL;
        break;
      }
      break;

      default:
      acceleration[ii] = ACCELERATION_INVALID;
      err_code |= RUUVI_ERROR_INTERNAL;
      break;
    }
  }
  p_acceleration->x_mg = acceleration[0];
  p_acceleration->y_mg = acceleration[1];
  p_acceleration->z_mg = acceleration[2];
  PLATFORM_LOG_DEBUG("Ready, err_code %d", err_code);
  return err_code;
}

#endif