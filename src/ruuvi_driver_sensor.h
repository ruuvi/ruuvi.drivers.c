#ifndef RUUVI_DRIVER_SENSOR_H
#define RUUVI_DRIVER_SENSOR_H
/**
 * @defgroup Sensor Common sensor interface
 * @brief Functions for setting up and using sensors
 *
 *
 */
/** @{ */
/**
 * @file ruuvi_driver_sensor.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-06-01
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause
 * @brief Ruuvi sensor interface <b>Lifecycle: Beta</b>
 *
 *
 *
 * Common interface to all Ruuvi Sensors
 * Every sensor must implement these functions:
 * - init
 * - uninit
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
 * Return name: Return a pointer to a constant 8-byte long string which represensts sensor, e.g. LIS2DH12\0 or BME280\0\0
 *
 * INIT, UNINT: Init will prepare sensor for use, reset the sensor, run self-test and place it in low-power mode. Additionally function pointers will be set up by init.
 *              Uninit will release any resources used by sensor
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

#define RD_SENSOR_INVALID_VALUE    RD_FLOAT_INVALID  //!< Signal this sensor value is erroneous
#define RD_SENSOR_INVALID_TIMSTAMP RD_UINT64_INVALID //!< Signal this timestamp value is erroneous

// Constants for sensor configuration and status
#define RD_SENSOR_CFG_DEFAULT         (0U)      //!< Default value, always valid for the sensor.
#define RD_SENSOR_CFG_CUSTOM_1        (0xC9U)   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_2        (0xCAU)   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_3        (0xCBU)   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_4        (0xCCU)   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_5        (0xCDU)   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_6        (0xCEU)   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_ERR_INVALID         (0xE0U)   //!< Error code, given parameter is invalid
#define RD_SENSOR_ERR_NOT_IMPLEMENTED (0xE1U)   //!< Error code, given parameter is not implemented (todo)
#define RD_SENSOR_ERR_NOT_SUPPORTED   (0xE2U)   //!< Error code, given parameter is not supported by sensor
#define RD_SENSOR_CFG_MIN             (0xF0U)   //!< Configure smallest supported and implemented value
#define RD_SENSOR_CFG_MAX             (0xF1U)   //!< Configure largest supported and implemented value
#define RD_SENSOR_CFG_SLEEP           (0xF2U)   //!< Sensor should go to sleep immediately
#define RD_SENSOR_CFG_SINGLE          (0xF3U)   //!< Sensor should go to sleep after single measurement
#define RD_SENSOR_CFG_CONTINUOUS      (0xF4U)   //!< Sensor will keep sampling at defined sample rate
#define RD_SENSOR_CFG_NO_CHANGE       (0xFFU)   //!< Do not change configured value

// DSP functions, complemented by DSP parameter
#define RD_SENSOR_DSP_LAST            (0U)    //!< Return last value from sensor. Parameter: No effect. Use default
#define RD_SENSOR_DSP_LOW_PASS        (1U<<1U) //!< Low pass sensor values Parameter: coefficient
#define RD_SENSOR_DSP_HIGH_PASS       (1U<<2U) //!< High pass sensor values Parameter: coefficient
#define RD_SENSOR_DSP_OS              (1U<<3U) //!< Oversample sensor values. Parameter: Number of samples

/** @brief convert Ruuvi GPIO into uint8_t */
#define RD_GPIO_TO_HANDLE(handle) ((((handle) >> 3U) & 0xE0U) + ((handle) & 0x1FU))
/** @brief convert uint8_t into Ruuvi GPIO */
#define RD_HANDLE_TO_GPIO(handle) ((((handle) & 0xE0U) << 3U) + ((handle) & 0x1FU))
/** @brief Mark sensor as unused with this handle */
#define RD_HANDLE_UNUSED (0xFFU)

#ifndef UNUSED_VARIABLE
#   define UNUSED_VARIABLE(X)  ((void)(X))
#endif

/**
 * @brief All sensors must implement configuration functions which accept this struct.
 */
typedef struct __attribute__ ( (packed, aligned (4)))
{
    uint8_t samplerate;     //!< Samplerate, in Hz
    uint8_t resolution;     //!< Resolution, in bits
    uint8_t scale;          //!< Scale, in relevant Si-unit
    uint8_t dsp_function;   //!< DSP function, one of @c RD_SENSOR_DSP_*
    uint8_t dsp_parameter;  //!< Parameter to DSP functions
    uint8_t mode;           //!< Mode, RD_SENSOR_SLEEP, _SINGLE, _CONTINOUS
    uint8_t reserved0;      //!< Reserved for future use
    uint8_t reserved1;      //!< Reserved for future use
}
rd_sensor_configuration_t;

/**
 * @brief Type of bus sensor uses.
 */
typedef enum
{
    RD_BUS_NONE = 0U, //!< No bus, internal to IC
    RD_BUS_SPI  = 1U, //!< SPI bus
    RD_BUS_I2C  = 2U, //!< I2C bus
    RD_BUS_UART = 3U, //!< UART bus
    RD_BUS_PDM  = 4U, //!< PDM bus
    RD_BUS_FAIL = 5U  //!< Test behaviour on invalid bus with this value.
} rd_bus_t;

/**
 * @brief Bitfield to describe related sensor data
 */
typedef struct
{
    unsigned int acceleration_x_g : 1; //!< Acceleration along X-axis, gravities.
    unsigned int acceleration_y_g : 1; //!< Acceleration along Y-axis, gravities.
    unsigned int acceleration_z_g : 1; //!< Acceleration along Z-axis, gravities.
    unsigned int co2_ppm : 1;          //!< CO2, Parts per million.
    unsigned int gyro_x_dps : 1;       //!< Rotation along X-axis, degrees per second.
    unsigned int gyro_y_dps : 1;       //!< Rotation along Y-axis, degrees per second.
    unsigned int gyro_z_dps : 1;       //!< Rotation along Z-axis, degrees per second.
    unsigned int humidity_rh : 1;      //!< Relative humidity, %.
    /** @brief Light level, dimensionless. Comparable only between identical devices. */
    unsigned int luminosity  : 1;
    unsigned int magnetometer_x_g : 1; //!< Magnetic flux along X-axis, Gauss.
    unsigned int magnetometer_y_g : 1; //!< Magnetic flux along Y-axis, Gauss.
    unsigned int magnetometer_z_g : 1; //!< Magnetic flux along Z-axis, Gauss.
    unsigned int pm_1_ugm3 : 1;        //!< Ultra-fine particulate matter, microgram per m^3.
    unsigned int pm_2_ugm3 : 1;        //!< Fine particulate matter, microgram per m^3.
    unsigned int pm_4_ugm3 : 1;        //!< Medium particulate matter, microgram per m^3.
    unsigned int pm_10_ugm3 : 1;       //!< Coarse particulate matter, microgram per m^3.
    unsigned int pressure_pa : 1;      //!< Pressure, pascals
    unsigned int spl_dbz : 1;          //!< Unweighted sound pressure level.
    unsigned int temperature_c : 1;    //!< Temperature, celcius
    unsigned int voc_ppm : 1;          //!< Volatile organic compounds, parts per million.
    unsigned int voltage_v : 1;        //!< Voltage, volts.
    unsigned int voltage_ratio : 1;    //!< Voltage, ratio to maximum
    unsigned int reserved: 10;         //!< Reserved bits, force remainder of bitfield to 0.
} rd_sensor_data_bitfield_t;

/**
 * C99 Standard 6.7.8.21
 * If there are fewer initializers in a brace-enclosed list than there are
 * elements or members of an aggregate, or fewer characters in a string literal
 * used to initialize an array of known size than there are elements in the array,
 * the remainder of the aggregate shall be initialized implicitly the same as
 * objects that have static storage duration.
 */
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_ACC_X_FIELD ((rd_sensor_data_fields_t){.datas.acceleration_x_g=1})
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_ACC_Y_FIELD ((rd_sensor_data_fields_t){.datas.acceleration_y_g=1})
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_ACC_Z_FIELD ((rd_sensor_data_fields_t){.datas.acceleration_z_g=1})
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_GYR_X_FIELD ((rd_sensor_data_fields_t){.datas.gyro_x_dps=1})
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_GYR_Y_FIELD ((rd_sensor_data_fields_t){.datas.gyro_y_dps=1})
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_GYR_Z_FIELD ((rd_sensor_data_fields_t){.datas.gyro_z_dps=1})
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_HUMI_FIELD ((rd_sensor_data_fields_t){.datas.humidity_rh=1})
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_PRES_FIELD ((rd_sensor_data_fields_t){.datas.pressure_pa=1})
/** @brief Shorthand for calling rd_sensor_data_parse(p_data, FIELD) */
#define RD_SENSOR_TEMP_FIELD ((rd_sensor_data_fields_t){.datas.temperature_c=1})



/**
 * @brief Union to access sensor data.
 *
 * MISRA deviation: Use of union.
 * Union is used here for fast operations on sensor data through field bitfield
 * and to give a meaningful value to each bit through datas.
 *
 * C99 and onwards allow type punning, but this is not portable to C++.
 * Run the integration tests on your platform.
 *
 */
typedef union // -V2514
{
    uint32_t bitfield; //!< Bitfield used to access sensor data.
    rd_sensor_data_bitfield_t datas; //!< Structured data field.
} rd_sensor_data_fields_t;

/**
 * @brief Generic sensor data struct.
 *
 * The data sensor struct contains a timestamp relative to sensor boot,
 * a list of fields contained within the sensor data and a pointer to array
 * of floats which contain the actual data.
 */
typedef struct rd_sensor_data_t
{
    uint64_t timestamp_ms;      //!< Timestamp of the event, @ref rd_sensor_timestamp_get.
    rd_sensor_data_fields_t
    fields; //!< Description of datafields which may be contained in this sample.
    rd_sensor_data_fields_t valid;  //!< Listing of valid data in this sample.
    /** @brief Data of sensor. Must contain as many elements as fields has bits set. */
    float * data;
} rd_sensor_data_t;

/** @brief Forward declare type definition of sensor structure */
typedef struct rd_sensor_t rd_sensor_t;

/**
 * @brief Initialize and uninitialize sensor.
 * Init and uninit will setup sensor with function pointers.
 * The sensor wil be initialized to lowest power state possible.
 *
 * @param[in,out] p_sensor pointer to sensor structure
 * @param[in] bus bus to use, i.r. I2C or SPI
 * @param[in] handle for the sensor, for example I2C address or SPI chip select pin
 * @return @c RD_SUCCESS on success
 * @return @c RD_ERROR_NULL if p_sensor is NULL
 * @return @c RD_ERROR_NOT_FOUND if there is no response from sensor or if ID of
 *            a sensor read over bus does not match expected value
 * @return @c RD_ERROR_SELFTEST if sensor is found but it does not pass selftest
 * @return @c RD_ERROR_INVALID_STATE if trying to initialize sensor which
 *            already has been initialized.
 **/
typedef rd_status_t (*rd_sensor_init_fp) (rd_sensor_t * const
        p_sensor, const rd_bus_t bus, const uint8_t handle);

/**
 *  @brief Setup a parameter of a sensor.
 *  The function will modify the pointed data to the actual value which was written
 *
 *  @param[in,out] parameter value to write to sensor configuration. Actual value written to sensor as output
 *  @return RD_SUCCESS on success
 *  @return RD_ERROR_NULL if parameter is NULL
 *  @return RD_ERROR_NOT_SUPPORTED if sensor cannot support given parameter
 *  @return RD_ERROR_NOT_IMPLEMENTED if the sensor could support parameter, but it's not implemented in fw.
 **/
typedef rd_status_t (*rd_sensor_setup_fp) (uint8_t * parameter);

/**
 * @brief Configure sensor digital signal processing.
 * Takes DSP function and a DSP parameter as input, configured value or error code as output.
 * Modifies input parameters to actual values written on the sensor.
 * DSP functions are run on the sensor HW, not in the platform FW.
 *
 * @param[in,out] dsp_function. DSP function to run on sensor. Can be a combination of several functions.
 * @param[in,out] dsp_parameter. Parameter to DSP function(s)
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if either parameter is NULL
 * @return RD_ERROR_NOT_SUPPORTED if sensor doesn't support given DSP
 * @return RD_ERROR_NOT_IMPLEMENTED if sensor supports given DSP, but
 *         driver does not implement it
 * @return RD_ERROR_INVALID_PARAM if parameter is invalid for any reason.
 **/
typedef rd_status_t (*rd_sensor_dsp_fp) (uint8_t * dsp_function,
        uint8_t * dsp_parameter);

/**
 * @brief Read latest data from sensor registers
 * Return latest data from sensor. Does not take a new sample, calling this function twice
 * in a row returns same data. Configure sensor in a single-shot mode to take a new sample
 * or leave sensor in a continuous mode to get updated data.
 *
 * p_data may contain, some, none or all of fields sensor is able to provide.
 * Fields which are already marked as valid will not be overwritten, filled fields
 * will get marked as valid.
 *
 * @param [out] p_data Pointer to sensor data @ref rd_sensor_data_t .
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if p_data is @c NULL.
 *
 */
typedef rd_status_t (*rd_sensor_data_fp) (rd_sensor_data_t * const p_data);

/**
 * @brief Convenience function to write/read entire configuration in one call.
 * Modifies input parameters to actual values written on the sensor.
 *
 * @param[in] p_sensor sensor to configure
 * @param[in,out] p_configuration Input: desired configuration. Output:
 *                configuration written to sensot.
 * @retval RD_SUCCESS if sensor was configured successfully.
 * @retval RD_ERROR_NULL if one of parameters is NULL
 * @return Error code from driver on other error.
 **/
typedef rd_status_t (*rd_configuration_fp) (
    const rd_sensor_t * const p_sensor,
    rd_sensor_configuration_t * const p_configuration);

/**
* @brief Read First-in-first-out (FIFO) buffer in sensor.
* Reads up to num_elements data points from FIFO and populates pointer data with them.
*
* @param[in, out] num_elements Input: number of elements in data.
                               Output: Number of elements placed in data.
* @param[out] Data array of  with num_elements slots.
* @retval RD_SUCCESS on success.
* @retval RD_ERROR_NULL if either parameter is NULL.
* @retval RD_ERROR_INVALID_STATE if FIFO is not in use.
* @retval RD_ERROR_NOT_SUPPORTED if the sensor does not have FIFO.
* @return error code from stack on error.
*/
typedef rd_status_t (*rd_sensor_fifo_read_fp) (size_t * const num_elements,
        rd_sensor_data_t * const data);

/**
* @brief Enable FIFO or FIFO interrupt full interrupt on sensor.
* FIFO interrupt Triggers an interrupt once FIFO is filled.
* It is responsibility of application to know the routing of and polarity of GPIO pins and
* configure the GPIO to register interrupts.
*
* @param[in] enable True to enable interrupt, false to disable interrupt
* @return RD_SUCCESS on success, error code from stack otherwise.
**/
typedef rd_status_t (*rd_sensor_fifo_enable_fp) (const bool enable);

/**
* @brief Enable level interrupt on sensor.
*
* Triggers as ACTIVE HIGH interrupt while detected data is above threshold.
*
* Trigger is symmetric, i.e. threshold is valid for above positive or below negative
* of given value.
*
* On accelerometer data is high-passed to filter out gravity.
* Axes are examined individually, compound data won't trigger the interrupt. e.g.
* accelerometer showing 0.8 G along X, Y, Z axes won't trigger at threshold of 1 G,
* even though the vector sum of axes is larger than 1 G.
*
* It is responsibility of application to know the GPIO routing and register
* GPIO interrupts.
*
* @param[in] enable  True to enable interrupt, false to disable interrupt
* @param[in,out] limit_g: Input: Desired acceleration to trigger the interrupt.
*                         Is considered as "at least", the acceleration is rounded up to
*                         next value.
*                         Output: written with value that was set to interrupt
* @retval RD_SUCCESS on success.
* @retval RD_INVALID_STATE if data limit is higher than maximum scale.
* @return error code from stack on error.
*
*/
typedef rd_status_t (*rd_sensor_level_interrupt_use_fp) (const bool enable,
        float * limit_g);

/**
 * @brief Return number of milliseconds since the start of RTC.
 *
 * @return milliseconds since start of RTC.
 * @return RD_UINT64T_INVALID if RTC is not running
 */
typedef uint64_t (*rd_sensor_timestamp_fp) (void);

/**
 * @brief Interface to sensor.
 * Some sensors can implement additional functions.
 * The additional functions are defined in the interface of the sensor.
 */
typedef struct rd_sensor_t
{
    /** @brief Sensor human-readable name. Should be at most 8 bytes long. */
    const char * name;
    /** @brief handle for sensor internal context */
    void * p_ctx;
    /** @brief Description of data fields the sensor is able to provide. */
    rd_sensor_data_fields_t provides;
    /** @brief @ref rd_sensor_init_fp */
    rd_sensor_init_fp  init;
    /** @brief @ref rd_sensor_init_fp */
    rd_sensor_init_fp  uninit;
    /** @brief @ref rd_sensor_setup_fp */
    rd_sensor_setup_fp samplerate_set;
    /** @brief @ref rd_sensor_setup_fp */
    rd_sensor_setup_fp samplerate_get;
    /** @brief @ref rd_sensor_setup_fp */
    rd_sensor_setup_fp resolution_set;
    /** @brief @ref rd_sensor_setup_fp */
    rd_sensor_setup_fp resolution_get;
    /** @brief @ref rd_sensor_setup_fp */
    rd_sensor_setup_fp scale_set;
    /** @brief @ref rd_sensor_setup_fp */
    rd_sensor_setup_fp scale_get;
    /** @brief @ref rd_sensor_setup_fp */
    rd_sensor_setup_fp mode_set;
    /** @brief @ref rd_sensor_setup_fp */
    rd_sensor_setup_fp mode_get;
    /** @brief @ref rd_sensor_dsp_fp */
    rd_sensor_dsp_fp   dsp_set;
    /** @brief @ref rd_sensor_dsp_fp */
    rd_sensor_dsp_fp   dsp_get;
    /** @brief @ref rd_configuration_fp */
    rd_configuration_fp configuration_set;
    /** @brief @ref rd_configuration_fp */
    rd_configuration_fp configuration_get;
    /** @brief @ref rd_sensor_data_fp */
    rd_sensor_data_fp   data_get;
    /** @brief @®ef rd_sensor_fifo_enable_fp */
    rd_sensor_fifo_enable_fp fifo_enable;
    /** @brief @®ef rd_sensor_level_interrupt_use_fp */
    rd_sensor_fifo_enable_fp fifo_interrupt_enable;
    /** @brief @®ef rd_sensor_level_interrupt_use_fp */
    rd_sensor_fifo_read_fp   fifo_read;
    /** @brief @®ef rd_sensor_level_interrupt_use_fp */
    rd_sensor_level_interrupt_use_fp level_interrupt_set;
} rd_sensor_t;

/**
 * @brief Implementation of ref rd_configuration_fp
 */
rd_status_t rd_sensor_configuration_set (const rd_sensor_t *
        sensor, rd_sensor_configuration_t * config);

/**
 * @brief Implementation of ref rd_configuration_fp
 */
rd_status_t rd_sensor_configuration_get (const rd_sensor_t *
        sensor, rd_sensor_configuration_t * config);

/**
 * @brief Setup timestamping.
 * Set to @c NULL to disable timestamps.
 *
 * @param[in] timestamp_fp Function pointer to @ref rd_sensor_timestamp_fp implementation
 * @retval RD_SUCCESS.
 */
rd_status_t rd_sensor_timestamp_function_set (
    const rd_sensor_timestamp_fp  timestamp_fp);

/**
 * @brief Calls the timestamp function and returns its value.
 * @return milliseconds since the start of RTC.
 * @retval RD_UINT64_INVALID if timestamp function is NULL
 */
uint64_t rd_sensor_timestamp_get (void);

/**
 * @brief Initialize sensor struct with non-null pointers which
 *        return RD_ERROR_NOT_INITIALIZED.
 *
 * This function is to ensure that NULL function pointers won't be called.
 * If name was NULL before calling this, name will point to "NOTINIT".
 * If name was already set, it won't be changed.
 *
 * @param[out] p_sensor pointer to sensor struct to initialize.
 */
void rd_sensor_initialize (rd_sensor_t * const p_sensor);

/**
 * @brief Mark sensor as uninitialized by calling the generic initialization.
 * Will not clear the name of the sensor.
 *
 * @param[out] p_sensor pointer to sensor struct to uninitialize.
 */
void rd_sensor_uninitialize (rd_sensor_t * const p_sensor);

/**
 * @brief Check if given sensor structure is already initialized.
 *
 * @param[in] sensor Sensor interface to check.
 * @return true if structure is initialized, false otherwise.
 */
bool rd_sensor_is_init (const rd_sensor_t * const sensor);

/**
 * @brief Populate given target data with data provided by sensor as requested.
 *
 * This function looks up the appropriate assigments on each data field in given target
 * and populates it with provided data if caller requested the field to be populated.
 * Populated fields are marked as valid.
 *
 * Example: Board can have these sensors in this order of priority:
 *  - TMP117 (temperature)
 *  - SHTC3 (temperature, humidity)
 *  - DPS310 (temperature, pressure)
 *  - LIS2DH12 (acceleration, temperature)
 *
 * If a target with fields for temperature, humidity, pressure and acceleration is
 * created and populated from data of the sensors end result will be:
 *
 * -> Temperature, timestamp from TMP117
 * -> Humidity from SHTC3
 * -> Pressure from DPS310
 * -> Acceleration from LIS2DH12
 *
 * If same firmware is run on a board with only LIS2DH12 populated, end result will be
 *
 * -> Temperature, timestamp, acceleration from LIS2DH12
 * -> RD_FLOAT_INVALID on humidity and pressure.
 *
 * @param[out] target Data to be populated. Fields must be initially populated with
 *                    RD_FLOAT_INVALID.
 * @param[in]  provided Data provided by sensor.
 * @param[in]  requested Fields to be filled if possible.
 */
void rd_sensor_data_populate (rd_sensor_data_t * const target,
                              const rd_sensor_data_t * const provided,
                              const rd_sensor_data_fields_t requested);

/**
 * @brief Parse data from provided struct.
 *
 * @param[in]  provided Data to be parsed.
 * @param[in]  requested One data field to be parsed.
 * @return     sensor value if found, RD_FLOAT_INVALID if the provided data didn't
 *             have a valid value.
 */
float rd_sensor_data_parse (const rd_sensor_data_t * const provided,
                            const rd_sensor_data_fields_t requested);

/**
 * @brief Count number of floats required for this data structure.
 *
 * @param[in]  target Structure to count number of fields from.
 * @return     Number of floats required to store the sensor data.
 */
uint8_t rd_sensor_data_fieldcount (const rd_sensor_data_t * const target);

/**
 * @brief Set a desired value to target data.
 *
 * This function looks up the appropriate assigments on each data field in given target
 * and populates it with provided data. Does nothing if there is no appropriate slot
 * in target data.
 *
 * This is a shorthand for @ref rd_sensor_data_populate for only one data field, without
 * setting timestamp.
 *
 * @param[out] target
 * @param[in]  field  Quantity to set, exactly one must be set to true.
 * @param[in]  value  Value of quantity,
 */
void rd_sensor_data_set (rd_sensor_data_t * const target,
                         const rd_sensor_data_fields_t field,
                         const float value);

/**
 * @brief Validate that given setting can be set on a sensor which supports only default value.
 *
 * @param[in,out] input Input: Must be RD_SENSOR_CFG_DEFAULT, _NO_CHANGE, _MIN or _MAX.
 *                      Output: _DEFAULT
 * @param[in] mode Mode sensor is currently in. Must be sleep to configure sensor.
 */
rd_status_t validate_default_input_set (uint8_t * const input, const uint8_t mode);

/**
 * @brief Validate and get input when only allowed value is default.
 *
 * @param[out] input Setting of sensor to get. Will be RD_SENSOR_CFG_DEFAULT.
 *
 * @retval RD_SUCCESS if input is not NULL.
 * @retval RD_ERROR_NULL if input is NULL.
 */
rd_status_t validate_default_input_get (uint8_t * const input);

/**
 * @brief Check if sensor has valid data at given index.
 *
 * Data is considered valid if target->fields and target->valid both are set.
 * Index is referred to number of fields.
 *
 * Typical usage:
 * @code
 * const uint8_t fieldcount = rd_sensor_data_fieldcount(p_data);
 * for(uint8_t ii = 0; ii < fieldcount; ii++)
 * {
 *     if(rd_sensor_has_valid_data(p_data, ii)
 *     {
 *        do_stuff(p_data->data[ii], rd_sensor_field_type(p_data, ii));
 *     }
 * }
 * @endcode
 *
 * @param[in] target Pointer to data to check.
 * @param[in] index index of data to check.
 * @retval true If data at target->data[index] has a valid value.
 * @retval false If target is NULL, index is higher than fields in data or data at
 *               index is not marked as valid.
 *
 * @note To determine the type of data, use @ref rd_sensor_field_type.
 */
bool rd_sensor_has_valid_data (const rd_sensor_data_t * const target,
                               const uint8_t index);

/**
 * @brief Check the type of data at given index.
 *
 * This function is used to determine what type of data given index has.
 *
 * Typical usage:
 * @code
 * rd_sensor_data_bitfield_t type = rd_sensor_field_type(p_data, index);
 * if(1 == type.temperature_c)
 * {
 *    do_stuff_with_temperature (p_data->data[index])
 * }
 * @endcode
 *
 * @param[in] target Data to check
 * @param[in] index  Index of field to check.
 * @return rd_sensor_data_bitfield_t with field corresponding to index set, or 0 if
 *                                   target doesn't have any data type at given index.
 */
rd_sensor_data_bitfield_t rd_sensor_field_type (const rd_sensor_data_t * const target,
        const uint8_t index);

/** @} */
#endif
