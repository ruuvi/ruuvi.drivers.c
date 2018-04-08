#ifndef BLE4_STACK_H
#define BLE4_STACK_H

#include "ruuvi_error.h"
#include <stdbool.h>

// Call this function before any other ble operation. Initializes BLE stack.
ruuvi_status_t ble4_stack_init(void);

// Sets name to softdevice. Calling this function alone won't update 
// advertised name, the effect will take place after next call to process()
ruuvi_status_t ble4_set_name(uint8_t* name, uint8_t name_length, bool include_serial);

#endif