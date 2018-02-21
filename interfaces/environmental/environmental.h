#ifndef ENVIRONMENTAL_H
#define ENVIRONMENTAL_H
#include "ruuvi_error.h"

typedef struct
{
  float temperature; // C
  float humidity;    // RH-%
  float pressure;    // Pa
}environmental_data_t;
//TODO: Fallback mechanism, i.e. BME280 not responding -> use nRF52

/**
 * Setup driver, initialize sensor, run self-test
 */
ret_code_t environmental_init(void);

ret_code_t environmental_samplerate_set(ruuvi_standard_message_t* message);

ret_code_t environmental_resolution_set(ruuvi_standard_message_t* message);

ret_code_t environmental_dsp_set(ruuvi_standard_message_t* message);

ret_code_t environmental_scale_set(ruuvi_standard_message_t* message);

/** 
 * Fill message->payload with environmental data type 
 **/
ret_code_t environmental_data_get(ruuvi_standard_message_t* message);

/**
 *  Ruuvi message handler
 */
ret_code_t environmental_configure(ruuvi_standard_message_t* message);

#endif