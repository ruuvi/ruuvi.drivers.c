#include "sdk_application_config.h"
#if NRF5_SDK15_ADC

#include "adc.h"
#include "ruuvi_error.h"
#include "ruuvi_sensor.h"

#include "nrf_drv_saadc.h"
#include "sdk_errors.h"

#define PLATFORM_LOG_MODULE_NAME adc
#if ADC_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       ADC_LOG_LEVEL
#define PLATFORM_LOG_INFO_COLOR  ADC_INFO_COLOR
#else // ANT_BPWR_LOG_ENABLED
#define PLATFORM_LOG_LEVEL       0
#endif // ANT_BPWR_LOG_ENABLED
#include "platform_log.h"
PLATFORM_LOG_MODULE_REGISTER();

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS  600.0  //!< Reference voltage (in milli volts) used by ADC while doing conversion.
#define ADC_RES_10BIT                  1024.0 //!< Maximum digital value for 10-bit ADC conversion.
#define ADC_PRE_SCALING_COMPENSATION   6.0    //!< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
    ((((ADC_VALUE) *ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)

static nrf_saadc_value_t     m_buffer_pool[2][1];
static float voltage = ADC_INVALID;

static void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
  if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
  {
    ret_code_t err_code;

    // Setup second buffer
    err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 1);
    APP_ERROR_CHECK(err_code);

    PLATFORM_LOG_INFO("%d", ADC_RESULT_IN_MILLI_VOLTS(p_event->data.done.p_buffer[0]));
    voltage = ADC_RESULT_IN_MILLI_VOLTS(p_event->data.done.p_buffer[0]);
  }
}

// Support only battery reads for now
ruuvi_status_t adc_init(ruuvi_sensor_t* adc)
{
  ret_code_t err_code = NRF_SUCCESS;
  nrf_saadc_channel_config_t channel_config =
    NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);

  //Init SAADC with default configuration
  err_code |= nrf_drv_saadc_init(NULL, saadc_callback);
  APP_ERROR_CHECK(err_code);

  //Use only channel 0
  err_code |= nrf_drv_saadc_channel_init(0, &channel_config);
  APP_ERROR_CHECK(err_code);

  // Prepare first buffer
  err_code |= nrf_drv_saadc_buffer_convert(m_buffer_pool[0], 1);
  APP_ERROR_CHECK(err_code);

  // Prepare second buffer
  err_code |= nrf_drv_saadc_buffer_convert(m_buffer_pool[1], 1);
  APP_ERROR_CHECK(err_code);

  return platform_to_ruuvi_error(&err_code);
}

// Start sampling given channel
ruuvi_status_t adc_sample_asynchronous(adc_channel_t channel)
{
  ret_code_t err_code = nrf_drv_saadc_sample();
  return platform_to_ruuvi_error(&err_code);
}

// read latest data from given channel - TODO return battery voltage for now
float adc_get_data(adc_channel_t channel)
{
  return voltage;
}

#endif