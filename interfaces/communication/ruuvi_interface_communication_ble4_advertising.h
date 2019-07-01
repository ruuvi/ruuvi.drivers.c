/**
 * Ruuvi BLE data advertising.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_COMMUNICATION_BLE4_ADVERTISING_H
#define RUUVI_INTERFACE_COMMUNICATION_BLE4_ADVERTISING_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"
#include <stdint.h>

/*
 * Initializes radio hardware, advertising module and scanning module
 *
 * Returns RUUVI_DIRVER_SUCCESS on success, RUUVI_DIRVER_ERROR_INVALID_STATE if radio is already initialized
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_init(
  ruuvi_interface_communication_t* const channel);

/*
 * Uninitializes radio hardware, advertising module and scanning module
 *
 * Returns RUUVI_DIRVER_SUCCESS on success or if radio was not initialized.
 * Returns RUUVI_DRIVER_ERROR_INVALID_STATE if radio hardware was initialized by another radio module.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_uninit(
  ruuvi_interface_communication_t* const channel);

/*
 * Setter for broadcast advertisement interval.
 *
 * parameter ms: Milliseconds, random delay will be added to the interval to avoid collisions. min 100 ms, max 10 000 ms.
 * returns RUUVI_DRIVER_SUCCESS on success, RUUVI_DRIVER_ERROR_INVALID_PARAM if the parameter is outside allowed range
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_tx_interval_set(
  const uint32_t ms);

/**
 * Getter for broadcast advertisement interval
 *
 * parameter ms: Output. Milliseconds between transmission, without the random delay.
 * return RUUVI_DRIVER_SUCCESS on success, RUUVI_DRIVER_ERROR_INVALID_STATE if advertisement module is not initialized.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_tx_interval_get(
  uint32_t* ms);

ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_manufacturer_id_set(
  const uint16_t id);

// Set manufacturer specific data to advertise. Clears previous data.
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_data_set(
  const uint8_t* data, const uint8_t data_length);

/**
 * @brief Start or stop advertising GATT connection.
 *
 * This function configures flags of BLE advertisement as connectable. 
 * It does not start advertising itself. 
 * Data goes to the scan response. 
 * Power can be saved by setting name as NULL and advertise_nus as false as the scan responses are not required.
 * 
 * @param[in] connectable True to start advertising connectablity. False to stop advertising connectablity
 * @param[in] name Name of the device to be advertised. Can be at most 10 bytes + trailing NULL.
 * @param[in] advertise_nus True to enable advertising UUID of NUS in the scan response.
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return error code from stack in case there is  error.
 *
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_connectablity(
  const bool connectable, const char* const name,
  const bool advertise_nus);

/**
 * Send data as manufacturer specific data payload.
 * If no new data is placed to the buffer, last message sent will be repeated.
 *
 * Returns RUUVI_DRIVER_SUCCESS if the data was placed in buffer
 * Returns RUUVI_DRIVER_ERROR_INVALID_LENGTH if data length is over 24 bytes
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_send(
  ruuvi_interface_communication_message_t* message);

// Set / get radio tx power
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_tx_power_set(
  int8_t* dbm);
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_tx_power_get(
  int8_t* dbm);

// Not implemented
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_receive(
  ruuvi_interface_communication_message_t* message);

// Not implemented
//ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_rx_interval_set(uint32_t* window_interval_ms, uint32_t* window_size_ms);
//ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_rx_interval_get(uint32_t* window_interval_ms, uint32_t* window_size_ms);

/**
 * Event handler for radio activity interrupts. This is called by ruuvi_platform_interface_radio.c event, application should
 * not call this function directly.
 *
 * parameter evt: Radio activity event.
 */
void ruuvi_interface_communication_ble4_advertising_activity_handler(
  const ruuvi_interface_communication_radio_activity_evt_t evt);

#endif