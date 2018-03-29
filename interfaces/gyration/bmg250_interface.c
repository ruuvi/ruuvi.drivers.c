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
/* Structure to set the gyro config */
static struct bmg250_cfg gyro_cfg;
static ruuvi_sensor_mode_t state_power_mode = RUUVI_SENSOR_MODE_SLEEP;

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

  

  /* Setting the power mode as normal */
  gyro.power_mode = BMG250_GYRO_NORMAL_MODE;
  result |= bmg250_set_power_mode(&gyro);

  /* Read the set configuration from the sensor */
  result |= bmg250_get_sensor_settings(&gyro_cfg, &gyro);

  /* Selecting the ODR as 25Hz */
  gyro_cfg.odr = BMG250_ODR_25HZ;

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

  /* Suspend gyro */
  gyro.power_mode = BMG250_GYRO_SUSPEND_MODE;
  state_power_mode = RUUVI_SENSOR_MODE_SLEEP;
  result |= bmg250_set_power_mode(&gyro);
  if (BMG250_OK != result) { err_code |= RUUVI_ERROR_INTERNAL; PLATFORM_LOG_ERROR("Failed to suspend gyro");}

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
  if(NULL == samplerate) { return RUUVI_ERROR_NULL; }
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  int8_t result = BMG250_OK;

  // Stop byro, store desired mode
  if(RUUVI_SENSOR_SAMPLERATE_STOP   == *samplerate)         
  { 
    uint8_t pwr = gyro.power_mode;
    gyro.power_mode = BMG250_GYRO_SUSPEND_MODE;
    result |= bmg250_set_power_mode(&gyro);
    gyro.power_mode = pwr;
  }
  else if(RUUVI_SENSOR_SAMPLERATE_SINGLE == *samplerate)    { return RUUVI_ERROR_NOT_SUPPORTED; }
  else if(RUUVI_SENSOR_SAMPLERATE_MIN == *samplerate)       { gyro_cfg.odr = BMG250_ODR_25HZ;   }
  else if(RUUVI_SENSOR_SAMPLERATE_MAX == *samplerate)       { gyro_cfg.odr = BMG250_ODR_3200HZ; }
  else if(RUUVI_SENSOR_SAMPLERATE_NO_CHANGE == *samplerate) { return RUUVI_SUCCESS;             }
  else if(25 >= *samplerate)  { gyro_cfg.odr = BMG250_ODR_25HZ;   }
  else if(50 >= *samplerate)  { gyro_cfg.odr = BMG250_ODR_50HZ;   }
  else if(100 >= *samplerate) { gyro_cfg.odr = BMG250_ODR_100HZ;  }
  else if(200 >= *samplerate) { gyro_cfg.odr = BMG250_ODR_200HZ;  }
  else { return RUUVI_ERROR_NOT_SUPPORTED; }

  result |= bmg250_set_sensor_settings(&gyro_cfg, &gyro);
  // Restore power mode if we did not stop the gyro.
  if(RUUVI_SENSOR_MODE_CONTINOUS == state_power_mode
    && RUUVI_SENSOR_SAMPLERATE_STOP   != *samplerate)
  {
    gyro.power_mode = BMG250_GYRO_NORMAL_MODE;
    result |= bmg250_set_power_mode(&gyro);
  }
  if(BMG250_OK != result)
  {
    err_code |= RUUVI_ERROR_INTERNAL;
  }
  return err_code;
}

ruuvi_status_t bmg250_interface_samplerate_get(ruuvi_sensor_samplerate_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_ERROR_NULL; }

  switch(gyro_cfg.odr)
  {
    case BMG250_ODR_25HZ:
      *samplerate = 25;
      break;

    case BMG250_ODR_50HZ:
      *samplerate = 50;
      break;

    case BMG250_ODR_100HZ:
      *samplerate = 100;
      break;

    case BMG250_ODR_200HZ:
      *samplerate = 200;
      break;

    case BMG250_ODR_3200HZ:
      *samplerate = RUUVI_SENSOR_SAMPLERATE_MAX;
      break;

     default:
       return RUUVI_ERROR_INTERNAL;
  }
  return RUUVI_SUCCESS;
}

ruuvi_status_t bmg250_interface_resolution_set(ruuvi_sensor_resolution_t* resolution)
{
  return RUUVI_ERROR_NOT_SUPPORTED;
}

ruuvi_status_t bmg250_interface_resolution_get(ruuvi_sensor_resolution_t* resolution)
{
  if (NULL == resolution) { return RUUVI_ERROR_NULL; }
  *resolution = 16;
  return RUUVI_SUCCESS;
}

ruuvi_status_t bmg250_interface_scale_set(ruuvi_sensor_scale_t* scale)
{
  if(NULL ==  scale) { return RUUVI_ERROR_NULL; }

  if(RUUVI_SENSOR_SCALE_NO_CHANGE == *scale) { return RUUVI_SUCCESS; }
  else if(RUUVI_SENSOR_SCALE_MIN == *scale) { gyro_cfg.range = BMG250_RANGE_125_DPS; }
  else if(RUUVI_SENSOR_SCALE_MAX == *scale) { gyro_cfg.range = BMG250_RANGE_2000_DPS;}
  else if(125 >= *scale) { gyro_cfg.range = BMG250_RANGE_125_DPS; }
  else if(250 >= *scale) { gyro_cfg.range = BMG250_RANGE_250_DPS; }
  else { return RUUVI_ERROR_NOT_SUPPORTED; }
  
  int8_t result = bmg250_set_sensor_settings(&gyro_cfg, &gyro);
  return (BMG250_OK == result) ? RUUVI_SUCCESS : RUUVI_ERROR_INTERNAL;
}

ruuvi_status_t bmg250_interface_scale_get(ruuvi_sensor_scale_t* scale)
{
  if(NULL ==  scale) { return RUUVI_ERROR_NULL; }

  switch(gyro_cfg.range)
  {
    case BMG250_RANGE_125_DPS:
      *scale = 125;
      break;

    case BMG250_RANGE_250_DPS:
      *scale = 250;
      break;

    case BMG250_RANGE_2000_DPS:
      *scale = RUUVI_SENSOR_SCALE_MAX;
      break;

    default:
      return RUUVI_ERROR_INTERNAL;
  }
  return RUUVI_SUCCESS;
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

  if(NULL == mode) { return RUUVI_ERROR_NULL; }

  switch(*mode)
  {
    case RUUVI_SENSOR_MODE_SLEEP:
      state_power_mode = *mode;
      gyro.power_mode = BMG250_GYRO_SUSPEND_MODE;
      break;

    case RUUVI_SENSOR_MODE_CONTINOUS:
      state_power_mode = *mode;
      gyro.power_mode = BMG250_GYRO_NORMAL_MODE;
      break;
    
    default:
      return RUUVI_ERROR_NOT_SUPPORTED;
  }
  int8_t result = bmg250_set_power_mode(&gyro);
  return (BMG250_OK == result) ? RUUVI_SUCCESS : RUUVI_ERROR_INTERNAL;
}

ruuvi_status_t bmg250_interface_mode_get(ruuvi_sensor_mode_t* mode)
{
  if(NULL == mode) { return RUUVI_ERROR_NULL; }
  *mode = state_power_mode;
  return RUUVI_SUCCESS;
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
  if(NULL == data) { return RUUVI_ERROR_NULL; }

  typedef struct
{
  float x_mdps;
  float y_mdps;
  float z_mdps;
}ruuvi_gyration_data_t;

ruuvi_gyration_data_t* p_gyro = (ruuvi_gyration_data_t*) data;
struct bmg250_sensor_data gyro_data;

int8_t result = bmg250_get_sensor_data(BMG250_DATA_SEL, &gyro_data, &gyro);

switch(gyro_cfg.range)
{
    case BMG250_RANGE_125_DPS:
      p_gyro->x_mdps = BMG250_125_RAW_TO_DPS(gyro_data.x);
      p_gyro->y_mdps = BMG250_125_RAW_TO_DPS(gyro_data.y);
      p_gyro->z_mdps = BMG250_125_RAW_TO_DPS(gyro_data.z);
      break;

    case BMG250_RANGE_250_DPS:
      p_gyro->x_mdps = BMG250_250_RAW_TO_DPS(gyro_data.x);
      p_gyro->y_mdps = BMG250_250_RAW_TO_DPS(gyro_data.y);
      p_gyro->z_mdps = BMG250_250_RAW_TO_DPS(gyro_data.z);
      break;

    case BMG250_RANGE_2000_DPS:
      p_gyro->x_mdps = BMG250_2000_RAW_TO_DPS(gyro_data.x);
      p_gyro->y_mdps = BMG250_2000_RAW_TO_DPS(gyro_data.y);
      p_gyro->z_mdps = BMG250_2000_RAW_TO_DPS(gyro_data.z);
      break;

    default:
      return RUUVI_ERROR_INTERNAL;
      }
return (BMG250_OK == result) ? RUUVI_SUCCESS : RUUVI_ERROR_INTERNAL;
}


#endif