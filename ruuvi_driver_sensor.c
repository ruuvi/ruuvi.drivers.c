#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include <stddef.h>

ruuvi_driver_status_t ruuvi_driver_sensor_configuration_set(const ruuvi_driver_sensor_t* sensor, ruuvi_driver_sensor_configuration_t* config)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  if(NULL == sensor || NULL == config) { return RUUVI_DRIVER_ERROR_NULL; }
  if(NULL == sensor->samplerate_set) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }
  err_code |= sensor->samplerate_set(&(config->samplerate));
  err_code |= sensor->resolution_set(&(config->resolution));
  err_code |= sensor->scale_set(&(config->scale));
  err_code |= sensor->dsp_set(&(config->dsp_function), &(config->dsp_parameter));
  err_code |= sensor->mode_set(&(config->mode));
  return err_code;
}

ruuvi_driver_status_t ruuvi_driver_sensor_configuration_get(const ruuvi_driver_sensor_t* sensor, ruuvi_driver_sensor_configuration_t* config)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  if(NULL == sensor || NULL == config) { return RUUVI_DRIVER_ERROR_NULL; }
  if(NULL == sensor->samplerate_set) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }
  err_code |= sensor->samplerate_get(&(config->samplerate));
  err_code |= sensor->resolution_get(&(config->resolution));
  err_code |= sensor->scale_get(&(config->scale));
  err_code |= sensor->dsp_get(&(config->dsp_function), &(config->dsp_parameter));
  err_code |= sensor->mode_get(&(config->mode));
  return err_code;
}