/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
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
 * Environmental sensor implementation on Nordic SDK15 / nRF52832.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_NRF52832_ENVIRONMENTAL_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_environmental.h"
#include "ruuvi_interface_environmental_mcu.h"

#include "nrf_sdm.h"
#include "nrf_temp.h"

#include <string.h>

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
          ruuvi_interface_environmental_mcu_mode_get(&MACRO_MODE); \
          if(RUUVI_DRIVER_SENSOR_CFG_SLEEP != MACRO_MODE) { return RUUVI_DRIVER_ERROR_INVALID_STATE; } \
          } while(0)

// Flag to keep track if we should update the temperature register on data read.
static bool autorefresh  = false;
static bool sensor_is_init = false;
static float temperature;
static uint64_t tsample;
static const char m_tmp_name[] = "nRF5TMP"; //!< Human-readable name

static void nrf52832_temperature_sample (void)
{
    uint8_t sd_enabled;
    int32_t raw_temp;
    // Check if softdevice is enabled
    sd_softdevice_is_enabled (&sd_enabled);

    // If Nordic softdevice is enabled, we cannot use temperature peripheral directly
    if (sd_enabled)
    {
        sd_temp_get (&raw_temp);
    }

    // If SD is not enabled, call the peripheral directly.
    if (!sd_enabled)
    {
        NRF_TEMP->TASKS_START = 1; /** Start the temperature measurement. */

        /* Busy wait while temperature measurement is not finished, you can skip waiting if you enable interrupt for DATARDY event and read the result in the interrupt. */
        /*lint -e{845} // A zero has been given as right argument to operator '|'" */
        while (NRF_TEMP->EVENTS_DATARDY == 0)
        {
            // Do nothing.
        }

        NRF_TEMP->EVENTS_DATARDY = 0;
        /**@note Workaround for PAN_028 rev2.0A anomaly 29 - TEMP: Stop task clears the TEMP register. */
        raw_temp = nrf_temp_read();
        /**@note Workaround for PAN_028 rev2.0A anomaly 30 - TEMP: Temp module analog front end does not power down when DATARDY event occurs. */
        NRF_TEMP->TASKS_STOP = 1; /** Stop the temperature measurement. */
    }

    temperature = raw_temp / 4.0f;
    tsample = ruuvi_driver_sensor_timestamp_get();
}

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_init (ruuvi_driver_sensor_t *
        environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
    if (NULL == environmental_sensor) { return RUUVI_DRIVER_ERROR_NULL; }

    if (true == sensor_is_init) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

    ruuvi_driver_sensor_initialize (environmental_sensor);
    // Workaround for PAN_028 rev2.0A anomaly 31 - TEMP: Temperature offset value has to be manually loaded to the TEMP module
    nrf_temp_init();
    tsample     = RUUVI_DRIVER_UINT64_INVALID;
    temperature = RUUVI_DRIVER_FLOAT_INVALID;
    // Setup function pointers
    environmental_sensor->init              = ruuvi_interface_environmental_mcu_init;
    environmental_sensor->uninit            = ruuvi_interface_environmental_mcu_uninit;
    environmental_sensor->samplerate_set    =
        ruuvi_interface_environmental_mcu_samplerate_set;
    environmental_sensor->samplerate_get    =
        ruuvi_interface_environmental_mcu_samplerate_get;
    environmental_sensor->resolution_set    =
        ruuvi_interface_environmental_mcu_resolution_set;
    environmental_sensor->resolution_get    =
        ruuvi_interface_environmental_mcu_resolution_get;
    environmental_sensor->scale_set         = ruuvi_interface_environmental_mcu_scale_set;
    environmental_sensor->scale_get         = ruuvi_interface_environmental_mcu_scale_get;
    environmental_sensor->dsp_set           = ruuvi_interface_environmental_mcu_dsp_set;
    environmental_sensor->dsp_get           = ruuvi_interface_environmental_mcu_dsp_get;
    environmental_sensor->mode_set          = ruuvi_interface_environmental_mcu_mode_set;
    environmental_sensor->mode_get          = ruuvi_interface_environmental_mcu_mode_get;
    environmental_sensor->data_get          = ruuvi_interface_environmental_mcu_data_get;
    environmental_sensor->configuration_set = ruuvi_driver_sensor_configuration_set;
    environmental_sensor->configuration_get = ruuvi_driver_sensor_configuration_get;
    environmental_sensor->name              = m_tmp_name;
    environmental_sensor->provides.datas.temperature_c = 1;
    sensor_is_init = true;
    return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_uninit (
    ruuvi_driver_sensor_t * environmental_sensor, ruuvi_driver_bus_t bus, uint8_t handle)
{
    if (NULL == environmental_sensor) { return RUUVI_DRIVER_ERROR_NULL; }

    sensor_is_init = false;
    autorefresh = false;
    ruuvi_driver_sensor_uninitialize (environmental_sensor);
    tsample     = RUUVI_DRIVER_UINT64_INVALID;
    return RUUVI_DRIVER_SUCCESS;
}

// Continuous sampling is not supported, mark pointed value as default even if parameter is one of no-changes
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_samplerate_set (
    uint8_t * samplerate)
{
    if (NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

    VERIFY_SENSOR_SLEEPS();
    uint8_t original = *samplerate;
    *samplerate = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID (original);
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_samplerate_get (
    uint8_t * samplerate)
{
    if (NULL == samplerate) { return RUUVI_DRIVER_ERROR_NULL; }

    *samplerate = RUUVI_DRIVER_SENSOR_CFG_DEFAULT;
    return RUUVI_DRIVER_SUCCESS;
}

// Temperature resolution is fixed to 10 bits, including sign. Return error to driver, but mark used value to pointer.
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_resolution_set (
    uint8_t * resolution)
{
    if (NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

    VERIFY_SENSOR_SLEEPS();

    // If 10 bits was given, return success
    if (10 == *resolution) {return RUUVI_DRIVER_SUCCESS; }

    // Otherwise mark the actual resolution
    uint8_t original = *resolution;
    *resolution = 10;
    RETURN_SUCCESS_ON_VALID (original);
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_resolution_get (
    uint8_t * resolution)
{
    if (NULL == resolution) { return RUUVI_DRIVER_ERROR_NULL; }

    *resolution = 10;
    return RUUVI_DRIVER_SUCCESS;
}

// Scale cannot be set. Our scale is fixed at (2^9) / 4 = 128 (or -127).
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_scale_set (uint8_t * scale)
{
    if (NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

    VERIFY_SENSOR_SLEEPS();

    // If 128 or less was given, return success
    if (128 >= *scale)
    {
        *scale = 128;
        return RUUVI_DRIVER_SUCCESS;
    }

    // Otherwise mark the actual scale
    uint8_t original = *scale;
    *scale = 128;
    RETURN_SUCCESS_ON_VALID (original);
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_scale_get (uint8_t * scale)
{
    if (NULL == scale) { return RUUVI_DRIVER_ERROR_NULL; }

    *scale = 128;
    return RUUVI_DRIVER_SUCCESS;
}

// Return success on DSP_LAST and acceptable defaults, not supported otherwise
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_dsp_set (uint8_t * dsp,
        uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter) { return RUUVI_DRIVER_ERROR_NULL; }

    VERIFY_SENSOR_SLEEPS();

    if (RUUVI_DRIVER_SENSOR_DSP_LAST == * dsp) { return RUUVI_DRIVER_SUCCESS; }

    uint8_t original = *dsp;
    *dsp       = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
    *parameter = RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED;
    RETURN_SUCCESS_ON_VALID (original);
    return RUUVI_DRIVER_ERROR_NOT_SUPPORTED;
}

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_dsp_get (uint8_t * dsp,
        uint8_t * parameter)
{
    *dsp = RUUVI_DRIVER_SENSOR_DSP_LAST;
    *parameter = 1;
    return RUUVI_DRIVER_SUCCESS;
}

// Start single on command, mark autorefresh with continuous
ruuvi_driver_status_t ruuvi_interface_environmental_mcu_mode_set (uint8_t * mode)
{
    if (NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

    // Enter sleep by default and by explicit sleep commmand
    if (RUUVI_DRIVER_SENSOR_CFG_SLEEP == *mode || RUUVI_DRIVER_SENSOR_CFG_DEFAULT == *mode)
    {
        autorefresh = false;
        *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
        return RUUVI_DRIVER_SUCCESS;
    }

    if (RUUVI_DRIVER_SENSOR_CFG_SINGLE == *mode)
    {
        // Do nothing if sensor is in continuous mode
        uint8_t current_mode;
        ruuvi_interface_environmental_mcu_mode_get (&current_mode);

        if (RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == current_mode)
        {
            *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
            return RUUVI_DRIVER_ERROR_INVALID_STATE;
        }

        // Enter sleep after measurement
        autorefresh = false;
        *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
        // Global float is updated by sample
        nrf52832_temperature_sample();
        return RUUVI_DRIVER_SUCCESS;
    }

    if (RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS == *mode)
    {
        autorefresh = true;
        return RUUVI_DRIVER_SUCCESS;
    }

    return RUUVI_DRIVER_ERROR_INVALID_PARAM;
}

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_mode_get (uint8_t * mode)
{
    if (NULL == mode) { return RUUVI_DRIVER_ERROR_NULL; }

    if (autorefresh)
    {
        *mode = RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS;
    }

    if (!autorefresh)
    {
        *mode = RUUVI_DRIVER_SENSOR_CFG_SLEEP;
    }

    return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_environmental_mcu_data_get (
    ruuvi_driver_sensor_data_t * const p_data)
{
    if (NULL == p_data) { return RUUVI_DRIVER_ERROR_NULL; }

    if (autorefresh) { nrf52832_temperature_sample(); }

    if (!isnan (temperature))
    {
        ruuvi_driver_sensor_data_t d_environmental;
        ruuvi_driver_sensor_data_fields_t env_fields = {.bitfield = 0};
        float env_values[1];
        env_values[0] = temperature;
        env_fields.datas.temperature_c = 1;
        d_environmental.data = env_values;
        d_environmental.valid  = env_fields;
        d_environmental.fields = env_fields;
        ruuvi_driver_sensor_data_populate (p_data,
                                           &d_environmental,
                                           p_data->fields);
        p_data->timestamp_ms = tsample;
    }

    return RUUVI_DRIVER_SUCCESS;
}

#endif