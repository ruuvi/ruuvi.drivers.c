/**
 * BMI160 interface.
 * Requires Bosch BMI160-API, available under BSD-3 on GitHub.
 * Requires "application_config.h", will only get compiled if BMI160_GYRATION is defined
 * Requires "boards.h" for slave select pin and sensor presense definition
 */

#include "application_config.h" //TODO: write default header on driver repository
#include "boards.h"
#if BMI160_GYRATION

// Ruuvi headers
#include "ruuvi_error.h"
#include "ruuvi_sensor.h"
#include "gyration.h"
#include "bmi160_gyroscope_interface.h"

// Bosch driver.
#include "bmi160.h"
#include "bmi160_defs.h"

// Platform functions
#include "spi.h"
#include "yield.h"

#define PLATFORM_LOG_MODULE_NAME bmi160_gyro_iface
#if BMI160_INTERFACE_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       BMI160_INTERFACE_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  BMI160_INTERFACE_INFO_COLOR
#else // ANT_BPWR_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       0
#endif // ANT_BPWR_LOG_ENABLED
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

/** State variables **/
static struct bmi160_dev gyro = {0};

//XXX
ruuvi_status_t bmi160_gyroscope_interface_init(ruuvi_sensor_t* gyration_sensor)
{
  int8_t result = BMI160_OK;
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  gyro.id = SPIM0_SS_GYROSCOPE_PIN;
  gyro.interface = BMI160_SPI_INTF;
  gyro.read = spi_bosch_platform_read;
  gyro.write = spi_bosch_platform_write;
  gyro.delay_ms = platform_delay_ms;

  uint8_t num_retries = 0;
  do {
    result = bmi160_init(&gyro);
    PLATFORM_LOG_DEBUG("BMI init status: %d", (uint32_t)result);
  } while (result && num_retries++ < 10);
  if (BMI160_OK != result) { err_code |= RUUVI_ERROR_NOT_FOUND; PLATFORM_LOG_ERROR("Gyro not found, %d", result);}
  result = bmi160_soft_reset(&gyro);

  /* Select the Output data rate, range of accelerometer sensor */
  gyro.accel_cfg.odr = BMI160_ACCEL_ODR_0_78HZ;
  gyro.accel_cfg.range = BMI160_ACCEL_RANGE_2G;
  gyro.accel_cfg.bw = BMI160_ACCEL_BW_NORMAL_AVG4;

  /* Select the power mode of accelerometer sensor */
  gyro.accel_cfg.power = BMI160_ACCEL_NORMAL_MODE;

  /* Select the Output data rate, range of Gyroscope sensor */
  gyro.gyro_cfg.odr = BMI160_GYRO_ODR_50HZ;
  gyro.gyro_cfg.range = BMI160_GYRO_RANGE_2000_DPS;
  gyro.gyro_cfg.bw = BMI160_GYRO_BW_NORMAL_MODE;

  /* Select the power mode of Gyroscope sensor */
  gyro.gyro_cfg.power = BMI160_GYRO_NORMAL_MODE;

  /* Set the sensor configuration */
  result = bmi160_set_sens_conf(&gyro);
  if (BMI160_OK != result) { err_code |= RUUVI_ERROR_INTERNAL; PLATFORM_LOG_ERROR("Failed to setup gyro"); }

  // Run self-test
  result = bmi160_perform_self_test(BMI160_GYRO_ONLY, &gyro);
  if (BMI160_OK != result) { err_code |= RUUVI_ERROR_SELFTEST; PLATFORM_LOG_ERROR("Gyro selftest failed");}

  // Compensate offset
  // result = bmi160_set_foc(&gyro);
  // if (BMI160_OK != result) { err_code |= RUUVI_ERROR_INTERNAL; PLATFORM_LOG_ERROR("Failed to compensate gyro");}

  if (RUUVI_SUCCESS == result)
  {
    gyration_sensor->init           = bmi160_gyroscope_interface_init;
    gyration_sensor->uninit         = bmi160_gyroscope_interface_uninit;
    gyration_sensor->samplerate_set = bmi160_gyroscope_interface_samplerate_set;
    gyration_sensor->samplerate_get = bmi160_gyroscope_interface_samplerate_get;
    gyration_sensor->resolution_set = bmi160_gyroscope_interface_resolution_set;
    gyration_sensor->resolution_get = bmi160_gyroscope_interface_resolution_get;
    gyration_sensor->scale_set      = bmi160_gyroscope_interface_scale_set;
    gyration_sensor->scale_get      = bmi160_gyroscope_interface_scale_get;
    gyration_sensor->dsp_set        = bmi160_gyroscope_interface_dsp_set;
    gyration_sensor->dsp_get        = bmi160_gyroscope_interface_dsp_get;
    gyration_sensor->mode_set       = bmi160_gyroscope_interface_mode_set;
    gyration_sensor->mode_get       = bmi160_gyroscope_interface_mode_get;
    gyration_sensor->interrupt_set  = bmi160_gyroscope_interface_interrupt_set;
    gyration_sensor->interrupt_get  = bmi160_gyroscope_interface_interrupt_get;
    gyration_sensor->data_get       = bmi160_gyroscope_interface_data_get;
  }
  return err_code;
}

ruuvi_status_t bmi160_gyroscope_interface_uninit(ruuvi_sensor_t* gyration_sensor)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_samplerate_set(ruuvi_sensor_samplerate_t* samplerate)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_samplerate_get(ruuvi_sensor_samplerate_t* samplerate)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_resolution_set(ruuvi_sensor_resolution_t* resolution)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_resolution_get(ruuvi_sensor_resolution_t* resolution)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_scale_set(ruuvi_sensor_scale_t* scale)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_scale_get(ruuvi_sensor_scale_t* scale)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_dsp_set(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_dsp_get(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_mode_set(ruuvi_sensor_mode_t* mode)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_mode_get(ruuvi_sensor_mode_t* mode)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_interrupt_set(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_interrupt_get(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmi160_gyroscope_interface_data_get(void* data)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}


#endif