/**
 * Copyright (c) 2016 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * ADC sensor implementation on Nordic SDK15 / nRF52832. Based on es_battery_voltage_saadc.c of nRF5 SDK15.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_NRF52832_ADC_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_adc.h"
#include "ruuvi_interface_adc_mcu.h"

#include "nrf_drv_saadc.h"

#include <string.h>

#define RUUVI_PLATFORM_ADC_NRF52832_DEFAULT_RESOLUTION 10

#define ADC_REF_VOLTAGE_IN_VOLTS  0.600f  // Reference voltage (in milli volts) used by ADC while doing conversion.
#define ADC_PRE_SCALING_COMPENSATION 6.0f    // The ADC is configured to use channel with prescaling as input. And hence the result of conversion is to be multiplied by prescaling to get the actual value of the voltage.

// Macro for checking "ignored" parameters NO_CHANGE, MIN, MAX, DEFAULT
#define RETURN_SUCCESS_ON_VALID(param) do {\
            if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT   == param ||\
               RUUVI_DRIVER_SENSOR_CFG_MIN       == param ||\
               RUUVI_DRIVER_SENSOR_CFG_MAX       == param ||\
               RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == param   \
             ) return RUUVI_DRIVER_SUCCESS;\
           } while(0)

// Macro for checking that sensor is in sleep mode before configuration
#define VERIFY_SENSOR_SLEEPS() do { \
          uint8_t MACRO_MODE = 0; \
          ruuvi_interface_adc_mcu_mode_get(&MACRO_MODE); \
          if(RUUVI_DRIVER_SENSOR_CFG_SLEEP != MACRO_MODE) { return RUUVI_DRIVER_ERROR_INVALID_STATE; } \
          } while(0)

static bool autorefresh  =
  false;        // Flag to keep track if we should update the adc on data read.
static bool adc_is_init  =
  false;        // Flag to keep track if ADC itself is initialized
static uint8_t adc_channel;              // Channel of ADC
static nrf_saadc_value_t adc_buf;        // Buffer used for storing ADC value.
static float adc_volts;                  // Value of last sample in volts.
static uint64_t adc_tsample;             // Time when sample was taken
static nrf_drv_saadc_config_t adc_config =
  NRF_DRV_SAADC_DEFAULT_CONFIG; // Structure for ADC configuration

static const char m_adc_name[] = "nRF5ADC"; //!< Human-readable name

static float raw_adc_to_volts(nrf_saadc_value_t adc)
{
  // Get ADC max value into counts
  uint8_t resolution;
  ruuvi_interface_adc_mcu_resolution_get(&resolution);
  uint16_t counts = 1 << resolution;
  return (ADC_REF_VOLTAGE_IN_VOLTS * ((float)adc / (float)counts) *
          ADC_PRE_SCALING_COMPENSATION);
}

static void nrf52832_adc_sample(void)
{
  nrf_drv_saadc_sample_convert(1, &adc_buf);
  adc_tsample = ruuvi_driver_sensor_timestamp_get();
  adc_volts = raw_adc_to_volts(adc_buf);
}

/**@brief Function handling events from 'nrf_drv_saadc.c'.
 * No implementation needed
 *
 * @param[in] p_evt SAADC event.
 */
static void saadc_event_handler(nrf_drv_saadc_evt_t const* p_evt)
{
  if(p_evt->type == NRF_DRV_SAADC_EVT_DONE)
  {
  }
}

// Converts Ruuvi ADC channel to nRF adc channel
static nrf_saadc_input_t ruuvi_to_nrf_adc_channel(ruuvi_interface_adc_channel_t channel)
{
  switch(channel)
  {
    case RUUVI_INTERFACE_ADC_AIN0:
      return NRF_SAADC_INPUT_AIN0;

    case RUUVI_INTERFACE_ADC_AIN1:
      return NRF_SAADC_INPUT_AIN1;

    case RUUVI_INTERFACE_ADC_AIN2:
      return NRF_SAADC_INPUT_AIN2;

    case RUUVI_INTERFACE_ADC_AIN3:
      return NRF_SAADC_INPUT_AIN3;

    case RUUVI_INTERFACE_ADC_AIN4:
      return NRF_SAADC_INPUT_AIN4;

    case RUUVI_INTERFACE_ADC_AIN5:
      return NRF_SAADC_INPUT_AIN5;

    case RUUVI_INTERFACE_ADC_AIN6:
      return NRF_SAADC_INPUT_AIN6;

    case RUUVI_INTERFACE_ADC_AIN7:
      return NRF_SAADC_INPUT_AIN7;

    case RUUVI_INTERFACE_ADC_AINVDD:
      return NRF_SAADC_INPUT_VDD;

    default:
      return NRF_SAADC_INPUT_DISABLED;
  }
}

// Converts ruuvi ADC resolution to nRF SAADC resolution
static nrf_saadc_resolution_t ruuvi_to_nrf_resolution(const uint8_t resolution)
{
  switch(resolution)
  {
    case RUUVI_DRIVER_SENSOR_CFG_MIN:
    case 8:
      return NRF_SAADC_RESOLUTION_8BIT;

    case 10:
      return NRF_SAADC_RESOLUTION_10BIT;

    case 12:
      return NRF_SAADC_RESOLUTION_12BIT;

    case RUUVI_DRIVER_SENSOR_CFG_MAX:
    case 14:
      return NRF_SAADC_RESOLUTION_14BIT;

    // we can't return "invalid", return 10 as default instead.
    default:
      return NRF_SAADC_RESOLUTION_10BIT;
  }
}

// Converts nRF SAADC resolution to ruuvi ADC resolution
static uint8_t nrf_to_ruuvi_resolution(const nrf_saadc_resolution_t resolution)
{
  switch(resolution)
  {
    case NRF_SAADC_RESOLUTION_8BIT:
      return 8;

    case NRF_SAADC_RESOLUTION_10BIT:
      return 10;

    case NRF_SAADC_RESOLUTION_12BIT:
      return 12;

    case NRF_SAADC_RESOLUTION_14BIT:
      return 14;

    default:
      return RUUVI_PLATFORM_ADC_NRF52832_DEFAULT_RESOLUTION;
  }
}

// Uninitializes and reinitializes ADC to apply new configuration
static ruuvi_driver_status_t reinit_adc(void)
{
  ret_code_t err_code = NRF_SUCCESS;
  nrf_drv_saadc_uninit();
  err_code |= nrf_drv_saadc_init(&adc_config, saadc_event_handler);
  adc_volts = RUUVI_INTERFACE_ADC_INVALID;
  adc_tsample = RUUVI_DRIVER_UINT64_INVALID;
  autorefresh = false;
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

// Convert UINT8_T to nRF oversampling
static nrf_saadc_oversample_t uint_to_nrf_os(uint8_t* const parameter)
{
  nrf_saadc_oversample_t oversample = NRF_SAADC_OVERSAMPLE_DISABLED;
  if(2 >= *parameter)
    {
      oversample = NRF_SAADC_OVERSAMPLE_2X;
      *parameter = 2;
    }
    else if(4 >= *parameter)
    {
      oversample = NRF_SAADC_OVERSAMPLE_4X;
      *parameter = 4;
    }
    else if(8 >= *parameter)
    {
      oversample = NRF_SAADC_OVERSAMPLE_8X;
      *parameter = 8;
    }
    else if(16 >= *parameter)
    {
      oversample = NRF_SAADC_OVERSAMPLE_16X;
      *parameter = 16;
    }
    else if(32 >= *parameter)
    {
      oversample = NRF_SAADC_OVERSAMPLE_32X;
      *parameter = 32;
    }
    else if(64 >= *parameter)
    {
      oversample = NRF_SAADC_OVERSAMPLE_64X;
      *parameter = 64;
    }
    else if(128 >= *parameter)
    {
      oversample = NRF_SAADC_OVERSAMPLE_128X;
      *parameter = 128;
    }
  return oversample;
} 

ruuvi_driver_status_t ruuvi_interface_adc_mcu_init(ruuvi_driver_sensor_t* adc_sensor,
    ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == adc_sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  if(RUUVI_DRIVER_BUS_NONE != bus) { return RUUVI_DRIVER_ERROR_NOT_SUPPORTED; }
  
  // Support only one instance.
  if(adc_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  // Initialize ADC structure
  ruuvi_driver_sensor_initialize(adc_sensor);
  nrf_drv_saadc_config_t adc_default = NRF_DRV_SAADC_DEFAULT_CONFIG;
  memcpy(&adc_config, &adc_default, sizeof(adc_config));
  ret_code_t err_code = nrf_drv_saadc_init(&adc_config, saadc_event_handler);

  if(NRF_SUCCESS == err_code) { adc_is_init = true; }

  // Initialize given channel. Only one ADC channel is supported at a time
  nrf_saadc_channel_config_t ch_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(
      ruuvi_to_nrf_adc_channel(handle));
  ch_config.gain =  NRF_SAADC_GAIN1_6;
  nrf_saadc_channel_init(0, &ch_config);
  adc_channel = handle;
  // Setup function pointers
  adc_sensor->init              = ruuvi_interface_adc_mcu_init;
  adc_sensor->uninit            = ruuvi_interface_adc_mcu_uninit;
  adc_sensor->samplerate_set    = ruuvi_interface_adc_mcu_samplerate_set;
  adc_sensor->samplerate_get    = ruuvi_interface_adc_mcu_samplerate_get;
  adc_sensor->resolution_set    = ruuvi_interface_adc_mcu_resolution_set;
  adc_sensor->resolution_get    = ruuvi_interface_adc_mcu_resolution_get;
  adc_sensor->scale_set         = ruuvi_interface_adc_mcu_scale_set;
  adc_sensor->scale_get         = ruuvi_interface_adc_mcu_scale_get;
  adc_sensor->dsp_set           = ruuvi_interface_adc_mcu_dsp_set;
  adc_sensor->dsp_get           = ruuvi_interface_adc_mcu_dsp_get;
  adc_sensor->mode_set          = ruuvi_interface_adc_mcu_mode_set;
  adc_sensor->mode_get          = ruuvi_interface_adc_mcu_mode_get;
  adc_sensor->data_get          = ruuvi_interface_adc_mcu_data_get;
  adc_sensor->configuration_set = ruuvi_driver_sensor_configuration_set;
  adc_sensor->configuration_get = ruuvi_driver_sensor_configuration_get;
  adc_sensor->name              = m_adc_name;
  adc_sensor->provides.datas.voltage_v = 1;
  adc_volts = RUUVI_INTERFACE_ADC_INVALID;
  adc_tsample = RUUVI_DRIVER_UINT64_INVALID;
  autorefresh = false;
  return ruuvi_nrf5_sdk15_to_ruuvi_error(err_code);
}

ruuvi_driver_status_t ruuvi_interface_adc_mcu_uninit(ruuvi_driver_sensor_t* adc_sensor,
    ruuvi_driver_bus_t bus, uint8_t handle)
{
  if(NULL == adc_sensor) { return RUUVI_DRIVER_ERROR_NULL; }

  ruuvi_driver_sensor_uninitialize(adc_sensor);
  nrf_drv_saadc_uninit();
  adc_is_init = false;
  autorefresh = false;
  adc_tsample = RUUVI_DRIVER_UINT64_INVALID;
  adc_volts = RUUVI_INTERFACE_ADC_INVALID;
  return RUUVI_DRIVER_SUCCESS;
}

// Continuous sampling is not supported (although we could use timer and PPI to implement it), mark pointed value as not supported even if parameter is one of no-changes
ruuvi_driver_status_t ruuvi_interface_adc_mcu_samplerate_set(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  uint8_t original = *samplerate;
  *samplerate = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
  RETURN_SUCCESS_ON_VALID(original);
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_adc_mcu_samplerate_get(uint8_t* samplerate)
{
  if(NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

  *samplerate = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_adc_mcu_resolution_set(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();

  if(RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE == *resolution)    { return ruuvi_interface_adc_mcu_resolution_get(resolution); }

  if(RUUVI_DRIVER_SENSOR_CFG_MIN == *resolution)
  {
    *resolution = 8;
    adc_config.resolution = NRF_SAADC_RESOLUTION_8BIT;
  }
  else if(RUUVI_DRIVER_SENSOR_CFG_MAX == *resolution)
  {
    *resolution = 14;
    adc_config.resolution = NRF_SAADC_RESOLUTION_14BIT;
  }
  else if(RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *resolution)
  {
    *resolution = RUUVI_PLATFORM_ADC_NRF52832_DEFAULT_RESOLUTION;
    adc_config.resolution = ruuvi_to_nrf_resolution(*resolution);
  }
  else if(8 >= *resolution)  { adc_config.resolution = NRF_SAADC_RESOLUTION_8BIT;  *resolution = 8; }
  else if(10 >= *resolution) { adc_config.resolution = NRF_SAADC_RESOLUTION_10BIT; *resolution = 10; }
  else if(12 >= *resolution) { adc_config.resolution = NRF_SAADC_RESOLUTION_12BIT; *resolution = 12; }
  else if(14 >= *resolution) { adc_config.resolution = NRF_SAADC_RESOLUTION_14BIT; *resolution = 14; }
  else
  {
    *resolution = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
  }

  // Uninit and reinit adc with new settings.
  return reinit_adc();
}

ruuvi_driver_status_t ruuvi_interface_adc_mcu_resolution_get(uint8_t* resolution)
{
  if(NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

  *resolution = nrf_to_ruuvi_resolution(adc_config.resolution);
  return RUUVI_DRIVER_SUCCESS;
}

// While scale could be adjustable, we'll use fixed 3600 mV.
ruuvi_driver_status_t ruuvi_interface_adc_mcu_scale_set(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  uint8_t original = *scale;
  // "At least" 3
  *scale = 3;
  RETURN_SUCCESS_ON_VALID(original);
  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_adc_mcu_scale_get(uint8_t* scale)
{
  if(NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

  // "At least" 3
  *scale = 3;
  return RUUVI_DRIVER_SUCCESS;
}

// Return success on DSP_LAST, DSP_OVERSAMPLING and acceptable defaults, not supported otherwise
ruuvi_driver_status_t ruuvi_interface_adc_mcu_dsp_set(uint8_t* dsp, uint8_t* parameter)
{
  if(NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }

  VERIFY_SENSOR_SLEEPS();
  // Store originals
  uint8_t dsp_original;
  dsp_original       = *dsp;
  // Ge actual values
  ruuvi_interface_adc_mcu_dsp_get(dsp, parameter);

  // Set new values if applicable
  if(RUUVI_DRIVER_SENSOR_DSP_LAST == dsp_original ||
      RUUVI_DRIVER_SENSOR_CFG_DEFAULT == dsp_original)
  {
    adc_config.oversample = NRF_SAADC_OVERSAMPLE_DISABLED;
    *parameter = 1;
    *dsp = RUUVI_DRIVER_SENSOR_DSP_LAST;
    reinit_adc();
    return RUUVI_DRIVER_SUCCESS;
  }

  // 128 is maximum we support
  if((RUUVI_DRIVER_SENSOR_DSP_OS == dsp_original) &&
      (128 < *parameter))
  {
    adc_config.oversample = uint_to_nrf_os(parameter);
    reinit_adc();
    return RUUVI_DRIVER_SUCCESS;
  }

  return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_adc_mcu_dsp_get(uint8_t* dsp, uint8_t* parameter)
{
  switch(adc_config.oversample)
  {
    case NRF_SAADC_OVERSAMPLE_DISABLED:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_LAST;
      *parameter = 1;
      break;

    case NRF_SAADC_OVERSAMPLE_2X:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 2;
      break;

    case NRF_SAADC_OVERSAMPLE_4X:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 4;
      break;

    case NRF_SAADC_OVERSAMPLE_8X:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 8;
      break;

    case NRF_SAADC_OVERSAMPLE_16X:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 16;
      break;

    case NRF_SAADC_OVERSAMPLE_32X:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 32;
      break;

    case NRF_SAADC_OVERSAMPLE_64X:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 64;
      break;

    case NRF_SAADC_OVERSAMPLE_128X:
      *dsp = RUUVI_DRIVER_SENSOR_DSP_OS;
      *parameter = 128;
      break;

    default:
      return RUUVI_DRIVER_ERROR_INTERNAL;
  }

  return RUUVI_DRIVER_SUCCESS;
}

// Start single on command, mark autorefresh with continuous
ruuvi_driver_status_t ruuvi_interface_adc_mcu_mode_set(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  // Enter sleep by default and by explicit sleep commmand
  if(RUUVI_DRIVER_SENSOR_CFG_SLEEP == *mode || RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *mode)
  {
    autorefresh = false;
    *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
    return RUUVI_DRIVER_SUCCESS;
  }

  if(RUUVI_DRIVER_SENSOR_CFG_SINGLE == *mode)
  {
    // Do nothing if sensor is in continuous mode
    uint8_t current_mode;
    ruuvi_interface_adc_mcu_mode_get(&current_mode);

    if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == current_mode)
    {
      *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
      return RUUVI_DRIVER_ERROR_INVALID_STATE;
    }

    // Enter sleep after measurement
    autorefresh = false;
    *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
    // Global float is updated by sample
    nrf52832_adc_sample();
    return RUUVI_DRIVER_SUCCESS;
  }

  if(RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == *mode)
  {
    autorefresh = true;
    return RUUVI_DRIVER_SUCCESS;
  }

  return RUUVI_DRIVER_ERROR_INVALID_PARAM;
}


ruuvi_driver_status_t ruuvi_interface_adc_mcu_mode_get(uint8_t* mode)
{
  if(NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

  if(autorefresh)
  {
    *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
  }

  if(!autorefresh)
  {
    *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
  }

  return RUUVI_DRIVER_SUCCESS;
}


ruuvi_driver_status_t ruuvi_interface_adc_mcu_data_get(ruuvi_driver_sensor_data_t* const p_data)
{
  if(NULL == p_data) { return RUUVI_DRIVER_ERROR_NULL; }

  if(autorefresh) { nrf52832_adc_sample(); }

  p_data->timestamp_ms    = RUUVI_DRIVER_UINT64_INVALID;

  if(!isnan(adc_volts))
  {
    ruuvi_driver_sensor_data_t d_adc;
    ruuvi_driver_sensor_data_fields_t adc_fields = {.bitfield = 0};
    float adc_values[1];
    adc_values[0] = adc_volts;
    adc_fields.datas.voltage_v = 1;
    d_adc.data = adc_values;
    d_adc.valid  = adc_fields;
    d_adc.fields = adc_fields;
    ruuvi_driver_sensor_data_populate(p_data,
                                      &d_adc,
                                      p_data->fields);
    p_data->timestamp_ms = adc_tsample;
  }

  return RUUVI_DRIVER_SUCCESS;
}

/**
 * @brief take complex sample
 *
 * This function fills the need for more complex sampling, such as using differential 
 * measurement, different reference voltages and oversampling. 
 * Initializes the ADC before sampling and uninitializes the ADC after sampling. 
 *
 * @param[in]  sample definition of the sample to take
 * @param[out] data value of sample in volts and as a ratio to reference. 
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_NULL if either parameter is NULL
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if ADC is already initialized
 * @return RUUVI_DRIVER_ERROR_INVALID_PARAMETER if configuration is invalid in any manner
 * @return error code from stack on other error. 
 */
ruuvi_driver_status_t ruuvi_interface_adc_complex_sample(const ruuvi_interface_adc_sample_t* const sample, ruuvi_interface_adc_data_t* const data)
{
  if(NULL == data || NULL == sample) { return RUUVI_DRIVER_ERROR_NULL; }
  if(true == adc_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  // Initialize ADC
  nrf_drv_saadc_config_t adc_default = NRF_DRV_SAADC_DEFAULT_CONFIG;
  memcpy(&adc_config, &adc_default, sizeof(adc_config));
  uint8_t os = sample->oversamples;
  adc_config.oversample = uint_to_nrf_os(&os);
  adc_config.resolution = NRF_SAADC_RESOLUTION_14BIT;
  ret_code_t err_code = nrf_drv_saadc_init(&adc_config, saadc_event_handler);

  // Initialize given channel. Only one ADC channel is supported at a time
  nrf_saadc_channel_config_t ch_config = 
  {                                                                  \
    .resistor_p = NRF_SAADC_RESISTOR_DISABLED,                       \
    .resistor_n = NRF_SAADC_RESISTOR_DISABLED,                       \
    .gain       = NRF_SAADC_GAIN1_6,                                 \
    .reference  = NRF_SAADC_REFERENCE_INTERNAL,                      \
    .acq_time   = NRF_SAADC_ACQTIME_10US,                            \
    .mode       = (RUUVI_INTERFACE_ADC_AINGND == (sample->negative)) ? NRF_SAADC_MODE_SINGLE_ENDED : NRF_SAADC_MODE_DIFFERENTIAL,\
    .pin_p      = ruuvi_to_nrf_adc_channel(sample->positive),        \
    .pin_n      = ruuvi_to_nrf_adc_channel(sample->negative)         
  };

  nrf_saadc_channel_init(0, &ch_config);
  return err_code;
}

#endif