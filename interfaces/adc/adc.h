#ifndef ADC_H
#define ADC_H

#include "ruuvi_error.h"
#include "ruuvi_sensor.h"

#define ADC_INVALID RUUVI_FLOAT_INVALID

typedef enum{
  AIN_0,
  AIN_1,
  AIN_2,
  AIN_3,
  AIN_4,
  AIN_5,
  AIN_6,
  AIN_7,
  AIN_8,
  AIN_9,
  AIN_BATTERY
}adc_channel_t;

// ruuvi_status_t adc_channel_set(adc_channel_t channel);

//TODO: Sensor interface functions
ruuvi_status_t adc_init(ruuvi_sensor_t* adc);

// Start sampling given channel
ruuvi_status_t adc_sample_asynchronous(adc_channel_t channel);

// read latest data from given channel
float adc_get_data(adc_channel_t channel);


#endif