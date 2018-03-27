/**
 * BMG250 interface.
 * Requires Bosch BMG250-API, available under BSD-3 on GitHub.
 * Requires "application_config.h", will only get compiled if BMG250_GYRATION is defined
 * Requires "boards.h" for slave select pin and sensor presense definition
 */

#include "application_config.h" //TODO: write default header on driver repository
#include "boards.h"
#if BMG250_GYRATION

// Ruuvi headers
#include "ruuvi_error.h"
#include "ruuvi_sensor.h"
#include "gyration.h"
#include "bmg250_interface.h"

// Bosch driver.
#include "bmg250.h"
#include "bmg250_defs.h"

// Platform functions
#include "spi.h"
#include "yield.h"

#define PLATFORM_LOG_MODULE_NAME bmg250_iface
#if BMG250_INTERFACE_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       BMG250_INTERFACE_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  BMG250_INTERFACE_INFO_COLOR
#else // ANT_BPWR_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       0
#endif // ANT_BPWR_LOG_ENABLED
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

/** State variables **/
static struct bmg250_dev gyro = {0};

//XXX
ruuvi_status_t bmg250_interface_init(ruuvi_sensor_t* gyration_sensor)
{
  int8_t result = BMG250_OK;
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  gyro.dev_id = SPIM0_SS_GYROSCOPE_PIN;
  gyro.interface = BMG250_SPI_INTF;
  gyro.read = spi_bosch_platform_read;
  gyro.write = spi_bosch_platform_write;
  gyro.delay_ms = platform_delay_ms;

  uint8_t num_retries = 0;
  do {
    result = bmg250_init(&gyro);
    PLATFORM_LOG_DEBUG("BMG init status: %d", (uint32_t)result);
  } while (result && num_retries++ < 10);
  if (BMG250_OK != result) { err_code |= RUUVI_ERROR_NOT_FOUND; PLATFORM_LOG_ERROR("Gyro not found, %d", result);}
  result = bmg250_soft_reset(&gyro);

  /* Structure to set the gyro config */
  struct bmg250_cfg gyro_cfg;

  /* Setting the power mode as normal */
  gyro.power_mode = BMG250_GYRO_NORMAL_MODE;
  result |= bmg250_set_power_mode(&gyro);

  /* Read the set configuration from the sensor */
  result |= bmg250_get_sensor_settings(&gyro_cfg, &gyro);

  /* Selecting the ODR as 100Hz */
  gyro_cfg.odr = BMG250_ODR_100HZ;

  /* Selecting the bw as Normal mode */
  gyro_cfg.bw = BMG250_BW_NORMAL_MODE;

  /* Selecting the range as 2000 Degrees/second */
  gyro_cfg.range = BMG250_RANGE_2000_DPS;

  /* Set the above selected ODR, Range, BW in the sensor */
  result |= bmg250_set_sensor_settings(&gyro_cfg, &gyro);
  if (BMG250_OK != result) { err_code |= RUUVI_ERROR_INTERNAL; PLATFORM_LOG_ERROR("Failed to setup gyro"); }

  // Run self-test
  result = bmg250_perform_self_test(&gyro);
  if (BMG250_OK != result) { err_code |= RUUVI_ERROR_SELFTEST; PLATFORM_LOG_ERROR("Gyro selftest failed");}

  // Compensate offset
  result = bmg250_set_foc(&gyro);
  if (BMG250_OK != result) { err_code |= RUUVI_ERROR_INTERNAL; PLATFORM_LOG_ERROR("Failed to compensate gyro");}

  if (RUUVI_SUCCESS == result)
  {
    gyration_sensor->init           = bmg250_interface_init;
    gyration_sensor->uninit         = bmg250_interface_uninit;
    gyration_sensor->samplerate_set = bmg250_interface_samplerate_set;
    gyration_sensor->samplerate_get = bmg250_interface_samplerate_get;
    gyration_sensor->resolution_set = bmg250_interface_resolution_set;
    gyration_sensor->resolution_get = bmg250_interface_resolution_get;
    gyration_sensor->scale_set      = bmg250_interface_scale_set;
    gyration_sensor->scale_get      = bmg250_interface_scale_get;
    gyration_sensor->dsp_set        = bmg250_interface_dsp_set;
    gyration_sensor->dsp_get        = bmg250_interface_dsp_get;
    gyration_sensor->mode_set       = bmg250_interface_mode_set;
    gyration_sensor->mode_get       = bmg250_interface_mode_get;
    gyration_sensor->interrupt_set  = bmg250_interface_interrupt_set;
    gyration_sensor->interrupt_get  = bmg250_interface_interrupt_get;
    gyration_sensor->data_get       = bmg250_interface_data_get;
  }
  return err_code;
}

ruuvi_status_t bmg250_interface_uninit(ruuvi_sensor_t* gyration_sensor)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_samplerate_set(ruuvi_sensor_samplerate_t* samplerate)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_samplerate_get(ruuvi_sensor_samplerate_t* samplerate)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_resolution_set(ruuvi_sensor_resolution_t* resolution)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_resolution_get(ruuvi_sensor_resolution_t* resolution)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_scale_set(ruuvi_sensor_scale_t* scale)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_scale_get(ruuvi_sensor_scale_t* scale)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_dsp_set(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_dsp_get(ruuvi_sensor_dsp_function_t* dsp, uint8_t* parameter)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_mode_set(ruuvi_sensor_mode_t* mode)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_mode_get(ruuvi_sensor_mode_t* mode)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_interrupt_set(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_interrupt_get(uint8_t number, float* threshold, ruuvi_sensor_trigger_t* trigger, ruuvi_sensor_dsp_function_t* dsp)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t bmg250_interface_data_get(void* data)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}


#endif