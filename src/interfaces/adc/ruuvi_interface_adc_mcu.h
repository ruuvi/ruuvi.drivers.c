#ifndef RUUVI_INTERFACE_ADC_MCU_H
#define RUUVI_INTERFACE_ADC_MCU_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_gpio.h"
/**
 * @addtogroup ADC
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_adc_mcu.h
 * @author Oleg Protasevich
 * @date 2019-06-03
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Interface for controlling ADC onboard MCU
 *
 *
 */

/** @brief Enable implementation selected by application */
#if RI_ADC_ENABLED
#  define RUUVI_NRF5_SDK15_ADC_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/* Analog input channels of device */
typedef enum
{
    RI_ADC_GND,           //!< GND of device.
    RI_ADC_AIN0,          //!< Channel 0 of ADC.
    RI_ADC_AIN1,          //!< Channel 1 of ADC.
    RI_ADC_AIN2,          //!< Channel 2 of ADC.
    RI_ADC_AIN3,          //!< Channel 3 of ADC.
    RI_ADC_AIN4,          //!< Channel 4 of ADC.
    RI_ADC_AIN5,          //!< Channel 5 of ADC.
    RI_ADC_AIN6,          //!< Channel 6 of ADC.
    RI_ADC_AIN7,          //!< Channel 7 of ADC.
    RI_ADC_AINVDD,        //!< Analog supply voltage.
    RI_ADC_CH_NUM         //!< Number of ADC inputs.
} ri_adc_channel_t;

/* ADC reference voltage. */
typedef enum
{
    RI_ADC_VREF_INTERNAL, //!< Internal voltage reference.
    RI_ADC_VREF_EXTERNAL  //!< External voltage reference.
} ri_adc_vref_t;

/* ADC mode */
typedef enum
{
    RI_ADC_MODE_SINGLE,       //!< Single ended mode
#ifdef RI_ADC_ADV_MODE_CONFIG
    RI_ADC_MODE_DIFFERENTIAL  //!< Differential mode
#endif
} ri_adc_mode_t;

/* ADC oversampling mode. */
typedef enum
{
    RI_ADC_OVERSAMPLE_DISABLED,
    RI_ADC_OVERSAMPLE_2X,
    RI_ADC_OVERSAMPLE_4X,
    RI_ADC_OVERSAMPLE_8X,
    RI_ADC_OVERSAMPLE_16X,
    RI_ADC_OVERSAMPLE_32X,
    RI_ADC_OVERSAMPLE_64X,
    RI_ADC_OVERSAMPLE_128X,
    RI_ADC_OVERSAMPLE_256X,
} ri_adc_oversample_t;

/* ADC resolutions. */
typedef enum
{
    RI_ADC_RESOLUTION_8BIT,
    RI_ADC_RESOLUTION_10BIT,
    RI_ADC_RESOLUTION_12BIT,
    RI_ADC_RESOLUTION_14BIT,
} ri_adc_resolution_t;

#ifdef RI_ADC_ADV_CONFIG
/* ADC pin resistor. */
typedef enum
{
    RI_ADC_RESISTOR_DISABLED,
    RI_ADC_RESISTOR_PULLDOWN,
    RI_ADC_RESISTOR_PULLUP,
    RI_ADC_RESISTOR_VDD1_2,
} nri_adc_resistor_t;

/* ADC acq_time. */
typedef enum
{
    RI_ADC_ACQTIME_3US,
    RI_ADC_ACQTIME_5US,
    RI_ADC_ACQTIME_10US,
    RI_ADC_ACQTIME_15US,
    RI_ADC_ACQTIME_20US,
    RI_ADC_ACQTIME_40US,
} ri_adc_acqtime_t;
#endif

/* ADC gain. */
typedef enum
{
    RI_ADC_GAIN1_6,
    RI_ADC_GAIN1_5,
    RI_ADC_GAIN1_4,
    RI_ADC_GAIN1_3,
    RI_ADC_GAIN1_2,
    RI_ADC_GAIN1,
    RI_ADC_GAIN2,
    RI_ADC_GAIN4,
} ri_adc_gain_t;

/* ADC pin config struct. */
typedef struct
{
    ri_adc_channel_t channel;
#ifdef RI_ADC_ADV_CONFIG
    nri_adc_resistor_t resistor;
#endif
} ri_adc_pin_config_t;

/* ADC x2 pins config struct. */
typedef struct
{
    ri_adc_pin_config_t p_pin;
#ifdef RI_ADC_ADV_MODE_CONFIG
    ri_adc_pin_config_t n_pin;
#endif
} ri_adc_pins_config_t;

/* ADC channel config struct. */
typedef struct
{
    ri_adc_mode_t mode;
    ri_adc_vref_t vref;
    ri_adc_gain_t gain;
#ifdef RI_ADC_ADV_CONFIG
    ri_adc_acqtime_t acqtime;
#endif
} ri_adc_channel_config_t;

/* ADC config struct. */
typedef struct
{
    ri_adc_oversample_t oversample;
    ri_adc_resolution_t resolution;
} ri_adc_config_t;

/* ADC result config struct. */
typedef struct
{
    float vdd;
    float divider;
} ri_adc_get_data_t;

/**
 * @brief Check if ADC is initialized.
 *
 * @retval true if ADC is initialized.
 * @retval false if ADC is not initialized.
 */
bool ri_adc_is_init (void);
/**
 * @brief Initialization of ADC
 *
 * @param[in] p_config Configuration of ADC.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC is already initialized
 *         or smth goes wrong.
 */
rd_status_t ri_adc_init (ri_adc_config_t * p_config);

/**
 * @brief Uninitialize ADC.
 *
 * @param[in] config_default Reset config to default.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC lock can't be released. Reboot.
 */
rd_status_t ri_adc_uninit (bool config_default);

/**
 * @brief Configure ADC channel.
 *
 * @param[in] channel_num ADC channel.
 * @param[in] p_pins ADC pin config.
 * @param[in] p_config ADC channel config.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC channel is locked.
 * @retval RD_ERROR_INVALID_PARAM if input channel incorrect.
 * @return RD_ERROR_NULL if either parameter is NULL.
 */
rd_status_t ri_adc_configure (uint8_t channel_num,
                              ri_adc_pins_config_t * p_pins,
                              ri_adc_channel_config_t * p_config);

/**
 * @brief Stop use ADC channel.
 *
 * @param[in] channel_num ADC channel
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC channel is locked.
 * @retval RD_ERROR_INVALID_PARAM if input channel incorrect.
 */
rd_status_t ri_adc_stop (uint8_t channel_num);

/**
 * @brief Get raw ADC data.
 *
 * @param[in] channel_num ADC channel.
 * @param[in, out] p_data raw ADC data.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC channel is locked.
 */
rd_status_t ri_adc_get_raw_data (uint8_t channel_num,
                                 int16_t * p_data);

/**
 * @brief Get ADC data in volts.
 *
 * @param[in] channel_num ADC channel.
 * @param[in] p_config ADC output config.
 * @param[out] p_data ADC data in volts.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_PARAM if input incorrect.
 * @return RD_ERROR_NULL if either parameter is NULL.
 */
rd_status_t ri_adc_get_data_absolute (uint8_t channel_num,
                                      ri_adc_get_data_t * p_config,
                                      float * p_data);

/**
 * @brief Get ADC data in volts.
 *
 * @param[in] channel_num ADC channel.
 * @param[in] p_config ADC output config.
 * @param[out] p_data ADC data as a ratio to VDD.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_PARAM if input incorrect.
 * @return RD_ERROR_NULL if either parameter is NULL.
 */
rd_status_t ri_adc_get_data_ratio (uint8_t channel_num,
                                   ri_adc_get_data_t * p_config,
                                   float * p_data);

/**
 * @brief Return true if given channel  index can be used by underlying implementation.
 *
 * @param[in] ch Channel to check.
 * @retval true Channel can be used (but might be reserved).
 * @retval false Channel is out of bounds.
 */
bool ri_adc_mcu_is_valid_ch (const uint8_t ch);
#endif
