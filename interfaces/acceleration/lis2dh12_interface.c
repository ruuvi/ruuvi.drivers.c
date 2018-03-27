/**
 * LIS2DH12 interface. 
 * Requires STM lis2dh12 driver, available on GitHub under unknown locense.
 * Requires "application_config.h", will only get compiled if LIS2DH12_ACCELERATION is defined
 * Requires "boards.h" for slave select pin
 * Requires floats enabled in application
 */

#include "application_config.h" //TODO: write default header on driver repository
#if LIS2DH12_ACCELERATION
#include "boards.h"
#include "acceleration.h"
#include "ruuvi_error.h"
#include "ruuvi_sensor.h"
#include "lis2dh12_interface.h"
#include "yield.h"
#include "spi.h"

#include "lis2dh12_reg.h"

#include <stdlib.h>
#include <string.h>

#define PLATFORM_LOG_MODULE_NAME lis2dh12_iface
#if LIS2DH12_INTERFACE_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       LIS2DH12_INTERFACE_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  LIS2DH12_INTERFACE_INFO_COLOR
#else // ANT_BPWR_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       0
#endif // ANT_BPWR_LOG_ENABLED
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

#ifndef APPLICATION_FLOAT_USE
  #error "LIS2DH12 interface requires floats, define APPLICATION_FLOAT_USE in makefile"
#endif

static uint8_t lis2dh12_ss_pin = SPIM0_SS_ACCELERATION_PIN;

/*!
 * @brief lis2dh12 sensor settings structure.
 */
typedef struct {
  /*! resolution */
  lis2dh12_op_md_t resolution;

  /*! scale */
  lis2dh12_fs_t scale;

  /*! data rate */
  lis2dh12_odr_t samplerate;

  /*! self-test */
  lis2dh12_st_t selftest;

  /*! operating mode */
  ruuvi_sensor_mode_t mode;

  /*! device control structure */
  lis2dh12_ctx_t ctx;
}lis2dh12;

static lis2dh12 dev;

// Check that self-test values differ enough
static ruuvi_status_t lis2dh12_verify_selftest_difference(axis3bit16_t* new, axis3bit16_t* old)
{
  if(LIS2DH12_2g != dev.scale || LIS2DH12_NM_10bit != dev.resolution) { return RUUVI_ERROR_INVALID_STATE; }

  // Calculate positive diffs of each axes and compare to expected change
  for(size_t ii = 0; ii < 3; ii++)
  {
    int16_t diff = new->i16bit[ii] - old->i16bit[ii];
    //Compensate justification
    diff >>=6;
    if(0 > diff) { diff = 0 - diff; }
    PLATFORM_LOG_DEBUG("Self-test diff: %d", diff);
    if(diff < 17)  { return RUUVI_ERROR_SELFTEST; }
    if(diff > 360) { return RUUVI_ERROR_SELFTEST; } 
  }
  return RUUVI_SUCCESS;
}

ruuvi_status_t lis2dh12_interface_init(ruuvi_sensor_t* acceleration_sensor)
{
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  // Initialize mems driver interface
  lis2dh12_ctx_t* dev_ctx = &(dev.ctx);
  dev_ctx->write_reg = spi_lis2dh12_platform_write;
  dev_ctx->read_reg = spi_lis2dh12_platform_read;
  dev_ctx->handle = &lis2dh12_ss_pin;  
  dev.mode = RUUVI_SENSOR_MODE_SLEEP;

  // Check device ID
  uint8_t whoamI = 0;
  PLATFORM_LOG_DEBUG("Getting WHOAMI");
  lis2dh12_device_id_get(dev_ctx, &whoamI);
  PLATFORM_LOG_DEBUG("Checking WHOAMI");
  if ( whoamI != LIS2DH12_ID ) { PLATFORM_LOG_ERROR("WHOAMI fail"); return RUUVI_ERROR_NOT_FOUND; }
  PLATFORM_LOG_DEBUG("WHOAMI Ok");
  uint8_t enable_axes = 0x07;
  lis2dh12_write_reg(dev_ctx, LIS2DH12_CTRL_REG1, &enable_axes, 1);

  // Disable Block Data Update, allow values to update even if old is not read
  lis2dh12_block_data_update_set(dev_ctx, PROPERTY_DISABLE);

    // Set Output Data Rate
  dev.samplerate = LIS2DH12_ODR_400Hz;
  lis2dh12_data_rate_set(dev_ctx, dev.samplerate);

  // Set full scale
  dev.scale = LIS2DH12_2g;
  lis2dh12_full_scale_set(dev_ctx, dev.scale);

  // (Don't) Enable temperature sensor
  //lis2dh12_temperature_meas_set(&dev_ctx, LIS2DH12_TEMP_ENABLE);

  // Set device in 10 bit mode
  dev.resolution = LIS2DH12_NM_10bit;
  lis2dh12_operating_mode_set(dev_ctx, dev.resolution);



  // Run self-test
  // wait for sample to be available
  platform_delay_ms(3);

  // read accelerometer
  axis3bit16_t data_raw_acceleration_old;
  axis3bit16_t data_raw_acceleration_new;
  memset(data_raw_acceleration_old.u8bit, 0x00, 3*sizeof(int16_t));
  memset(data_raw_acceleration_new.u8bit, 0x00, 3*sizeof(int16_t));
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_old.u8bit);
  PLATFORM_LOG_DEBUG("Read acceleration");

  // self-test to positive direction
  dev.selftest = LIS2DH12_ST_POSITIVE;
  lis2dh12_self_test_set(dev_ctx, dev.selftest);

  // wait 2 samples - LP, normal mode
  platform_delay_ms(9);

  // Check self-test result
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  PLATFORM_LOG_DEBUG("Read acceleration");
  err_code |= lis2dh12_verify_selftest_difference(&data_raw_acceleration_new, &data_raw_acceleration_old);

  // self-test to negative direction
  dev.selftest = LIS2DH12_ST_NEGATIVE;
  lis2dh12_self_test_set(dev_ctx, dev.selftest);

  // wait 2 samples
  platform_delay_ms(9);

  // Check self-test result
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  err_code |= lis2dh12_verify_selftest_difference(&data_raw_acceleration_new, &data_raw_acceleration_old);
  
  // turn self-test off, keep error code in case we "lose" sensor after self-test
  dev.selftest = LIS2DH12_ST_DISABLE;
  err_code |= lis2dh12_self_test_set(dev_ctx, dev.selftest);

  // Turn accelerometer off
  dev.samplerate = LIS2DH12_POWER_DOWN;
  err_code |= lis2dh12_data_rate_set(dev_ctx, dev.samplerate);

  if(RUUVI_SUCCESS == err_code)
  {
    acceleration_sensor->init           = lis2dh12_interface_init;
    acceleration_sensor->uninit         = lis2dh12_interface_uninit;
    acceleration_sensor->samplerate_set = lis2dh12_interface_samplerate_set;
    acceleration_sensor->samplerate_get = lis2dh12_interface_samplerate_get;
    acceleration_sensor->resolution_set = lis2dh12_interface_resolution_set;
    acceleration_sensor->resolution_get = lis2dh12_interface_resolution_get;
    acceleration_sensor->scale_set      = lis2dh12_interface_scale_set;
    acceleration_sensor->scale_get      = lis2dh12_interface_scale_get;
    acceleration_sensor->dsp_set        = lis2dh12_interface_dsp_set;
    acceleration_sensor->dsp_get        = lis2dh12_interface_dsp_get;
    acceleration_sensor->mode_set       = lis2dh12_interface_mode_set;
    acceleration_sensor->mode_get       = lis2dh12_interface_mode_get;
    acceleration_sensor->interrupt_set  = lis2dh12_interface_interrupt_set;
    acceleration_sensor->interrupt_get  = lis2dh12_interface_interrupt_get;
    acceleration_sensor->data_get       = lis2dh12_interface_data_get;
 }
  
  return err_code;
}

/*
 * Lis2dh12 does not have a proper softreset (boot does not reset all registers)
 * Therefore just stop sampling
 */
ruuvi_status_t lis2dh12_interface_uninit(ruuvi_sensor_t* sensor)
{
  dev.samplerate = LIS2DH12_POWER_DOWN;
  //LIS2DH12 function returns SPI write result which is ruuvi_status_t
  return lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
}

/**
 * Set up samplerate. Powers down sensor on SAMPLERATE_STOP, writes value to 
 * lis2dh12 only if mode is continous as writing samplerate to sensor starts sampling.
 * MAX is 400 Hz as it is valid for all power settings
 * Samplerate is rounded up, i.e. "Please give me at least samplerate F.", 5 is rounded to 10 Hz etc.
 */
ruuvi_status_t lis2dh12_interface_samplerate_set(ruuvi_sensor_samplerate_t* samplerate)
{
  if(NULL == samplerate)                                { return RUUVI_ERROR_NULL; }
  if(RUUVI_SENSOR_SAMPLERATE_SINGLE == *samplerate)     { return RUUVI_ERROR_NOT_SUPPORTED; } //HW does not support.
  if(RUUVI_SENSOR_SAMPLERATE_NO_CHANGE == *samplerate)  { return RUUVI_SUCCESS; }
  ruuvi_status_t err_code = RUUVI_SUCCESS;

  if(RUUVI_SENSOR_SAMPLERATE_STOP == *samplerate)     { dev.samplerate = LIS2DH12_POWER_DOWN; }
  else if(RUUVI_SENSOR_SAMPLERATE_MIN == *samplerate) { dev.samplerate = LIS2DH12_ODR_1Hz;    }
  else if(RUUVI_SENSOR_SAMPLERATE_MAX == *samplerate) { dev.samplerate = LIS2DH12_ODR_400Hz;  }
  else if(1   == *samplerate)                         { dev.samplerate = LIS2DH12_ODR_1Hz;    }
  else if(10  <= *samplerate)                         { dev.samplerate = LIS2DH12_ODR_10Hz;   }
  else if(25  <= *samplerate)                         { dev.samplerate = LIS2DH12_ODR_25Hz;   }
  else if(50  <= *samplerate)                         { dev.samplerate = LIS2DH12_ODR_50Hz;   }
  else if(100 <= *samplerate)                         { dev.samplerate = LIS2DH12_ODR_100Hz;  }
  else if(200 <= *samplerate)                         { dev.samplerate = LIS2DH12_ODR_200Hz;  }
  else { return RUUVI_ERROR_NOT_SUPPORTED; }

  // Write samplerate to lis if we're in continous mode or if sample rate is 0.
  if(dev.mode == RUUVI_SENSOR_MODE_CONTINOUS
    || dev.samplerate == LIS2DH12_POWER_DOWN)
  {
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
  }
return err_code;
}

/*
 *. Read samplerate to pointer
 */
ruuvi_status_t lis2dh12_interface_samplerate_get(ruuvi_sensor_samplerate_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_ERROR_NULL; }

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
    *samplerate = RUUVI_SENSOR_SAMPLERATE_MAX;
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
ruuvi_status_t lis2dh12_interface_resolution_set(ruuvi_sensor_resolution_t* resolution)
{
  if(NULL == resolution)                               { return RUUVI_ERROR_NULL; }
  if(RUUVI_SENSOR_RESOLUTION_NO_CHANGE == *resolution) { return RUUVI_SUCCESS; }
  
  if     (RUUVI_SENSOR_RESOLUTION_MIN == *resolution) { dev.resolution = LIS2DH12_LP_8bit;  }
  else if(RUUVI_SENSOR_RESOLUTION_MAX == *resolution) { dev.resolution = LIS2DH12_HR_12bit; }
  else if(8 <= *resolution )  { dev.resolution = LIS2DH12_LP_8bit; }
  else if(10 <= *resolution ) { dev.resolution = LIS2DH12_NM_10bit; }
  else if(12 <= *resolution ) { dev.resolution = LIS2DH12_HR_12bit; }
  else { return RUUVI_ERROR_NOT_SUPPORTED; }

  return lis2dh12_operating_mode_set(&(dev.ctx), dev.resolution);
}
ruuvi_status_t lis2dh12_interface_resolution_get(ruuvi_sensor_resolution_t* resolution)
{
  if(NULL == resolution) { return RUUVI_ERROR_NULL; }

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
    return RUUVI_ERROR_INTERNAL;
    break;
  }
  return RUUVI_SUCCESS;
}

/**
 * Setup lis2dh12 scale. Scale is rounded up, i.e. "at least tjis much"
 */
ruuvi_status_t lis2dh12_interface_scale_set(ruuvi_sensor_scale_t* scale)
{
  if(NULL == scale)                          { return RUUVI_ERROR_NULL; }
  if(RUUVI_SENSOR_SCALE_NO_CHANGE == *scale) { return RUUVI_SUCCESS;    }

  if     (RUUVI_SENSOR_SCALE_MIN == *scale)  { dev.scale = LIS2DH12_2g;  }
  else if(RUUVI_SENSOR_SCALE_MAX == *scale)  { dev.scale = LIS2DH12_16g; }
  else if(2  <= *scale)                      { dev.scale = LIS2DH12_2g;  } 
  else if(4  <= *scale)                      { dev.scale = LIS2DH12_4g;  } 
  else if(8  <= *scale)                      { dev.scale = LIS2DH12_8g;  } 
  else if(16 <= *scale)                      { dev.scale = LIS2DH12_16g; } 
  else                                       { return RUUVI_ERROR_NOT_SUPPORTED; }

  return lis2dh12_full_scale_set(&(dev.ctx), dev.scale);
}

ruuvi_status_t lis2dh12_interface_scale_get(ruuvi_sensor_scale_t* scale)
{
  if(NULL == scale) { return RUUVI_ERROR_NULL; }

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
    return RUUVI_ERROR_INTERNAL;
  }
  return RUUVI_SUCCESS;
}


ruuvi_status_t lis2dh12_interface_dsp_set(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t lis2dh12_interface_dsp_get(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t lis2dh12_interface_mode_set(ruuvi_sensor_mode_t* mode)
{
  if(NULL == mode) { return RUUVI_ERROR_NULL; }
  if(RUUVI_SENSOR_MODE_SINGLE_ASYNCHRONOUS == *mode) { return RUUVI_ERROR_NOT_SUPPORTED; }
  if(RUUVI_SENSOR_MODE_SINGLE_BLOCKING     == *mode) { return RUUVI_ERROR_NOT_SUPPORTED; }

  ruuvi_status_t err_code = RUUVI_SUCCESS;

  // Do not store power down mode to dev strucrture, so we can continue at previous data rate 
  // when mode is set to continous.
  if(RUUVI_SENSOR_MODE_SLEEP == *mode) 
  { 
    dev.mode = *mode;
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), LIS2DH12_POWER_DOWN);
  }
  else if(RUUVI_SENSOR_MODE_CONTINOUS == *mode) 
  { 
    dev.mode = *mode;
    err_code |= lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
  }
  else { err_code |= RUUVI_ERROR_INVALID_PARAM; }
  return err_code;
}

ruuvi_status_t lis2dh12_interface_mode_get(ruuvi_sensor_mode_t* mode)
{
  if(NULL == mode) { return RUUVI_ERROR_NULL; }

  switch(dev.mode)  
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

ruuvi_status_t lis2dh12_interface_interrupt_set(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}
ruuvi_status_t lis2dh12_interface_interrupt_get(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t lis2dh12_interface_data_get(void* data)
{
  if(NULL == data) { return RUUVI_ERROR_NULL; }
  PLATFORM_LOG_DEBUG("Getting data");

  ruuvi_status_t err_code = RUUVI_SUCCESS;
  axis3bit16_t raw_acceleration;
  memset(raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
  err_code |= lis2dh12_acceleration_raw_get(&(dev.ctx), raw_acceleration.u8bit);

  ruuvi_acceleration_data_t* p_acceleration = (ruuvi_acceleration_data_t*)data;
  float acceleration[3] = {0};
  PLATFORM_LOG_DEBUG("SPI Read");

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
        acceleration[ii] = ACCELERATION_INVALID;
        err_code |= RUUVI_ERROR_INTERNAL;
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
        acceleration[ii] = ACCELERATION_INVALID;
        err_code |= RUUVI_ERROR_INTERNAL;
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
        acceleration[ii] = ACCELERATION_INVALID;
        err_code |= RUUVI_ERROR_INTERNAL;
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