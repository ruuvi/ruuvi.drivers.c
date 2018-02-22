/**
 * LIS2DH12 interface. 
 * Requires STM lis2dh12 driver, available on GitHub under unknown locense.
 * Requires "application_config.h", will only get compiled if LIS2DH12_ACCELERATION is defined
 * Requires "boards.h" for slave select pin
 * Requires floats enabled in application
 */

#include "application_config.h" //TODO: write default header on driver repository
#ifdef LIS2DH12_ACCELERATION
#include "boards.h"
#include "ruuvi_error.h"
#include "ruuvi_sensor.h"
#include "lis2dh12_interface.h"
#include "yield.h"
#include "spi.h"

#include "lis2dh12_reg.h"

#include <stdlib.h>
#include <string.h>

// TODO: Platform log
#include "nrf_log.h" 
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

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
    NRF_LOG_INFO("Self-test diff: %d", diff);
    if(diff < 17)  { return RUUVI_ERROR_SELFTEST; }
    if(diff > 360) { return RUUVI_ERROR_SELFTEST; } 
  }
  return RUUVI_SUCCESS;
}

ruuvi_status_t lis2dh12_interface_init(void)
{
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  // Initialize mems driver interface
  lis2dh12_ctx_t* dev_ctx = &(dev.ctx);
  dev_ctx->write_reg = spi_stm_platform_write;
  dev_ctx->read_reg = spi_stm_platform_read;
  dev_ctx->handle = &lis2dh12_ss_pin;  

  // Check device ID
  uint8_t whoamI = 0;
  lis2dh12_device_id_get(dev_ctx, &whoamI);
  if ( whoamI != LIS2DH12_ID ) { return RUUVI_ERROR_NOT_FOUND; }
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
  NRF_LOG_INFO("Read acceleration");

  // self-test to positive direction
  dev.selftest = LIS2DH12_ST_POSITIVE;
  lis2dh12_self_test_set(dev_ctx, dev.selftest);

  // wait 2 samples - LP, normal mode
  platform_delay_ms(9);

  // Check self-test result
  lis2dh12_acceleration_raw_get(dev_ctx, data_raw_acceleration_new.u8bit);
  NRF_LOG_INFO("Read acceleration");
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
  
  return err_code;
}

/*
 * Lis2dh12 does not have a proper softreset (boot does not reset all registers)
 * Therefore just stop sampling
 */
ruuvi_status_t lis2dh12_interface_uninit(void)
{
  dev.samplerate = LIS2DH12_POWER_DOWN;
  //LIS2DH12 function returns SPI write result which is ruuvi_status_t
  return lis2dh12_data_rate_set(&(dev.ctx), dev.samplerate);
}

// ruuvi_status_t lis2dh12_interface_samplerate_set(ruuvi_sensor_samplerate_t* samplerate);
// ruuvi_status_t lis2dh12_interface_samplerate_get(ruuvi_sensor_samplerate_t* samplerate);
// ruuvi_status_t lis2dh12_interface_resolution_set(ruuvi_sensor_resolution_t* resolution);
// ruuvi_status_t lis2dh12_interface_resolution_get(ruuvi_sensor_resolution_t* resolution);
// ruuvi_status_t lis2dh12_interface_scale_set(ruuvi_sensor_scale_t* scale);
// ruuvi_status_t lis2dh12_interface_scale_get(ruuvi_sensor_scale_t* scale);
// ruuvi_status_t lis2dh12_interface_dsp_set(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter);
// ruuvi_status_t lis2dh12_interface_dsp_get(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter);
// ruuvi_status_t lis2dh12_interface_mode_set(ruuvi_sensor_mode_t*);
// ruuvi_status_t lis2dh12_interface_mode_get(ruuvi_sensor_mode_t*);
// ruuvi_status_t lis2dh12_interface_interrupt_set(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp);
// ruuvi_status_t lis2dh12_interface_interrupt_get(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp);
// ruuvi_status_t lis2dh12_interface_data_get(void* data);

#endif