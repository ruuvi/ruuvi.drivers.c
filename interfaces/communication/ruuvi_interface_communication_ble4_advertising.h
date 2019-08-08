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

typedef enum 
{
  NONCONNECTABLE_NONSCANNABLE,
  CONNECTABLE_NONSCANNABLE,
  CONNECTABLE_SCANNABLE,
  NONCONNECTABLE_SCANNABLE
}RUUVI_INTERFACE_COMMUNICATION_BLE4_ADVERTISING_TYPE;

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

/**
 * @brief Configure advertising data with a scan response. 
 * The scan response must be separately enabled.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_scan_response_setup
  (const char* const name,
  const bool advertise_nus);

ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_type_set(RUUVI_INTERFACE_COMMUNICATION_BLE4_ADVERTISING_TYPE type);

/** @brief Notify advertising module that advertising has been stopped by external event */
void ruuvi_interface_communication_ble4_advertising_notify_stop(void);

/** @brief start advertising with previously configured settings */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_start();

/** @brief stop advertising */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_stop();

#endif