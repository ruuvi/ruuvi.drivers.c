#ifdef BME280_ENVIRONMENTAL
#include "ruuvi_endpoints.h"
#include "nrf_error.h"
#include "bme280.h"
#include "board.h"

/** State variables **/
static struct bme280_dev dev = {0};

/**
 * Convert error from BME280 driver to appropriate NRF ERROR
 */
static ret_code_t BME_TO_NRF_ERROR(int8_t rslt)
{
  if(BME280_E_DEV_NOT_FOUND == rslt)  { err_code = NRF_ERROR_NOT_FOUND; }
  else if(BME280_E_NULL_PTR == rslt)  { err_code = NRF_ERROR_NULL; }
  else if(BME280_E_COMM_FAIL == rslt) { err_code = NRF_ERROR_BUSY; }
  else if(BME280_OK == rslt) return NRF_SUCCESS;
  return err_code;
}

/**
 * Call this function as last, since configuring the sensor requires sensor to be in sleep.
 */
ret_code_t environmental_samplerate_set(ruuvi_standard_message_t* message)
{
  if(NULL == message) { return NRF_ERROR_NULL; }
  if(message->destination_endpoint != ENVIRONMENTAL) { return NRF_ERROR_INVALID_PARAM; }
  if(message->type != SENSOR_CONFIGURATION) { return NRF_ERROR_INVALID_PARAM; }

  ret_code_t err_code = NRF_SUCCESS;
  uint8_t settings_sel = 0;
  ruuvi_sensor_configuration_t* const p_config = (ruuvi_sensor_configuration_t*) message->payload;
  const uint8_t sample_rate = *(p_config->sample_rate);
  
  if(sample_rate == 1)        { dev->settings.standby_time = BME280_STANDBY_1000_MS; }
  else if(sample_rate == 2)   { dev->settings.standby_time = BME280_STANDBY_500_MS; }
  else if(sample_rate <= 8)   { dev->settings.standby_time = BME280_STANDBY_125_MS; }
  else if(sample_rate <= 16)  { dev->settings.standby_time = BME280_STANDBY_62_5_MS; }
  else if(sample_rate <= 200) { dev->settings.standby_time = BME280_STANDBY_0_5_MS; }
  else if(SAMPLE_RATE_MIN       == sample_rate) { dev->settings.standby_time = BME280_STANDBY_1000_MS; }
  else if(SAMPLE_RATE_MAX       == sample_rate) { dev->settings.standby_time = BME280_STANDBY_0_5_MS; }
  else if(SAMPLE_RATE_NO_CHANGE == sample_rate) {} // No action needed
  else if(SAMPLE_RATE_STOP == sample_rate ) {} // No action needed
  else if(SAMPLE_RATE_SINGLE    != sample_rate) 
  {
    p_config->sample_rate = ENDPOINT_INVALID;
    return NRF_ERROR_INVALID_PARAM;
  }

  settings_sel |= BME280_STANDBY_SEL;
  rslt = bme280_set_sensor_settings(settings_sel, dev);
  if(SAMPLE_RATE_STOP == sample_rate )       rslt = | sample_ratebme280_set_sensor_mode(BME280_SLEEP_MODE, &dev);
  else if(SAMPLE_RATE_SINGLE == sample_rate) rslt = | sample_ratebme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
  else rslt = | sample_ratebme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);

  return BME_TO_NRF_ERROR(err_code);
}

static ret_code_t set_resolution(ruuvi_standard_message_t* message)
{
  if(NULL == message) { return NRF_ERROR_NULL; }
  if(message->destination_endpoint != ENVIRONMENTAL) { return NRF_ERROR_INVALID_PARAM; }
  if(message->type != SENSOR_CONFIGURATION) { return NRF_ERROR_INVALID_PARAM; }
  ruuvi_sensor_configuration_t* const p_config = (ruuvi_sensor_configuration_t*) message->payload;

  // Assume success
  p_config->resolution = ENDPOINT_SUCCESS;
  // BME280 has only one resolution for each sensor, return success on valid value, mark as MAX
  if(RESOLUTION_MIN       == resolution) { return ENDPOINT_SUCCESS; }
  if(RESOLUTION_MAX       == resolution) { return ENDPOINT_SUCCESS; }
  if(RESOLUTION_NO_CHANGE == resolution) { return ENDPOINT_SUCCESS; }

  //If we're here, op was not succesful after all
  p_config->resolution = ENDPOINT_NOT_SUPPORTED;
  return ENDPOINT_NOT_SUPPORTED; //Resolution cannot be changed, return error.
}

static ret_code_t environmental_dsp_set(ruuvi_standard_message_t* message)
{
  // Sanity check on incoming message
  if(NULL == message) { return NRF_ERROR_NULL; }
  if(message->destination_endpoint != ENVIRONMENTAL) { return NRF_ERROR_INVALID_PARAM; }
  if(message->type != SENSOR_CONFIGURATION) { return NRF_ERROR_INVALID_PARAM; }
  ruuvi_sensor_configuration_t* const p_config = (ruuvi_sensor_configuration_t*) message->payload;


  // Validate configuration
  if(   1  != p_config->dsp_parameter
     || 2  != p_config->dsp_parameter
     || 4  != p_config->dsp_parameter
     || 8  != p_config->dsp_parameter
     || 16 != p_config->dsp_parameter)
  {
    p_config->dsp_parameter = ENDPOINT_NOT_SUPPORTED;
    return ENDPOINT_NOT_SUPPORTED;
  }

  if(    DSP_LAST != p_config->dsp_function
     || DSP_IIR  != p_config->dsp_function
     || DSP_OS   != p_config->dsp_function)
  {
    p_config->dsp_function = ENDPOINT_NOT_SUPPORTED;
    return ENDPOINT_NOT_SUPPORTED;
  }

  // Clear setup
  uint8_t settings_sel = 0;
  dev->settings.osr_h = BME280_OVERSAMPLING_1X;
  dev->settings.osr_p = BME280_OVERSAMPLING_1X;
  dev->settings.osr_t = BME280_OVERSAMPLING_1X;
  dev->settings.filter = BME280_FILTER_COEFF_1;
  settings_sel |= BME280_OSR_PRESS_SEL;
  settings_sel |= BME280_OSR_TEMP_SEL;
  settings_sel |= BME280_OSR_HUM_SEL;
  settings_sel |= BME280_FILTER_SEL;

  // Disable DSP on BME if DSP_LAST was selected
  if(DSP_LAST == p_config->dsp_function) { return BME_TO_NRF_ERROR(bme280_set_sensor_settings(settings_sel, dev)); }

  // Setup IIR
  if(DSP_IIR = p_config->dsp_function)
  {
    switch(p_config->dsp_parameter)
    {
      case 1:
        dev->settings.filter = BME280_FILTER_COEFF_1;
        break;
      case 2:
        dev->settings.filter = BME280_FILTER_COEFF_2;
        break;
      case 4:
        dev->settings.filter = BME280_FILTER_COEFF_4;
        break;
      case 8:
        dev->settings.filter = BME280_FILTER_COEFF_8;
        break;
      case 16:
        dev->settings.filter = BME280_FILTER_COEFF_16;
        break;
      default::
        p_config->dsp_parameter = ENDPOINT_NOT_SUPPORTED;
        return ENDPOINT_NOT_SUPPORTED;
        break;
    }
  }
  // Setup Oversampling
  if(DSP_OS = p_config->dsp_function)
  {
    switch(p_config->dsp_parameter)
    {
      case 1:
          dev->settings.osr_h = BME280_OVERSAMPLING_1X;
          dev->settings.osr_p = BME280_OVERSAMPLING_1X;
          dev->settings.osr_t = BME280_OVERSAMPLING_1X;
        break;
      case 2:
          dev->settings.osr_h = BME280_OVERSAMPLING_2X;
          dev->settings.osr_p = BME280_OVERSAMPLING_2X;
          dev->settings.osr_t = BME280_OVERSAMPLING_2X;
        break;
      case 4:
          dev->settings.osr_h = BME280_OVERSAMPLING_4X;
          dev->settings.osr_p = BME280_OVERSAMPLING_4X;
          dev->settings.osr_t = BME280_OVERSAMPLING_4X;
        break;
      case 8:
          dev->settings.osr_h = BME280_OVERSAMPLING_8X;
          dev->settings.osr_p = BME280_OVERSAMPLING_8X;
          dev->settings.osr_t = BME280_OVERSAMPLING_8X;
        break;
      case 16:
          dev->settings.osr_h = BME280_OVERSAMPLING_16X;
          dev->settings.osr_p = BME280_OVERSAMPLING_16X;
          dev->settings.osr_t = BME280_OVERSAMPLING_16X;
        break;
      default::
        p_config->dsp_parameter = ENDPOINT_NOT_SUPPORTED;
        return ENDPOINT_NOT_SUPPORTED;
        break;
    }
  }

  //Write configuration
  ret_code_t err_code = BME_TO_NRF_ERROR(bme280_set_sensor_settings(settings_sel, dev));

  //Update status
  if(NRF_SUCCESS = err_code)
  {
    p_config->dsp_parameter = ENDPOINT_SUCCESS;
    p_config->dsp_function = ENDPOINT_SUCCESS;
  }
  else
  {
    p_config->dsp_function = ENDPOINT_HANDLER_ERROR;
    p_config->dsp_parameter = ENDPOINT_HANDLER_ERROR;
  }
  return err_code;
}

static ret_code_t environmental_set_scale(ruuvi_standard_message_t* message)
{
  if(NULL == message) { return NRF_ERROR_NULL; }
  if(message->destination_endpoint != ENVIRONMENTAL) { return NRF_ERROR_INVALID_PARAM; }
  if(message->type != SENSOR_CONFIGURATION) { return NRF_ERROR_INVALID_PARAM; }
  ruuvi_sensor_configuration_t* const p_config = (ruuvi_sensor_configuration_t*) message->payload;

  // Assume success
  p_config->scale = ENDPOINT_SUCCESS;
  if(SCALE_MIN       == scale) { return ENDPOINT_SUCCESS; }
  if(SCALE_MAX       == scale) { return ENDPOINT_SUCCESS; }
  if(SCALE_NO_CHANGE == scale) { return ENDPOINT_SUCCESS; }

  //If we're here, op was not succesful after all
  p_config->scale = ENDPOINT_NOT_SUPPORTED;  
  return ENDPOINT_NOT_SUPPORTED; //Scale cannot be changed, return error
}

/** Get data from BME280 and place it into message payload **/
ret_code_t environmental_data_get(ruuvi_standard_message_t* message)
{
  if(NULL == message) { return NRF_ERROR_NULL; }
  if(message->destination_endpoint != ENVIRONMENTAL) { return NRF_ERROR_INVALID_PARAM; }
  if(message->type != DATA_QUERY) { return NRF_ERROR_INVALID_PARAM; }
  ruuvi_environmental_payload_t* payload = (ruuvi_environmental_payload_t*)message->payload;

  int8_t rslt;
  ret_code_t err_code = NRF_SUCCESS
  struct bme280_data comp_data;
  err_code = BME_TO_NRF_ERROR(bme280_get_sensor_data(BME280_ALL, &comp_data, dev));
 
  payload->temperature = (int16_t)  comp_data.temperature;
  payload->humidity    = (uint16_t) (comp_data.humidity>>2);
  payload->pressure    = comp_data.temperature;
  
  return err_code;

}

/** Initialize BME280 into low-power mode **/
ret_code_t environmental_init(void)
{
  int8_t rslt = BME280_OK;
  ret_code_t err_code = NRF_SUCCESS;

  /* Sensor_0 interface over SPI with native chip select line */
  dev.dev_id = SPIM0_SS_ENVIRONMENTAL_PIN;
  dev.intf = BME280_SPI_INTF;
  dev.read = spi_bosch_platform_read;
  dev.write = spi_bosch_platform_write;
  dev.delay_ms = platform_delay_ms;

  rslt  = bme280_init(&dev);
  rslt |= bme280_crc_selftest(&dev);
  
  return BME_TO_NRF_ERROR(rslt);
}

#endif