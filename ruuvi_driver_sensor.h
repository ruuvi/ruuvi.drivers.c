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
 * @date 2019-02-17
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

#define RUUVI_DRIVER_SENSOR_INVALID_VALUE    RUUVI_DRIVER_FLOAT_INVALID  ///< Signal this float sensor is erroneous
#define RUUVI_DRIVER_SENSOR_INVALID_TIMSTAMP RUUVI_DRIVER_UINT64_INVALID ///< Signal this timestamp value is erroneous

// Constants for sensor configuration and status
#define RUUVI_DRIVER_SENSOR_CFG_DEFAULT         0      ///< Default value, always valid for the sensor. 
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
 * All sensors must implement configuration function which accepts this struct.
 */
typedef struct __attribute__((packed, aligned(4)))
{
  uint8_t samplerate;
  uint8_t resolution;
  uint8_t scale;
  uint8_t dsp_function;
  uint8_t dsp_parameter;
  uint8_t mode;
  uint8_t reserved0;
  uint8_t reserved1;
}
ruuvi_driver_sensor_configuration_t;

/**
 * Type of bus sensor uses.
 */
typedef enum
{
  RUUVI_DRIVER_BUS_NONE = 0,
  RUUVI_DRIVER_BUS_SPI  = 1,
  RUUVI_DRIVER_BUS_I2C  = 2,
  RUUVI_DRIVER_BUS_UART = 3
} ruuvi_driver_bus_t;

/**
 * Generic sensor data struct, used with ruuvi_driver_sensor_data_fp in tests.
 * It's strongly recommended to match this format in all sensor data formats.
 */
typedef struct ruuvi_driver_sensor_data_t
{
  uint64_t timestamp;
  float value0;
  float value1;
  float value2;
} ruuvi_driver_sensor_data_t;

typedef struct ruuvi_driver_sensor_t
  ruuvi_driver_sensor_t; ///< forward declaration *and* typedef

/**
 * @brief Initialize and uninitialize snesot
 * Init and uninit will setup sensor with function pointers.
 * The sensor wil be initialized to lowest power state possible
 *
 * @param[in,out] p_sensor pointer to sensor structure
 * @param[in] bus bus to use, i.r. I2C or SPI
 * @param[in] handle for the sensor, for example I2C address or SPI chip select pin
 * @return @c RUUVI_DRIVER_SUCCESS on success
 * @return @c RUUVI_DRIVER_ERROR_NULL if p_sensor is NULL
 * @return @c RUUVI_DRIVER_ERROR_NOT_FOUND if there is no response from sensor or if ID of a sensor read over bus does not match expected value
 * @return @c RUUVI_DRIVER_ERROR_SELFTEST if sensor is found but it does not pass selftest
 * @return @c RUUVI_DRIVER_ERROR_INVALID_STATE if trying to initialize sensor which already has been initialized.
 **/
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_init_fp)(ruuvi_driver_sensor_t* const
    p_sensor, const ruuvi_driver_bus_t bus, const uint8_t handle);

/**
 *  @bried Setup a parameter of a sensor.
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
 * @bried Configure sensor digital signal processing.
 * Takes DSP function and a DSP parameter as input, configured value or error code as output.
 * Modifies input parameters to actual values written on the sensor.
 * DSP functions are run on the sensor HW, not in the platform FW.
 *
 * @param[in,out] dsp_function. DSP function to run on sensor. Can be a combination of several functions.
 * @param[in,out] dsp_parameter. Parameter to DSP function(s)
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
 * @warning if sensor data is not valid for any reason, data is populated with RUUVI_DRIVER_FLOAT_INVALID.
 */
typedef ruuvi_driver_status_t (*ruuvi_driver_sensor_data_fp)(void* p_data);

/**
 * @brief Convenience function to write/read entire configuration in one call
 * Modifies input parameters to actual values written on the sensor.
 *
 * @param[in] p_sensor sensor to configure
 * @param[in,out] p_configuration Input: desired configuration. Output: configuration written to sensot.
 **/
typedef ruuvi_driver_status_t (*ruuvi_driver_configuration_fp)(
  const ruuvi_driver_sensor_t* const p_sensor,
  ruuvi_driver_sensor_configuration_t* const p_configuration);

/**
 * @brief Return number of milliseconds since the start of RTC.
 *
 * @return milliseconds since start of RTC.
 * @return RUUVI_DRIVER_UINT64T_INVALID if RTC is not running
 */
typedef uint64_t (*ruuvi_driver_sensor_timestamp_fp)(void);

/**
 * Interface to sensor.
 * Some sensors can implement additional functions, such as interrupts or FIFO reads.
 * The additional functions are defined in the interface of the sensor.
 */
typedef struct ruuvi_driver_sensor_t
{
  ruuvi_driver_sensor_init_fp   init;              ///< @ref ruuvi_driver_sensor_init_fp
  ruuvi_driver_sensor_init_fp   uninit;            ///< @ref ruuvi_driver_sensor_init_fp
  ruuvi_driver_sensor_setup_fp  samplerate_set;    ///< @ref ruuvi_driver_sensor_setup_fp
  ruuvi_driver_sensor_setup_fp
  samplerate_get;    ///< @ref ruuvi_driver_sensor_setup_fp Only gets value, input value has no effect.
  ruuvi_driver_sensor_setup_fp  resolution_set;    ///< @ref ruuvi_driver_sensor_setup_fp
  ruuvi_driver_sensor_setup_fp
  resolution_get;    ///< @ref ruuvi_driver_sensor_setup_fp Only gets value, input value has no effect.
  ruuvi_driver_sensor_setup_fp  scale_set;         ///< @ref ruuvi_driver_sensor_setup_fp
  ruuvi_driver_sensor_setup_fp
  scale_get;         ///< @ref ruuvi_driver_sensor_setup_fp Only gets value, input value has no effect.
  ruuvi_driver_sensor_setup_fp  mode_set;          ///< @ref ruuvi_driver_sensor_setup_fp
  ruuvi_driver_sensor_setup_fp
  mode_get;          ///< @ref ruuvi_driver_sensor_setup_fp Only gets value, input value has no effect.
  ruuvi_driver_sensor_dsp_fp    dsp_set;           ///< @ref ruuvi_driver_sensor_dsp_fp
  ruuvi_driver_sensor_dsp_fp
  dsp_get;           ///< @ref ruuvi_driver_sensor_dsp_fp Only gets value, input value has no effect.
  ruuvi_driver_configuration_fp configuration_set; ///< @ref ruuvi_driver_configuration_fp
  ruuvi_driver_configuration_fp
  configuration_get; ///< @ref ruuvi_driver_configuration_fp Only gets value, input value has no effect.
  ruuvi_driver_sensor_data_fp   data_get;          ///< @ref ruuvi_driver_sensor_data_fp
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
 * @brief Setup timestamping
 * Set to @c NULL to disable timestamps.
 *
 * @param[in] timestamp_fp Function pointer to @ref ruuvi_driver_sensor_timestamp_fp implementation
 * @return RUUVI_DRIVER_SUCCESS
 */
ruuvi_driver_status_t ruuvi_driver_sensor_timestamp_function_set(
  const ruuvi_driver_sensor_timestamp_fp const timestamp_fp);

/**
 * @brief Calls the timestamp function and returns its value.
 * @return milliseconds since the start of RTC
 * @return RUUVI_DRIVER_UINT64_INVALID if timestamp function is NULL
 */
uint64_t ruuvi_driver_sensor_timestamp_get(void);
/*@}*/
#endif