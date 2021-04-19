#ifndef RUUVI_TASK_ADC_H
#define RUUVI_TASK_ADC_H

/** @{ */
/**
 * @addtogroup adc_tasks ADC tasks
 * @brief Internal Analog-to-digital converter control.
 *
 */
/** @} */
/**
 * @addtogroup adc_tasks
 */
/** @{ */
/**
 * @file ruuvi_task_adc.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-20-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Read ADC. Important: The ADC peripheral is shared by many different functions
 * onboard, so always uninitialize the ADC after use to let others use the peripheral.
 * Likewise important: The ADC might be in use while you need it, so <b>always</b> check
 * the return code from init and <b>do no block</b> if you can't get ADC lock.
 *
 * ADC lock must be atomic, and it requires the ri_atomic implementation.
 *
 * Typical usage:
 *
 * @code{.c}
 *  rd_status_t err_code = RD_SUCCESS;
 *  rd_sensor_configuration_t vdd_adc_configuration =
 *  {
 *    .dsp_function  = APPLICATION_ADC_DSP_FUNC,
 *    .dsp_parameter = APPLICATION_ADC_DSP_PARAM,
 *    .mode          = APPLICATION_ADC_MODE,
 *    .resolution    = APPLICATION_ADC_RESOLUTION,
 *    .samplerate    = APPLICATION_ADC_SAMPLERATE,
 *    .scale         = APPLICATION_ADC_SCALE
 *  };
 *  float battery_value;
 *  err_code = rt_adc_init();
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  err_code |= rt_adc_configure_se(&vdd_adc_configuration, RI_ADC_AINVDD, ABSOLUTE);
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  err_code |= rt_adc_sample();
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  err_code |= rt_adc_voltage_get(&battery_value);
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  err_code = rt_adc_uninit();
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 * @endcode
 */

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_log.h"

typedef enum
{
    RATIOMETRIC,    //!< ADC compares value to VDD
    ABSOLUTE        //!< ADC measures absolute voltage in volts
} rt_adc_mode_t;  //!< ADC against absolute reference or ratio to VDD

/**
 * @brief Reserve ADC
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC is already initialized.
 */
rd_status_t rt_adc_init (void);

/**
 * @brief Uninitialize ADC to release it for other users.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_FATAL if ADC lock can't be released. Reboot.
 */
rd_status_t rt_adc_uninit (void);

/**
 * @brief Check if ADC is initialized.
 *
 * @retval true if ADC is initialized.
 * @retval false if ADC is not initialized.
 */
bool rt_adc_is_init (void);

/**
 * @brief Configure ADC before sampling
 *
 * This function readies the ADC for sampling.
 * Configuring the ADC may take some time (< 1 ms) while actual sample must be as fast
 * as possible to catch transients.
 *
 * <b>Note:</b> ADC should be configured to sleep or continuous mode. To take a single sample,
 * call @ref rt_adc_sample after configuration. Configuring ADC into single sample mode is
 * equivalent to calling @ref rt_adc_sample and configuring ADC into sleep immediately.
 *
 * @param[in,out] config Configuration of ADC.
 * @param[in] handle Handle to ADC, i.e. ADC pin.
 * @param[in] mode sampling mode, @ref rt_adc_mode_t.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC is not initialized or if it is already configured.
 */
rd_status_t rt_adc_configure_se (rd_sensor_configuration_t * const config,
                                 const uint8_t handle, const rt_adc_mode_t mode);

/**
 * @brief Take a new sample on ADC configured in single-shot/sleep mode
 *
 * If this function returns RD_SUCCESS new sample can be immediately read
 * with rt_adc_voltage_get or rt_adc_ratio_get
 *
 * @retval RD_SUCCESS Sampling was successful
 * @retval RD_ERROR_INVALID_STATE ADC is not initialized or configured
 */
rd_status_t rt_adc_sample (void);

/**
 * @brief Populate data with latest sample.
 *
 * The data is absolute voltage relative to device ground.
 *
 * @param[in] data Data which has a field for absolute ADC value
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if ADC is not initialized or configured.
 * @retval error code from stack on error.
 */
rd_status_t rt_adc_voltage_get (rd_sensor_data_t * const data);

/**
 * @brief Populate data with latest ratiometric value.
 *
 * The data is ratio between 0.0 (gnd) and 1.0 (VDD). However the implementation is
 * allowed to return negative values and values higher than 1.0 if the real voltage is
 * beyond the supply rails or if differential sample is negative.
 *
 * @param[in] data Data which has a field for ratiometric ADC value
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if ADC is not initialized or configured.
 * @retval error code from stack on error.
 */
rd_status_t rt_adc_ratio_get (rd_sensor_data_t * const data);


/**
 * @brief Prepare for sampling VDD
 *
 * This function should be called before entering energy intensive activity, such as using radio to transmit data.
 * After calling this function ADC is primed for measuring the voltage droop of battery. On ADC configuration error,
 * uninitialize ADC.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if ADC is already initialized.
 */
rd_status_t rt_adc_vdd_prepare (rd_sensor_configuration_t * const vdd_adc_configuration);

/**
 * @brief Sample VDD
 *
 * This function should be called as soon as possible after energy intensive activity.
 * After a successful call value returned by @ref rt_adc_vdd_get is updated and ADC is released.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if rt_adc_vdd_prepare wasn't called.
 */
rd_status_t rt_adc_vdd_sample (void);

/**
 * @brief Get VDD
 *
 * This function should be called any time after @ref rt_adc_vdd_sample.
 * The value returned will remain fixed until next call to @ref rt_adc_vdd_sample.
 *
 * @param[out] vdd VDD voltage in volts.
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if rt_adc_vdd_sample wasn't called.
 */
rd_status_t rt_adc_vdd_get (float * const vdd);

/**
 * @brief Get absolute Voltage Sample from selected ADC handle
 *
 * This function initializes ADC, reads absolute voltage sample from selected handle.
 * The parameter sample is absolute voltage relative to the device ground.
 *
 * @param[in] configuration Configuration of ADC.
 * @param[in] handle Handle to ADC, i.e. ADC pin.
 * @param[in] sample Pointer to a Voltage Sample.
 * @retval RD_SUCCESS on success
 * @retval error code from stack on error.
 */
rd_status_t rt_adc_absolute_sample (rd_sensor_configuration_t * const configuration,
                                    const uint8_t handle, float * const sample);

/**
 * @brief Get ratiometric VDD Sample from selected ADC handle
 *
 * This function initializes ADC, reads ratiometric voltage sample from selected handle.
 * The parameter sample is a ration of voltage pin, relative to VDD, varying between
 * 0.0 (GND) and 1.0 (VDD). Additionally the parameter sample could be outside limits as
 * described in @ref rt_adc_ratio_get.
 *
 * @param[in] configuration Configuration of ADC.
 * @param[in] handle Handle to ADC, i.e. ADC pin.
 * @param[in] sample Pointer to a Voltage Sample.
 * @retval RD_SUCCESS on success
 * @retval error code from stack on error.
 */
rd_status_t rt_adc_ratiometric_sample (rd_sensor_configuration_t * const configuration,
                                       const uint8_t handle, float * const sample);
/** @} */
#endif // TASK_ADC_H