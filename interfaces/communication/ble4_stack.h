#ifndef BLE4_STACK_H
#define BLE4_STACK_H

#include "ruuvi_error.h"

ruuvi_status_t ble4_stack_init(void);
ruuvi_status_t ble4_set_name(uint8_t* name, uint8_t name_length);

#endif