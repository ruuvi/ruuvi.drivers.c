#ifndef BLE4_ADVERTISEMENT_H
#define BLE4_ADVERTISEMENT_H
#include "ruuvi_error.h"
#include "communication.h"

//Scan response has always tag name
//Advertisement field supports only manufacturer data
typedef enum{
  NON_CONNECTABLE_NON_SCANNABLE,
  NON_CONNECTABLE_SCANNABLE,
  CONNECTABLE_SCANNABLE
}ble4_advertisement_type_t;

// Functions for setting up advertisement constants
// Each change takes effect immediately
ruuvi_status_t ble4_advertisement_set_interval(int16_t ms);
ruuvi_status_t ble4_advertisement_set_power(int8_t dbm);
ruuvi_status_t ble4_advertisement_set_type(ble4_advertisement_type_t advertisement_type);
ruuvi_status_t ble4_advertisement_set_manufacturer_id(uint16_t id);

// Functions for implementing communication api
ruuvi_status_t ble4_advertisement_init(void);
ruuvi_status_t ble4_advertisement_uninit(void);
ruuvi_status_t ble4_advertisement_process_asynchronous(void);
ruuvi_status_t ble4_advertisement_process_synchronous(void);
ruuvi_status_t ble4_advertisement_flush_tx(void);
ruuvi_status_t ble4_advertisement_message_put(ruuvi_communication_message_t* msg);

#endif