/**
 * Ruuvi NFC interface
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_COMMUNICATION_NFC_H
#define RUUVI_INTERFACE_COMMUNICATION_NFC_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include <stdint.h>

/*
 * Initializes NFC hardware
 *
 * Returns RUUVI_DIRVER_SUCCESS on success, RUUVI_DIRVER_ERROR_INVALID_STATE if radio is already initialized
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_init(ruuvi_interface_communication_t* const channel);

/*
 * Uninitializes NFC hardware
 *
 * Returns RUUVI_DIRVER_SUCCESS on success or if radio was not initialized.
 * Returns RUUVI_DRIVER_ERROR_INVALID_STATE if radio hardware was initialized by another radio module.
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_uninit(ruuvi_interface_communication_t* const channel);

// Encodes the given data fields into NFC buffer. Clears previous data.
ruuvi_driver_status_t ruuvi_interface_communication_nfc_data_set(const uint8_t* data, const uint8_t data_length);

/**
 * Send data as ascii-encoded binary.
 *
 * Returns RUUVI_DRIVER_SUCCESS if the data was placed in buffer
 * Returns error code from the stack if data could not be placed to the buffer
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_send(ruuvi_interface_communication_message_t* messge);

/**
 * Sets the device firmware version into "FW" text field.
 *
 * returns RUUVI_DRIVER_SUCCESS on success
 * returns RUUVI_DRIVER_INVALID_LENGTH if name is over 20 bytes long
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_fw_version_set(const uint8_t* const version, const uint8_t length);

/**
 * Sets the device mac address into "ad" text field.
 *
 * returns RUUVI_DRIVER_SUCCESS on success
 * returns RUUVI_DRIVER_INVALID_LENGTH if name is over 20 bytes long
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_address_set(const uint8_t* const address, const uint8_t length);

/**
 * Sets the device id into "id" text field.
 *
 * returns RUUVI_DRIVER_SUCCESS on success
 * returns RUUVI_DRIVER_INVALID_LENGTH if name is over 20 bytes long
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_id_set(const uint8_t* const id, const uint8_t length);

// Not implemented
ruuvi_driver_status_t ruuvi_interface_communication_nfc_receive(ruuvi_interface_communication_message_t* messge);

#endif