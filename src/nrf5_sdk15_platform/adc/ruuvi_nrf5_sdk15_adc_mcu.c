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
#include "ruuvi_interface_adc_mcu.h"
#if RUUVI_NRF5_SDK15_ADC_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"

#include "nrf_drv_saadc.h"

#include <string.h>

#define ADC_REF_VOLTAGE_INVALID   0.0000f
#define ADC_REF_DIVIDER_INVALID   0.0000f
#define ADC_REF_VOLTAGE_IN_VOLTS  0.600f       // Internal reference voltage.
#define ADC_REF_EXT_VDD_DIV       4            // ADC divides VDD by 4 for reference.
#define ADC_PRE_SCALING_COMPENSATION_1_6 6.00f
#define ADC_PRE_SCALING_COMPENSATION_1_5 5.00f
#define ADC_PRE_SCALING_COMPENSATION_1_4 4.00f
#define ADC_PRE_SCALING_COMPENSATION_1_3 3.00f
#define ADC_PRE_SCALING_COMPENSATION_1_2 2.00f
#define ADC_PRE_SCALING_COMPENSATION_1 1.00f
#define ADC_PRE_SCALING_COMPENSATION_2 0.50f
#define ADC_PRE_SCALING_COMPENSATION_4 0.25f
#define ADC_PRE_SCALING_NUM 8

#define ADC_BITS_RESOLUTION_8 8
#define ADC_BITS_RESOLUTION_10 10
#define ADC_BITS_RESOLUTION_12 12
#define ADC_BITS_RESOLUTION_14 14
#define ADC_BITS_RESOLUTION_NUM 4

static float pre_scaling_values[ADC_PRE_SCALING_NUM] =
{
    ADC_PRE_SCALING_COMPENSATION_1_6,
    ADC_PRE_SCALING_COMPENSATION_1_5,
    ADC_PRE_SCALING_COMPENSATION_1_4,
    ADC_PRE_SCALING_COMPENSATION_1_3,
    ADC_PRE_SCALING_COMPENSATION_1_2,
    ADC_PRE_SCALING_COMPENSATION_1,
    ADC_PRE_SCALING_COMPENSATION_2,
    ADC_PRE_SCALING_COMPENSATION_4
};

static nrf_saadc_channel_config_t channel_configs[NRF_SAADC_CHANNEL_COUNT];
static nrf_saadc_channel_config_t * p_channel_configs[NRF_SAADC_CHANNEL_COUNT] =
{
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};
static bool m_adc_is_init = false;
static nrf_drv_saadc_config_t adc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;

static uint8_t bits_resolution[ADC_BITS_RESOLUTION_NUM] =
{
    ADC_BITS_RESOLUTION_8, ADC_BITS_RESOLUTION_10,
    ADC_BITS_RESOLUTION_12, ADC_BITS_RESOLUTION_14,
};

/**
 * @brief convert @ref ri_adc_oversample_t to nrf_saadc_oversample_t.
 */
static inline nrf_saadc_oversample_t ruuvi_to_nrf_oversample (const ri_adc_oversample_t
        oversample)
{
    return (nrf_saadc_oversample_t) oversample;
}

/**
 * @brief convert @ref ri_adc_resolution_t to nrf_saadc_resolution_t.
 */
static inline nrf_saadc_resolution_t ruuvi_to_nrf_resolution (const ri_adc_resolution_t
        resolution)
{
    return (nrf_saadc_resolution_t) resolution;
}

/**
 * @brief convert @ref ri_adc_resolution_t to nrf_saadc_resolution_t.
 */
static inline uint8_t nrf_to_bits_resolution (const nrf_saadc_resolution_t resolution)
{
    return bits_resolution[resolution];
}

/**
 * @brief convert @ref ri_adc_channel_t to nrf_saadc_input_t.
 */
static inline nrf_saadc_input_t ruuvi_to_nrf_channel (const ri_adc_channel_t channel)
{
    return (nrf_saadc_input_t) channel;
}

/**
 * @brief convert @ref ri_adc_vref_t to nrf_saadc_reference_t.
 */
static inline nrf_saadc_reference_t ruuvi_to_nrf_vref (const ri_adc_vref_t vref)
{
    nrf_saadc_reference_t nrfref = NRF_SAADC_REFERENCE_INTERNAL;

    switch (vref)
    {
        case RI_ADC_VREF_EXTERNAL:
            nrfref = NRF_SAADC_REFERENCE_VDD4; //!< 1/4 VDD
            break;

        case RI_ADC_VREF_INTERNAL:

        // Intentional fallthrough
        default:
            nrfref = NRF_SAADC_REFERENCE_INTERNAL;
            break;
    }

    return nrfref;
}

/**
 * @brief convert @ref ri_adc_mode_t to nrf_saadc_mode_t.
 */
static inline nrf_saadc_mode_t ruuvi_to_nrf_mode (const ri_adc_mode_t mode)
{
    return (nrf_saadc_mode_t) mode;
}

/**
 * @brief convert @ref nrf_saadc_gain_t to ri_adc_gain_t.
 */
static inline ri_adc_gain_t nrf_to_ruuvi_gain (const nrf_saadc_gain_t gain)
{
    return (ri_adc_gain_t) gain;
}

/**
 * @brief convert @ref nrf_saadc_reference_t to ri_adc_vref_t.
 */
static inline ri_adc_vref_t nrf_to_ruuvi_vref (const nrf_saadc_reference_t gain)
{
    return (ri_adc_vref_t) gain;
}

#ifdef RI_ADC_ADV_CONFIG

/**
 * @brief convert @ref nri_adc_resistor_t to nrf_saadc_resistor_t.
 */
static inline nrf_saadc_resistor_t ruuvi_to_nrf_resistor (const nri_adc_resistor_t
        resistor)
{
    return (nrf_saadc_resistor_t) resistor;
}

/**
 * @brief convert @ref ri_adc_acqtime_t to nrf_saadc_acqtime_t.
 */
static inline nrf_saadc_acqtime_t ruuvi_to_nrf_acqtime (const ri_adc_acqtime_t acqtime)
{
    return (nrf_saadc_acqtime_t) acqtime;
}

#endif

/**
 * @brief convert @ref ri_adc_gain_t to nrf_saadc_gain_t.
 */
static inline nrf_saadc_gain_t ruuvi_to_nrf_gain (const ri_adc_gain_t gain)
{
    return (nrf_saadc_gain_t) gain;
}


/**
 * @brief convert @ref raw adc value to volts.
 */
static float raw_adc_to_volts (uint8_t channel_num,
                               ri_adc_get_data_t * p_config,
                               int16_t * adc)
{
    nrf_saadc_channel_config_t * p_ch_config =
        p_channel_configs[channel_num];
    uint16_t counts = 1 << nrf_to_bits_resolution (adc_config.resolution);
    float result;

    // Only voltages referred to internal VREF are accurate.
    if (NRF_SAADC_REFERENCE_INTERNAL == p_ch_config->reference)
    {
        result = (ADC_REF_VOLTAGE_IN_VOLTS * ( (float) (*adc) / (float) counts) *
                  pre_scaling_values[ (uint8_t) nrf_to_ruuvi_gain (p_ch_config->gain)] *
                  p_config->divider);
    }
    // This relies on VDD accuracy and is at best indicative.
    else
    {
        result = (p_config->vdd * ( (float) (*adc) / (float) counts) // Raw ADC ref VDD
                  * pre_scaling_values[ (uint8_t) nrf_to_ruuvi_gain (p_ch_config->gain)] // Prescaling
                  / ADC_REF_EXT_VDD_DIV // ADC ref prescaling
                  * p_config->divider); // External divider
    }

    return result;
}

/**
 * @brief convert @ref raw adc value to ratio to VDD.
 */
static float raw_adc_to_ratio (uint8_t channel_num,
                               ri_adc_get_data_t * p_config,
                               int16_t * adc)
{
    nrf_saadc_channel_config_t * p_ch_config =
        p_channel_configs[channel_num];
    uint16_t counts = 1 << nrf_to_bits_resolution (adc_config.resolution);
    float result;

    // This relies on VDD accuracy and is at best indicative.
    if (NRF_SAADC_REFERENCE_INTERNAL == p_ch_config->reference)
    {
        // Absolute voltage
        result = (ADC_REF_VOLTAGE_IN_VOLTS * ( (float) (*adc) / (float) counts) *
                  pre_scaling_values[ (uint8_t) nrf_to_ruuvi_gain (p_ch_config->gain)] *
                  p_config->divider);
        // Divided to a ratio
        result /= p_config->vdd;
    }
    // Measurement referred to VDD.
    else
    {
        result = ( (float) (*adc) / (float) counts);
    }

    return result;
}

/**
 * @brief Function handling events from 'nrf_drv_saadc.c'.
 * No implementation needed
 *
 * @param[in] p_evt SAADC event.
 */
static void saadc_event_handler (nrf_drv_saadc_evt_t const * p_evt)
{
    if (p_evt->type == NRF_DRV_SAADC_EVT_DONE)
    {
    }
}

bool  ri_adc_is_init (void)
{
    return m_adc_is_init;
}

rd_status_t ri_adc_init (ri_adc_config_t * p_config)
{
    rd_status_t status = RD_SUCCESS;

    if (false == ri_adc_is_init())
    {
        if (NULL != p_config)
        {
            adc_config.resolution = ruuvi_to_nrf_resolution (p_config->resolution);
            adc_config.oversample = ruuvi_to_nrf_oversample (p_config->oversample);
        }

        if (NRF_SUCCESS == nrf_drv_saadc_init (&adc_config, saadc_event_handler))
        {
            m_adc_is_init = true;
            status = RD_SUCCESS;
        }
        else
        {
            status = RD_ERROR_INVALID_STATE;
        }
    }

    return status;
}

rd_status_t ri_adc_uninit (bool config_default)
{
    rd_status_t status = RD_SUCCESS;

    if (true == ri_adc_is_init())
    {
        nrf_drv_saadc_uninit();

        if (true == config_default)
        {
            nrf_drv_saadc_config_t def_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
            memcpy (&adc_config, &def_config, sizeof (nrf_drv_saadc_config_t));

            for (uint8_t i = 0; i < NRF_SAADC_CHANNEL_COUNT; i++)
            {
                p_channel_configs[i] = NULL;
            }
        }

        m_adc_is_init = false;
    }

    return status;
}

rd_status_t ri_adc_stop (uint8_t channel_num)
{
    rd_status_t status = RD_SUCCESS;

    if (true == ri_adc_is_init())
    {
        if (NRF_SAADC_CHANNEL_COUNT > channel_num)
        {
            nrf_drv_saadc_config_t def_config = NRF_DRV_SAADC_DEFAULT_CONFIG;

            if (NULL != p_channel_configs[ channel_num])
            {
                memcpy (&channel_configs[ channel_num],
                        &def_config,
                        sizeof (nrf_drv_saadc_config_t));
                p_channel_configs[ channel_num] = NULL;

                if (NRF_SUCCESS != nrf_drv_saadc_channel_uninit (channel_num))
                {
                    status = RD_ERROR_INVALID_STATE;
                }
            }
        }
        else
        {
            status = RD_ERROR_INVALID_PARAM;
        }
    }

    return status;
}

rd_status_t ri_adc_configure (uint8_t channel_num,
                              ri_adc_pins_config_t * p_pins,
                              ri_adc_channel_config_t * p_config)
{
    rd_status_t status = RD_SUCCESS;

    if (true == ri_adc_is_init())
    {
        if (true == nrf_drv_saadc_is_busy())
        {
            nrf_drv_saadc_abort();
        }

        if ( (NULL != p_pins) && (NULL != p_config))
        {
            if ( (NRF_SAADC_CHANNEL_COUNT > channel_num) &&
                    (p_channel_configs[ channel_num] == NULL))
            {
                nrf_saadc_channel_config_t ch_config;
#ifdef RI_ADC_ADV_MODE_CONFIG

                if (RI_ADC_MODE_DIFFERENTIAL == p_config->mode)
                {
                    nrf_saadc_channel_config_t def_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_DIFFERENTIAL (
                            ruuvi_to_nrf_channel (p_pins->p_pin.channel),
                            ruuvi_to_nrf_channel (p_pins->n_pin.channel));
                    memcpy (&ch_config, &def_config, sizeof (nrf_saadc_channel_config_t));
#ifdef RI_ADC_ADV_CONFIG
                    ch_config.resistor_p = ruuvi_to_nrf_resistor (p_pins->p_pin.resistor);
                    ch_config.resistor_n = ruuvi_to_nrf_resistor (p_pins->n_pin.resistor);
#endif
                }
                else
                {
#endif
                    nrf_saadc_channel_config_t def_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE (
                            ruuvi_to_nrf_channel (p_pins->p_pin.channel));
                    memcpy (&ch_config, &def_config, sizeof (nrf_saadc_channel_config_t));
#ifdef RI_ADC_ADV_CONFIG
                    ch_config.resistor_p = ruuvi_to_nrf_resistor (p_pins->p_pin.resistor);
#endif
#ifdef RI_ADC_ADV_MODE_CONFIG
                }

#endif
                ch_config.reference = ruuvi_to_nrf_vref (p_config->vref);
#ifdef RI_ADC_ADV_CONFIG
                ch_config.acq_time = ruuvi_to_nrf_acqtime (p_config->acqtime);
#else

                // Use 1/6 gain for internal reference and 1/4 gain for external reference.
                // This allows ADC to use maximum non-saturated scale.
                if (NRF_SAADC_REFERENCE_INTERNAL == ch_config.reference)
                {
                    p_config->gain = RI_ADC_GAIN1_6;
                }
                else
                {
                    p_config->gain = RI_ADC_GAIN1_4;
                }

#endif
                ch_config.gain = ruuvi_to_nrf_gain (p_config->gain);
                memcpy (&channel_configs[ channel_num],
                        &ch_config,
                        sizeof (nrf_saadc_channel_config_t));
                p_channel_configs[ channel_num] =
                    &channel_configs[ channel_num];

                for (uint8_t i = 0; i < NRF_SAADC_CHANNEL_COUNT; i++)
                {
                    if (NULL != p_channel_configs[i])
                    {
                        if (NRF_SUCCESS != nrf_drv_saadc_channel_init (i, &channel_configs[i]))
                        {
                            status |= RD_ERROR_INVALID_STATE;
                        }
                    }
                }
            }
            else
            {
                status = RD_ERROR_INVALID_PARAM;
            }
        }
        else
        {
            status = RD_ERROR_NULL;
        }
    }
    else
    {
        status = RD_ERROR_INVALID_STATE;
    }

    return status;
}

rd_status_t ri_adc_get_raw_data (uint8_t channel_num,
                                 int16_t * p_data)
{
    rd_status_t status = RD_ERROR_INVALID_STATE;
    nrf_saadc_value_t adc_buf;

    if (NRF_SUCCESS == nrf_drv_saadc_sample_convert (channel_num, &adc_buf))
    {
        (*p_data) = (int16_t) (adc_buf);
        status = RD_SUCCESS;
    }

    return status;
}

/**
 * @brief get raw adc reading.
 */
static rd_status_t nrf5_adc_get_raw (uint8_t channel_num,
                                     ri_adc_get_data_t * p_config,
                                     int16_t * const p_data)
{
    rd_status_t status = RD_SUCCESS;

    if (NULL == p_config || NULL == p_data)
    {
        status |= RD_ERROR_NULL;
    }
    else
    {
        nrf_saadc_channel_config_t * p_ch_config =
            p_channel_configs[channel_num];

        if ( (NULL == p_ch_config) ||
                (p_config->vdd == ADC_REF_VOLTAGE_INVALID) ||
                (p_config->divider == ADC_REF_DIVIDER_INVALID) ||
                (isnan (p_config->divider)) ||
                (isnan (p_config->vdd) && (RI_ADC_VREF_EXTERNAL ==
                                           nrf_to_ruuvi_vref (p_ch_config->reference))))
        {
            status |= RD_ERROR_INVALID_PARAM;
        }
        else
        {
            status |= ri_adc_get_raw_data (channel_num, p_data);
        }
    }

    return status;
}

rd_status_t ri_adc_get_data_absolute (uint8_t channel_num,
                                      ri_adc_get_data_t * p_config,
                                      float * p_data)
{
    int16_t data;
    // Input check in function.
    rd_status_t status = nrf5_adc_get_raw (channel_num, p_config, &data);

    if (RD_SUCCESS == status)
    {
        (*p_data) = raw_adc_to_volts (channel_num, p_config, &data);
    }

    return status;
}

rd_status_t ri_adc_get_data_ratio (uint8_t channel_num,
                                   ri_adc_get_data_t * p_config,
                                   float * p_data)
{
    int16_t data;
    // Input check in function.
    rd_status_t status = nrf5_adc_get_raw (channel_num, p_config, &data);

    if (RD_SUCCESS == status)
    {
        (*p_data) = raw_adc_to_ratio (channel_num, p_config, &data);
    }

    return status;
}

bool ri_adc_mcu_is_valid_ch (const uint8_t ch)
{
    return ch < NRF_SAADC_CHANNEL_COUNT;
}

#endif
