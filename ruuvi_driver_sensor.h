#ifndef RUUVI_DRIVER_SENSOR_H
#define RUUVI_DRIVER_SENSOR_H
/**
 * @defgroup Sensor Common sensor interface
 * @brief Functions for setting up and using sensors
 *
 *
 */
/*@{*/
/**
 * @file ruuvi_driver_sensor.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-07-10
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause
 * @brief Ruuvi sensor interface
 *
 * Common interface to all Ruuvi Sensors
 * Every sensor must implement these functions:
 * - init
 * - samplerate_set
 * - samplerate_get
 * - dsp_set
 * - dsp_get
 * - scale_set
 * - scale_set
 * - resolution_set
 * - resolution_get
 * - mode_set
 * - mode_get
 * - data_get
 *
 * If function does not make sense for the sensor, it will return error code.
 *
 * INIT, UNINT: Init will prepare sensor for use, reset, run self-test and place it in low-power mode. Additionally function pointers will be set up by init.
 *              Uninit will release any resources used by sensor. Uninit NULLs the sensor function pointers.
 *
 * Samplerate: Applicable on continuous mode, how often sensor takes samples. Hz
 *
 * DSP: DSP function and parameter, i.e. "OVERSAMPLING, 16". Return error if the device does not support it.
 *
 * scale: Maximum scale in a meaningful physical unit, such as celcius or pascal.
 *
 * resolution: Resolution in bits.
 *
 * mode: Sleep, single, continuous.
 *  - Sleep mode should enter lowest-power state available
 *  - Single will return once new data is available with data_get call
 *  - Continuous: Sensor will sample at given rate. Returns immediately, data will be available after first sample
 *
 * data get: return latest sample from sensor
 */

#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RUUVI_DRIVER_SENSOR_INVALID_VALUE    RUUVI_DRIVER_FLOAT_INVALID  ///< Signal this float sensor is erroneous
#define RUUVI_DRIVER_SENSOR_INVALID_TIMSTAMP RUUVI_DRIVER_UINT64_INVALID ///< Signal this timestamp value is erroneous

// Constants for sensor configuration and status
#define RUUVI_DRIVER_SENSOR_CFG_DEFAULT         0      ///< Default value, always valid for the sensor.
#define RUUVI_DRIVER_SENSOR_CFG_CUSTOM_1        0xC9   ///< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RUUVI_DRIVER_SENSOR_CFG_CUSTOM_2        0xCA   ///< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RUUVI_DRIVER_SENSOR_CFG_CUSTOM_3        0xCB   ///< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RUUVI_DRIVER_SENSOR_CFG_CUSTOM_4        0xCC   ///< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RUUVI_DRIVER_SENSOR_CFG_CUSTOM_5        0xCD   ///< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RUUVI_DRIVER_SENSOR_CFG_CUSTOM_6        0xCE   ///< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RUUVI_DRIVER_SENSOR_ERR_INVALID         0xE0   ///< Error code, given parameter is invalid
#define RUUVI_DRIVER_SENSOR_ERR_NOT_IMPLEMENTED 0xE1   ///< Error code, given parameter is not implemented (todo)
#define RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED   0xE2   ///< Error code, given parameter is not supported by sensor
#define RUUVI_DRIVER_SENSOR_CFG_MIN             0xF0   ///< Configure smallest supported and implemented value
#define RUUVI_DRIVER_SENSOR_CFG_MAX             0xF1   ///< Configure largest supported and implemented value
#define RUUVI_DRIVER_SENSOR_CFG_SLEEP           0xF2   ///< Sensor should go to sleep immediately
#define RUUVI_DRIVER_SENSOR_CFG_SINGLE          0xF3   ///< Sensor should go to sleep after single measurement
#define RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS      0xF4   ///< Sensor will keep sampling at defined sample rate
#define RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE       0xFF   ///< Do not change configured value

// DSP functions, complemented by DSP parameter
#define RUUVI_DRIVER_SENSOR_DSP_LAST            0      ///< Return last value from sesnor. Parameter: No effect. Use default
#define RUUVI_DRIVER_SENSOR_DSP_LOW_PASS        (1<<1) ///< Low pass sensor values Parameter: coefficient
#define RUUVI_DRIVER_SENSOR_DSP_HIGH_PASS       (1<<2) ///< High pass sensor values Parameter: coefficient
#define RUUVI_DRIVER_SENSOR_DSP_OS              (1<<3) ///< Oversample sensor values. Parameter: Number of samples

/**
 * @brief All sensors must implement configuration functions which accepts this struct.
 */
typedef struct __attribute__((packed, aligned(4)))
{
  uint8_t samplerate;     //!< Samplerate, in Hz
  uint8_t resolution;     //!< Resolution, in bits
  uint8_t scale;          //!< Scale, in relevant Si-unit
  uint8_t dsp_function;   //!< DSP function, one of @c RUUVI_DRIVER_SENSOR_DSP_*
  uint8_t dsp_parameter;  //!< Parameter to DSP functions
  uint8_t mode;           //!< Mode, RUUVI_DRIVER_SENSOR_SLEEP, _SINGLE, _CONTINOUS
  uint8_t reserved0;      //!< Reserved for future use
  uint8_t reserved1;      //!< Reserved for future use
}
ruuvi_driver_sensor_configuration_t;

/**
 * @brief Type of bus sensor uses.
 */
typedef enum
{
  RUUVI_DRIVER_BUS_NONE = 0, //!< No bus, internal to IC
  RUUVI_DRIVER_BUS_SPI  = 1, //!< SPI bus
  RUUVI_DRIVER_BUS_I2C  = 2, //!< I2C bus
  RUUVI_DRIVER_BUS_UART = 3, //!< UART bus
  RUUVI_DRIVER_BUS_PDM  = 4  //!< PDM bus
} ruuvi_driver_bus_t;

/**
 * @brief Generic sensor data struct. 
 * Used with ruuvi_driver_sensor_data_fp in tests.
 * It's strongly recommended to match this format in all sensor data formats.
 */
typedef struct ruuvi_driver_sensor_data_t
{
  uint64_t timestamp; //!< Timestamp of the event, @ref ruuvi_driver_sensor_timestamp_get
  float value0;       //!< First value of sensor
  float value1;       //!< Second value of sensor
  float value2;       //!< Third value of sensor 
} ruuvi_driver_sensor_data_t;

/** @brief Forward declare type definition of sensor structure */
typedef struct ruuvi_driver_sensor_t ruuvi_driver_sensor_t; 

/**
 * @brief Initialize and uninitialize sensor.
 * Init and uninit will setup sensor with function pointers.
 * The sensor wil be initialized to lowest power state possible
 *
 * @param[in,out] p_sensor pointer to sensor structure
 * @param[in] bus bus to use, i.r. I2C or SPI
 * @param[in] handle for the sensor, for example I2C address or SPI chip select pin
 * @return @c RUUVI_DRIVER_SUCCESS on success
 * @return @c RUUVI_DRIVER_ERROR_NULL if p_sensor is NULL
 * @return @c RUUVI_DRIVER_ERROR_NOT_FOUND if there is no response from sensor or if ID of
 *            a sensor read over bus does not match expected value
 * @return @c RUUVI_DRIVER_ERROR_SELFTEST if sensor is found but it does not pass selftest
 * @return @c RUUVI_DRIVER_ERROR_INVALID_STATE if trying to initialize sensor which 
 *            already has been initialized.
 **/
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_init_fp)(ruuvi_driver_sensor_t* const
    p_sensor, const ruuvi_driver_bus_t bus, const uint8_t handle);

/** @brief convert Ruuvi GPIO into uint8_t */
#define RUUVI_DRIVER_GPIO_TO_HANDLE(handle) ((((handle) >> 3) & 0xE0) + ((handle) & 0x1F))
/** @brief convert uint8_t into Ruuvi GPIO */
#define RUUVI_DRIVER_HANDLE_TO_GPIO(handle) ((((handle) & 0xE0) << 3) + ((handle) & 0x1F))

/**
 *  @brief Setup a parameter of a sensor.
 *  The function will modify the pointed data to the actual value which was written
 *
 *  @param[in,out] parameter value to write to sensor configuration. Actual value written to sensor as output
 *  @return RUUVI_DRIVER_SUCCESS on success
 *  @return RUUVI_DRIVER_ERROR_NULL if parameter is NULL
 *  @return RUUVI_DRIVER_ERROR_NOT_SUPPORTED if sensor cannot support given parameter
 *  @return RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED if the sensor could support parameter, but it's not implemented in fw.
 **/
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_setup_fp)(uint8_t* parameter);

/**
 * @brief Configure sensor digital signal processing.
 * Takes DSP function and a DSP parameter as input, configured value or error code as output.
 * Modifies input parameters to actual values written on the sensor.
 * DSP functions are run on the sensor HW, not in the platform FW.
 *
 * @param[in,out] dsp_function. DSP function to run on sensor. Can be a combination of several functions.
 * @param[in,out] dsp_parameter. Parameter to DSP function(s)
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_NULL if either parameter is NULL
 * @return RUUVI_DRIVER_ERROR_NOT_SUPPORTED if sensor doesn't support given DSP
 * @return RUUVI_DRIVER_ERROR_NOT_IMPLEMENTED if sensor supports given DSP, but
 *         driver does not implement it
 * @return RUUVI_DRIVER_ERROR_INVALID_PARAM if parameter is invalid for any reason.
 **/
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_dsp_fp)(uint8_t* dsp_function,
    uint8_t* dsp_parameter);

/**
 * @brief Read latest data from sensor registers
 * Return latest data from sensor. Does not take a new sample, calling this function twice
 * in a row returns same data. Configure sensor in a single-shot mode to take a new sample
 * or leave sensor in a continuous mode to get updated data.
 *
 * @param [out] p_data Pointer to sensor data @ref ruuvi_driver_sensor_data_t .
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_NULL if p_data is @c NULL.
 *
 * @warning if sensor data is not valid for any reason, data is populated with 
 *          @c RUUVI_DRIVER_FLOAT_INVALID.
 */
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_data_fp)(void* p_data);

/**
 * @brief Convenience function to write/read entire configuration in one call.
 * Modifies input parameters to actual values written on the sensor.
 *
 * @param[in] p_sensor sensor to configure
 * @param[in,out] p_configuration Input: desired configuration. Output: 
 *                configuration written to sensot.
 **/
typedef ruuvi_driver_status_t (*ruuvi_driver_configuration_fp)(
  const ruuvi_driver_sensor_t* const p_sensor,
  ruuvi_driver_sensor_configuration_t* const p_configuration);

/**
* @brief Read First-in-first-out (FIFO) buffer
* Reads up to num_elements data points from FIFO and populates pointer data with them
*
* @param[in, out] num_elements Input: number of elements in data. 
                               Output: Number of elements placed in data
* @param[out] data array of ruuvi_interface_acceleration_data_t with num_elements slots.
* @return RUUVI_DRIVER_SUCCESS on success
* @return RUUVI_DRIVER_ERROR_NULL if either parameter is NULL
* @return RUUVI_DRIVER_ERROR_INVALID_STATE if FIFO is not in use
* @return error code from stack on error.
*/
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_fifo_read_fp)(size_t* num_elements,
    ruuvi_driver_sensor_data_t* data);

/**
* @brief Enable FIFO or FIFO interrupt full interrupt on sensor.
* FIFO interrupt Triggers as ACTIVE HIGH interrupt once FIFO is filled. 
* It is responsibility of application to know the routing of GPIO pins and
* configure the GPIO to register interrupts.
*
* @param[in] enable True to enable interrupt, false to disable interrupt
* @return RUUVI_DRIVER_SUCCESS on success, error code from stack otherwise.
**/
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_fifo_enable_fp)(const bool enable);

/**
* @brief Enable level interrupt on sensor.
* Triggers as ACTIVE HIGH interrupt while detected data is above threshold.
* Trigger is symmetric, i.e. thershold is valid for above positive or below negative 
* of given value.
* On accelerometer data is high-passed to filter out gravity.
* Axes are examined individually, compound data won't trigger the interrupt.
* It is responsibility of application to know the GPIO routing and register
* GPIO interrupts. 
*
* @param[in] enable  True to enable interrupt, false to disable interrupt
* @param[in,out] limit_g: Input: Desired acceleration to trigger the interrupt.
*                         Is considered as "at least", the acceleration is rounded up to 
*                         next value.
*                         Output: written with value that was set to interrupt
* @return RUUVI_DRIVER_SUCCESS on success
* @return RUUVI_DRIVER_INVALID_STATE if data limit is higher than maximum scale
* @return error code from stack on error.
*
*/
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_level_interrupt_use_fp)(const bool enable,
    float* limit_g);

/**
 * @brief Return number of milliseconds since the start of RTC.
 *
 * @return milliseconds since start of RTC.
 * @return RUUVI_DRIVER_UINT64T_INVALID if RTC is not running
 */
typedef uint64_t (*ruuvi_driver_sensor_timestamp_fp)(void);

/**
 * @brief Interface to sensor.
 * Some sensors can implement additional functions.
 * The additional functions are defined in the interface of the sensor.
 */
typedef struct ruuvi_driver_sensor_t
{
  /** @brief @ref ruuvi_driver_sensor_init_fp */
  ruuvi_driver_sensor_init_fp   init;  
  /** @brief @ref ruuvi_driver_sensor_init_fp */            
  ruuvi_driver_sensor_init_fp   uninit;            
  /** @brief @ref ruuvi_driver_sensor_setup_fp */
  ruuvi_driver_sensor_setup_fp samplerate_set; 
  /** @brief @ref ruuvi_driver_sensor_setup_fp */
  ruuvi_driver_sensor_setup_fp samplerate_get;
  /** @brief @ref ruuvi_driver_sensor_setup_fp */
  ruuvi_driver_sensor_setup_fp resolution_set;
  /** @brief @ref ruuvi_driver_sensor_setup_fp */
  ruuvi_driver_sensor_setup_fp resolution_get;
  /** @brief @ref ruuvi_driver_sensor_setup_fp */
  ruuvi_driver_sensor_setup_fp scale_set;
  /** @brief @ref ruuvi_driver_sensor_setup_fp */
  ruuvi_driver_sensor_setup_fp scale_get;
  /** @brief @ref ruuvi_driver_sensor_setup_fp */
  ruuvi_driver_sensor_setup_fp mode_set;
  /** @brief @ref ruuvi_driver_sensor_setup_fp */
  ruuvi_driver_sensor_setup_fp mode_get;
  /** @brief @ref ruuvi_driver_sensor_dsp_fp */
  ruuvi_driver_sensor_dsp_fp   dsp_set;
  /** @brief @ref ruuvi_driver_sensor_dsp_fp */
  ruuvi_driver_sensor_dsp_fp   dsp_get;
  /** @brief @ref ruuvi_driver_configuration_fp */
  ruuvi_driver_configuration_fp configuration_set;
  /** @brief @ref ruuvi_driver_configuration_fp */
  ruuvi_driver_configuration_fp configuration_get;
  /** @brief @ref ruuvi_driver_sensor_data_fp */
  ruuvi_driver_sensor_data_fp   data_get;        
  /** @brief @®ef ruuvi_driver_sensor_fifo_enable_fp */
  ruuvi_driver_sensor_fifo_enable_fp fifo_enable;
  /** @brief @®ef ruuvi_driver_sensor_level_interrupt_use_fp */
  ruuvi_driver_sensor_fifo_enable_fp fifo_interrupt_enable;
  /** @brief @®ef ruuvi_driver_sensor_level_interrupt_use_fp */
  ruuvi_driver_sensor_fifo_read_fp   fifo_read;
  /** @brief @®ef ruuvi_driver_sensor_level_interrupt_use_fp */
  ruuvi_driver_sensor_level_interrupt_use_fp level_interrupt_set;
} ruuvi_driver_sensor_t;

/**
 * @brief implementation of ref ruuvi_driver_configuration_fp
 */
ruuvi_driver_status_t ruuvi_driver_sensor_configuration_set(const ruuvi_driver_sensor_t*
    sensor, ruuvi_driver_sensor_configuration_t* config);

/**
 * @brief implementation of ref ruuvi_driver_configuration_fp
 */
ruuvi_driver_status_t ruuvi_driver_sensor_configuration_get(const ruuvi_driver_sensor_t*
    sensor, ruuvi_driver_sensor_configuration_t* config);

/**
 * @brief Dummy implementations of FIFO function for sensors which don't implement them.
 *
 * Only reason to have this function is to avoid NULL pointer errors.   
 *
 * @param[in] enable no effect
 * @return @ref RUUVI_DRIVER_ERROR_NOT_SUPPORTED
 */
ruuvi_driver_status_t ruuvi_driver_dummy_fifo_enable(const bool enable);
/**
 * @brief Dummy implementations of FIFO function for sensors which don't implement them.
 *
 * Only reason to have this function is to avoid NULL pointer errors.   
 *
 * @param[in] enable no effect
 * @return @ref RUUVI_DRIVER_ERROR_NOT_SUPPORTED
 */
ruuvi_driver_status_t ruuvi_driver_dummy_fifo_interrupt_enable(const bool enable);
/**
 * @brief Dummy implementations of FIFO function for sensors which don't implement them.
 *
 * Only reason to have this function is to avoid NULL pointer errors.   
 *
 * @param[in] num_elements no effect
 * @param[in] data no effect
 * @return @ref RUUVI_DRIVER_ERROR_NOT_SUPPORTED
 */
ruuvi_driver_status_t ruuvi_driver_dummy_fifo_read(size_t* num_elements, ruuvi_driver_sensor_data_t* data);
/**
 * @brief Dummy implementation of level function for sensors which don't implement them.
 *
 * Only reason to have this function is to avoid NULL pointer errors.   
 *
 * @param[in] enable no effect
 * @param[im] limit_g no effect
 * @return @ref RUUVI_DRIVER_ERROR_NOT_SUPPORTED
 */
ruuvi_driver_status_t ruuvi_driver_dummy_level_interrupt_set(const bool enable, float* limit_g);

/**
 * @brief Setup timestamping
 * Set to @c NULL to disable timestamps.
 *
 * @param[in] timestamp_fp Function pointer to @ref ruuvi_driver_sensor_timestamp_fp implementation
 * @return RUUVI_DRIVER_SUCCESS
 */
ruuvi_driver_status_t ruuvi_driver_sensor_timestamp_function_set(
  const ruuvi_driver_sensor_timestamp_fp  timestamp_fp);

/**
 * @brief Calls the timestamp function and returns its value.
 * @return milliseconds since the start of RTC
 * @return RUUVI_DRIVER_UINT64_INVALID if timestamp function is NULL
 */
uint64_t ruuvi_driver_sensor_timestamp_get(void);
/*@}*/
#endif