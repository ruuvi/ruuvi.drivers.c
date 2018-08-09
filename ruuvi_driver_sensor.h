/*
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
 * INIT, UNINT: Init will prepare sensor for use, reset, run self-test and place it in low-power mode
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
 * mode: Sleep, single_blocking, single_asynchronous, continuous.
 *  - Sleep mode should enter lowest-power state available
 *  - Single_blocking will return once new data is available with data_get call
 *  - Single_asynchronous will start a measurement and return immediately if possible. Asynch function is allowed to block.
 *  - Continuous: Sensor will sample at given rate.
 *
 * data get: return data from sensor
 */

#ifndef RUUVI_SENSOR_H
#define RUUVI_SENSOR_H
#include "ruuvi_driver_error.h"



typedef enum {
  RUUVI_DRIVER_SENSOR_SAMPLERATE_STOP          = 0,   //Stop sampling
  RUUVI_DRIVER_SENSOR_SAMPLERATE_SINGLE        = 251, //Take a single sample
  RUUVI_DRIVER_SENSOR_SAMPLERATE_MIN           = 252, //Minimum
  RUUVI_DRIVER_SENSOR_SAMPLERATE_MAX           = 253, //Maximum
  RUUVI_DRIVER_SENSOR_SAMPLERATE_NO_CHANGE     = 255
}ruuvi_driver_sensor_samplerate_t;

/**
 * Resolution. Other values are bits
 */
typedef enum {
  RUUVI_DRIVER_SENSOR_RESOLUTION_MIN        = 252,
  RUUVI_DRIVER_SENSOR_RESOLUTION_MAX        = 253,
  RUUVI_DRIVER_SENSOR_RESOLUTION_NO_CHANGE  = 255
}ruuvi_driver_sensor_resolution_t;

/**
 * Scale. Other values are in physical unit, i.e. gravity or volt
 */
typedef enum {
  RUUVI_DRIVER_SENSOR_SCALE_MIN        = 252,
  RUUVI_DRIVER_SENSOR_SCALE_MAX        = 253,
  RUUVI_DRIVER_SENSOR_SCALE_NO_CHANGE  = 255
}ruuvi_driver_sensor_scale_t;

/**
 *. DSP function
 */
typedef enum {
  RUUVI_DRIVER_SENSOR_DSP_LAST      = 0,      // Parameter: No effect
  RUUVI_DRIVER_SENSOR_DSP_LOW_PASS  = (1<<1), // Parameter: coefficient
  RUUVI_DRIVER_SENSOR_DSP_HIGH_PASS = (1<<2), // Parameter: coefficient
  RUUVI_DRIVER_SENSOR_DSP_IIR       = (1<<3), // Parameter: coefficient
  RUUVI_DRIVER_SENSOR_DSP_OS        = (1<<4)  // Parameter: Number of samples
}ruuvi_driver_sensor_dsp_function_t;

typedef enum {
  RUUVI_DRIVER_SENSOR_MODE_SLEEP               = 0,
  RUUVI_DRIVER_SENSOR_MODE_SINGLE_BLOCKING     = 1,
  RUUVI_DRIVER_SENSOR_MODE_SINGLE_ASYNCHRONOUS = 2,
  RUUVI_DRIVER_SENSOR_MODE_CONTINOUS           = 3
}ruuvi_driver_sensor_mode_t;



// Declare function pointers common to all sensors
//Forward declare struct
typedef struct ruuvi_sensor_t ruuvi_sensor_t;          // forward declaration *and* typedef
// Init and uninit take no parameters
typedef ruuvi_status_t(*ruuvi_sensor_init_fp)(ruuvi_sensor_t*);
// Setters, getters, sent to sensor on set, copied to pointer on get
typedef ruuvi_status_t(*ruuvi_sensor_samplerate_fp)(ruuvi_sensor_samplerate_t*);
typedef ruuvi_status_t(*ruuvi_sensor_resolution_fp)(ruuvi_sensor_resolution_t*);
typedef ruuvi_status_t(*ruuvi_sensor_scale_fp)(ruuvi_sensor_scale_t*);

// DSP function and a DSP parameter as input, configured value or error code as output.
typedef ruuvi_status_t(*ruuvi_sensor_dsp_fp)(ruuvi_sensor_dsp_function_t*, uint8_t*);
typedef ruuvi_status_t(*ruuvi_sensor_mode_fp)(ruuvi_sensor_mode_t*);

// Void pointer to sensor-specific struct which gets filled with data
typedef ruuvi_status_t(*ruuvi_sensor_data_fp)(void*);

struct ruuvi_sensor_t
{
  ruuvi_sensor_init_fp init;
  ruuvi_sensor_init_fp uninit;
  ruuvi_sensor_samplerate_fp samplerate_set;
  ruuvi_sensor_samplerate_fp samplerate_get;
  ruuvi_sensor_resolution_fp resolution_set;
  ruuvi_sensor_resolution_fp resolution_get;
  ruuvi_sensor_scale_fp      scale_set;
  ruuvi_sensor_scale_fp      scale_get;
  ruuvi_sensor_dsp_fp        dsp_set;
  ruuvi_sensor_dsp_fp        dsp_get;
  ruuvi_sensor_mode_fp       mode_set;
  ruuvi_sensor_mode_fp       mode_get;
  // Return single, latest measurement, fetched from sensor.
  ruuvi_sensor_data_fp       data_get;
};

/**
 *  Configuration message for a sensor, fits into std message payoad
 */
typedef struct {
  uint8_t samplerate;
  uint8_t resolution;
  uint8_t scale;
  uint8_t dsp_parameter
  ruuvi_driver_sensor_dsp_function_t dsp_function;
  ruuvi_driver_sensor_mode_t mode;
}ruuvi_driver_sensor_configuration_t;

#endif