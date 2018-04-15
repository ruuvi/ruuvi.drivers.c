#ifndef IMU_H
#define IMU_H
#include "ruuvi_error.h"
#include "acceleration.h"
#include "gyration.h"

#define IMU_INVALID RUUVI_FLOAT_INVALID

typedef struct {
  ruuvi_acceleration_data_t acceleration;
  ruuvi_gyration_data_t     gyration;
  float timestamp_ms;
}ruuvi_imu_data_t;

#endif