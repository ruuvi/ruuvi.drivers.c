#ifndef RUUVI_INTERFACE_COMMUNICATION_BLE4_ADVERTISING_H
#define RUUVI_INTERFACE_COMMUNICATION_BLE4_ADVERTISING_H

/**
 * @file ruuvi_interface_communication_ble4_advertising.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-09-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Commmon definitions and functions for all BLE4 advertising.
 *
 */

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"
#include <stdint.h>

/** @brief Alloved advertisement types */
typedef enum 
{
  NONCONNECTABLE_NONSCANNABLE, //!< Nonconnectable, nonscannable
  CONNECTABLE_NONSCANNABLE,    //!< Connectable, nonscannable
  CONNECTABLE_SCANNABLE,       //!< Connectable, scannable
  NONCONNECTABLE_SCANNABLE     //!< Nonconnectable, scannable
}RUUVI_INTERFACE_COMMUNICATION_BLE4_ADVERTISING_TYPE;

/** @brief Advertisement report from scanner 
 *
 */
 typedef struct {
   uint8_t addr[6];  //<! MAC address, MSB first
   int8_t rssi;      //!< RSSI of advertisement
   uint8_t data[31]; //!< Full payload of the advertisement
   size_t data_len;  //!< Length of received data
 }ruuvi_interface_communication_ble4_scan_t; 

/*
 * @brief Initialize radio hardware, advertising module and scanning module.
 *
 * @return RUUVI_DIRVER_SUCCESS on success, 
 * @return RUUVI_DIRVER_ERROR_INVALID_STATE if radio is already initialized
 *
 * @note This function calls ruuvi_interface_communication_radio_init internally, calling separate radio initialization is not required.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_init(
  ruuvi_interface_communication_t* const channel);

/*
 * @brief Uninitializes radio hardware, advertising module and scanning module.
 *
 * @param[out] channel Communication api to send and receive data via advertisements.
 *
 * @return RUUVI_DIRVER_SUCCESS on success or if radio was not initialized.
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if radio hardware was initialized by another radio module.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_uninit(
  ruuvi_interface_communication_t* const channel);

/*
 * @brief Setter for broadcast advertisement interval.
 *
 * @param[in] ms Milliseconds, random delay of 0 - 10 ms will be added to the interval on every TX to avoid collisions. min 100 ms, max 10 000 ms.
 * @return RUUVI_DRIVER_SUCCESS on success, 
 * @return RUUVI_DRIVER_ERROR_INVALID_PARAM if the parameter is outside allowed range
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_tx_interval_set(
  const uint32_t ms);

/**
 * @brief Getter for broadcast advertisement interval
 *
 * @param[out] ms Milliseconds between transmission, without the random delay.
 * @return RUUVI_DRIVER_SUCCESS on success. 
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if advertisement module is not initialized.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_tx_interval_get(
  uint32_t* ms);

/**
 * @brief Set manufacturer ID of manufacturer specific advertisement
 *
 * @param[in] id ID of manufacturer, MSB first. E.g. 0x0499 for Ruuvi Innovations.
 * @return RUUVI_DRIVER_SUCCESS
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_manufacturer_id_set(
  const uint16_t id);

/** 
 * @brief Set manufacturer specific data to advertise.
 * Clears previous data, but scan response is retained. The data is placed to primary advertisement.
 * This function can be called while advertising, but it does not quarantee that advertisement will be sent.
 * Calling this function twice in a row will result in first advertisement being immediately discarded.
 * 
 * @param[in] data to advertise
 * @param[in] data_length length of data ot advertise, maximum 24 bytes.
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NULL if data is NULL
 * @return RUUVI_DRIVER_ERROR_INVALID_LENGTH if data_length is over 24 bytes
 * @return error code from stack on other error. 
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_data_set(
  const uint8_t* data, const uint8_t data_length);

/**
 * @brief Send data as manufacturer specific data payload.
 * If no new data is placed to the buffer, last message sent will be repeated until advertising is stopped.
 *
 * @return RUUVI_DRIVER_SUCCESS if the data was placed in buffer
 * @return RUUVI_DRIVER_ERROR_NULL if message is NULL
 * @return RUUVI_DRIVER_ERROR_INVALID_LENGTH if data length is over 24 bytes
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_send(
  ruuvi_interface_communication_message_t* message);

// Set / get radio tx power
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_tx_power_set(
  int8_t* dbm);
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_tx_power_get(
  int8_t* dbm);

/** @brief Communication API read function */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_receive(
  ruuvi_interface_communication_message_t* message);

/** @brief setup scan window interval and window size.
 *  
 *  The scan window interval must be larger or equivalent to window size. 
 *  Example: Interval 1000 ms, window size 100 ms. The scanning will scan 100 ms at channel 37, wait 900 ms, scan 100 ms at channel 38,
 *  wait 900 ms, scan 100 ms at channel 39 and terminate. 
 * 
 *  @param[in] window_interval_ms interval of the window.
 *  @param[in] window_size_ms     window size within interval.
 *  @return RUUVI_DRIVER_SUCCESS  on success
 *  @return RUUVI_DRIVER_ERROR_INVALID_STATE if scan is ongoing
 *  @return RUUVI_DRIVER_ERROR_INVALID_PARAM if window is larger than interval or values are otherwise invalid.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_rx_interval_set(const uint32_t window_interval_ms, const uint32_t window_size_ms);

/** @brief get scan window interval and window size.
 *  
 * 
 *  @param[out] window_interval_ms interval of the window.
 *  @param[out] window_size_ms     window size within interval.
 *  @return RUUVI_DRIVER_SUCCESS  on success
 *  @return RUUVI_DRIVER_ERROR_NULL if either pointer is NULL
 *  @return RUUVI_DRIVER_ERROR_INVALID_PARAM if window is larger than interval or values are otherwise invalid.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_rx_interval_get(uint32_t* window_interval_ms, uint32_t* window_size_ms);

ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_scan_start(void);
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_scan_stop(void);

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

/** @brief send one raw packet */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_send_raw(uint8_t* data, size_t data_length);

ruuvi_driver_status_t ruuvi_interface_communication_ble4_advertising_ongoing(void);

#endif