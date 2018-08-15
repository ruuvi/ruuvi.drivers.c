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
 * mode: Sleep, single, continuous.
 *  - Sleep mode should enter lowest-power state available
 *  - Single will return once new data is available with data_get call
 *  - Continuous: Sensor will sample at given rate. Returns immediately
 *
 * data get: return data from sensor, either latest sample of FIFO buffer
 */

#ifndef RUUVI_SENSOR_H
#define RUUVI_SENSOR_H
#include "ruuvi_driver_error.h"

// Constants for sensor configuration and status
#define RUUVI_DRIVER_SENSOR_CFG_DEFAULT         0
#define RUUVI_DRIVER_SENSOR_ERR_INVALID         0xE0
#define RUUVI_DRIVER_SENSOR_ERR_NOT_IMPLEMENTED 0xE1
#define RUUVI_DRIVER_SENSOR_ERR_NOT_SUPPORTED   0xE2
#define RUUVI_DRIVER_SENSOR_ERR_INVALID         0xE0
#define RUUVI_DRIVER_SENSOR_CFG_MIN             0xF0
#define RUUVI_DRIVER_SENSOR_CFG_MAX             0xF1
#define RUUVI_DRIVER_SENSOR_CFG_SLEEP           0xF2   // Sensor should go to sleep after single measurement
#define RUUVI_DRIVER_SENSOR_CFG_SINGLE          0xF3   // Sensor should go to sleep after single measurement
#define RUUVI_DRIVER_SENSOR_CFG_CONTINUOUS      0xF4
#define RUUVI_DRIVER_SENSOR_CFG_ON_DRDY         0xF5   // Data ready
#define RUUVI_DRIVER_SENSOR_CFG_ON_INTERRUPT    0xF6   // Configuring interrupts is not yet supported.
#define RUUVI_DRIVER_SENSOR_CFG_NO_CHANGE       0xFF

#define RUUVI_DRIVER_SENSOR_DSP_LAST            0      // Parameter: No effect
#define RUUVI_DRIVER_SENSOR_DSP_LOW_PASS        (1<<1) // Parameter: coefficient
#define RUUVI_DRIVER_SENSOR_DSP_HIGH_PASS       (1<<2) // Parameter: coefficient
#define RUUVI_DRIVER_SENSOR_DSP_IIR             (1<<3) // Parameter: coefficient
#define RUUVI_DRIVER_SENSOR_DSP_OS              (1<<4) // Parameter: Number of samples

// Declare function pointers common to all sensors
typedef struct ruuvi_driver_sensor_t ruuvi_driver_sensor_t;          // forward declaration *and* typedef

// Init and uninit will setup our sensor with function pointers.
typedef ruuvi_driver_status_t(*ruuvi_driver_sensor_init_fp)(ruuvi_driver_sensor_t*);

// Setters, getters. Setter will modify the pointed value and put actual value on data in function.
// getter will put data on pointed value.
typedef ruuvi_driver_status_t(*ruuvi_driver_sensor_samplerate_fp)(uint8_t*);
typedef ruuvi_driver_status_t(*ruuvi_driver_sensor_resolution_fp)(uint8_t*);
typedef ruuvi_driver_status_t(*ruuvi_driver_sensor_scale_fp)(uint8_t*);

// DSP function and a DSP parameter as input, configured value or error code as output.
typedef ruuvi_driver_status_t(*ruuvi_driver_sensor_dsp_fp)(uint8_t*, uint8_t*);
typedef ruuvi_driver_status_t(*ruuvi_driver_sensor_mode_fp)(uint8_t*);

// Void pointer to sensor-specific struct which gets filled with data
typedef ruuvi_driver_status_t(*ruuvi_driver_sensor_data_fp)(void*);

// Typedef is above
struct ruuvi_driver_sensor_t{
  ruuvi_driver_sensor_init_fp init;
  ruuvi_driver_sensor_init_fp uninit;
  ruuvi_driver_sensor_samplerate_fp samplerate_set;
  ruuvi_driver_sensor_samplerate_fp samplerate_get;
  ruuvi_driver_sensor_resolution_fp resolution_set;
  ruuvi_driver_sensor_resolution_fp resolution_get;
  ruuvi_driver_sensor_scale_fp      scale_set;
  ruuvi_driver_sensor_scale_fp      scale_get;
  ruuvi_driver_sensor_dsp_fp        dsp_set;
  ruuvi_driver_sensor_dsp_fp        dsp_get;
  ruuvi_driver_sensor_mode_fp       mode_set;
  ruuvi_driver_sensor_mode_fp       mode_get;
  // Return latest measurement (or FIFO buffer) fetched from sensor at the time of calling this function.
  ruuvi_driver_sensor_data_fp       data_get;
};

#endif