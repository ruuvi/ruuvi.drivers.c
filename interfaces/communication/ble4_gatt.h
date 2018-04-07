/* 
 * BLE GATT implements Ruuvi communication via Nordic UART Service, NUS.
 *
 * Additional services, such as Device Information Service and DFU Services can. e initialized.
 */
#ifndef BLE4_GATT_H
#define BLE4_GATT_H



#include "ruuvi_error.h"
#include "communication.h"

#include <stdbool.h>

// Functions for implementing communication api
ruuvi_status_t ble4_nus_init(void);
ruuvi_status_t ble4_nus_uninit(void);
bool ble4_nus_is_connected(void);
ruuvi_status_t ble4_nus_process_asynchronous(void);
ruuvi_status_t ble4_nus_process_synchronous(void);
ruuvi_status_t ble4_nus_flush_tx(void);
ruuvi_status_t ble4_nus_flush_rx(void);
ruuvi_status_t ble4_nus_message_put(ruuvi_communication_message_t* msg);
ruuvi_status_t ble4_nus_message_get(ruuvi_communication_message_t* msg);

// Other GATT services
ruuvi_status_t ble4_gatt_init(void);
ruuvi_status_t ble4_dis_init(void);
ruuvi_status_t ble4_dfu_init(void);

#endif