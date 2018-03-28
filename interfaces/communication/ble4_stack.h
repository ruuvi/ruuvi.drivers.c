#ifndef BLE4_STACK_H
#define BLE4_STACK_H

#include "ruuvi_error.h"

ruuvi_status_t ble_stack_init(void);
ruuvi_status_t set_advertisement_tag_name(uint8_t* name, uint8_t name_length);

#endif