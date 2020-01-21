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
 * @date 2019-10-10
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
#define RD_SENSOR_CFG_DEFAULT         0      //!< Default value, always valid for the sensor.
#define RD_SENSOR_CFG_CUSTOM_1        0xC9   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_2        0xCA   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_3        0xCB   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_4        0xCC   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_5        0xCD   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_CFG_CUSTOM_6        0xCE   //!< Configuration range is 0...200, i.e. 0 ... 0xC8. Use C9 ... CF as sensor-specific values.
#define RD_SENSOR_ERR_INVALID         0xE0   //!< Error code, given parameter is invalid
#define RD_SENSOR_ERR_NOT_IMPLEMENTED 0xE1   //!< Error code, given parameter is not implemented (todo)
#define RD_SENSOR_ERR_NOT_SUPPORTED   0xE2   //!< Error code, given parameter is not supported by sensor
#define RD_SENSOR_CFG_MIN             0xF0   //!< Configure smallest supported and implemented value
#define RD_SENSOR_CFG_MAX             0xF1   //!< Configure largest supported and implemented value
#define RD_SENSOR_CFG_SLEEP           0xF2   //!< Sensor should go to sleep immediately
#define RD_SENSOR_CFG_SINGLE          0xF3   //!< Sensor should go to sleep after single measurement
#define RD_SENSOR_CFG_CONTINUOUS      0xF4   //!< Sensor will keep sampling at defined sample rate
#define RD_SENSOR_CFG_NO_CHANGE       0xFF   //!< Do not change configured value

// DSP functions, complemented by DSP parameter
#define RD_SENSOR_DSP_LAST            0      //!< Return last value from sensor. Parameter: No effect. Use default
#define RD_SENSOR_DSP_LOW_PASS        (1<<1) //!< Low pass sensor values Parameter: coefficient
#define RD_SENSOR_DSP_HIGH_PASS       (1<<2) //!< High pass sensor values Parameter: coefficient
#define RD_SENSOR_DSP_OS              (1<<3) //!< Oversample sensor values. Parameter: Number of samples

/** @brief convert Ruuvi GPIO into uint8_t */
#define RD_GPIO_TO_HANDLE(handle) ((((handle) >> 3) & 0xE0) + ((handle) & 0x1F))
/** @brief convert uint8_t into Ruuvi GPIO */
#define RD_HANDLE_TO_GPIO(handle) ((((handle) & 0xE0) << 3) + ((handle) & 0x1F))

/**
 * @brief All sensors must implement configuration functions which accepts this struct.
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
    RD_BUS_NONE = 0, //!< No bus, internal to IC
    RD_BUS_SPI  = 1, //!< SPI bus
    RD_BUS_I2C  = 2, //!< I2C bus
    RD_BUS_UART = 3, //!< UART bus
    RD_BUS_PDM  = 4, //!< PDM bus
    RD_BUS_FAIL = 5  //!< Test behaviour on invalid bus with this value.
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
unsigned int luminosity  :
    1;      //!< Light level, dimensionless. Comparable only between identical devices.
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
} rd_sensor_data_bitfield_t;

typedef union
{
    uint32_t bitfield;
    rd_sensor_data_bitfield_t datas;
} rd_sensor_data_fields_t;

/**
 * @brief Generic sensor data struct.
 */
typedef struct rd_sensor_data_t
{
    uint64_t timestamp_ms;          //!< Timestamp of the event, @ref rd_sensor_timestamp_get.
    rd_sensor_data_fields_t
    fields; //!< Description of datafields which may be contained in this sample.
    rd_sensor_data_fields_t valid;  //!< Listing of valid data in this sample.
    float * data;                   //!< Data of sensor.
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
 * @param [out] p_data Pointer to sensor data @ref rd_sensor_data_t .
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if p_data is @c NULL.
 *
 * @warning if sensor data is not valid for any reason, data is populated with
 *          @c RD_FLOAT_INVALID.
 */
typedef rd_status_t (*rd_sensor_data_fp) (rd_sensor_data_t * const p_data);

/**
 * @brief Convenience function to write/read entire configuration in one call.
 * Modifies input parameters to actual values written on the sensor.
 *
 * @param[in] p_sensor sensor to configure
 * @param[in,out] p_configuration Input: desired configuration. Output:
 *                configuration written to sensot.
 **/
typedef rd_status_t (*rd_configuration_fp) (
    const rd_sensor_t * const p_sensor,
    rd_sensor_configuration_t * const p_configuration);

/**
* @brief Read First-in-first-out (FIFO) buffer
* Reads up to num_elements data points from FIFO and populates pointer data with them
*
* @param[in, out] num_elements Input: number of elements in data.
                               Output: Number of elements placed in data
* @param[out] data array of ruuvi_interface_acceleration_data_t with num_elements slots.
* @return RD_SUCCESS on success
* @return RD_ERROR_NULL if either parameter is NULL
* @return RD_ERROR_INVALID_STATE if FIFO is not in use
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
* @return RD_SUCCESS on success
* @return RD_INVALID_STATE if data limit is higher than maximum scale
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
    /** @brief sensor human-readable name. Should be at most 8 bytes long. */
    const char * name;
    /** @brief Description of data fields the sensor is able to provide. */
    rd_sensor_data_fields_t provides;
    /** @brief @ref rd_sensor_init_fp */
    rd_sensor_init_fp   init;
    /** @brief @ref rd_sensor_init_fp */
    rd_sensor_init_fp   uninit;
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
 * @brief implementation of ref rd_configuration_fp
 */
rd_status_t rd_sensor_configuration_set (const rd_sensor_t *
        sensor, rd_sensor_configuration_t * config);

/**
 * @brief implementation of ref rd_configuration_fp
 */
rd_status_t rd_sensor_configuration_get (const rd_sensor_t *
        sensor, rd_sensor_configuration_t * config);

/**
 * @brief Setup timestamping
 * Set to @c NULL to disable timestamps.
 *
 * @param[in] timestamp_fp Function pointer to @ref rd_sensor_timestamp_fp implementation
 * @return RD_SUCCESS
 */
rd_status_t rd_sensor_timestamp_function_set (
    const rd_sensor_timestamp_fp  timestamp_fp);

/**
 * @brief Calls the timestamp function and returns its value.
 * @return milliseconds since the start of RTC
 * @return RD_UINT64_INVALID if timestamp function is NULL
 */
uint64_t rd_sensor_timestamp_get (void);

/**
 * @brief Initialize sensor struct with non-null pointers which return RD_ERROR_NOT_INITIALIZED
 *
 * @param[out] p_sensor pointer to sensor struct to initialize.
 */
void rd_sensor_initialize (rd_sensor_t * const p_sensor);

/**
 * @brief Mark sensor as uninitialized by calling the generic initialization.
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
 * @param[out] target Data to be populated.
 * @param[in]  provided Data provided by sensor.
 * @param[in]  requested Fields to be filled if possible.
 */
void rd_sensor_data_populate (rd_sensor_data_t * const target,
                              const rd_sensor_data_t * const provided,
                              const rd_sensor_data_fields_t requested);

/**
 * @brief Parse data from provided struct
 *
 * This function looks up the appropriate assigments on each data field in given target
 * and populates it with provided data if caller requested the field to be populated.
 *
 * @param[out] target Data to be populated.
 * @param[in]  provided Data provided by sensor.
 * @param[in]  requested Data to be parsed if possible
 * @return     sensor value if found, RD_FLOAT_INVALID if the provided data didn't have a valid value.
 */
float rd_sensor_data_parse (const rd_sensor_data_t * const provided,
                            const rd_sensor_data_fields_t requested);

/**
 * @brief count number of floats required for this data structure
 *
 * This function looks up the appropriate assigments on each data field in given target
 * and populates it with provided data if caller requested the field to be populated.
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
 * @param[in]  target
 * @param[in]  field  Quantity to set, exactly one must be set to true.
 * @param[in]  value  Value of quantity
 */
void rd_sensor_data_set (rd_sensor_data_t * const target,
                         const rd_sensor_data_fields_t field,
                         const float value);
/*@}*/
#endif