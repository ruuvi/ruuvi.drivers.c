/**
 * Ruuvi NFC interface.
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
 * Initializes NFC hardware.
 *
 * Returns RUUVI_DRIVER_SUCCESS on success, RUUVI_DRIVER_ERROR_INVALID_STATE if radio is already initialized
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_init (
    ruuvi_interface_communication_t * const channel);

/*
 * Uninitializes NFC hardware.
 *
 * Returns RUUVI_DRIVER_SUCCESS on success or if radio was not initialized.
 * Returns RUUVI_DRIVER_ERROR_INVALID_STATE if radio hardware was initialized by another radio module.
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_uninit (
    ruuvi_interface_communication_t * const channel);

// Encodes the given data fields into NFC buffer. Clears previous data.
ruuvi_driver_status_t ruuvi_interface_communication_nfc_data_set (void);

/**
 * Send data as ascii-encoded binary.
 *
 * Returns RUUVI_DRIVER_SUCCESS if the data was placed in buffer
 * Returns error code from the stack if data could not be placed to the buffer
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_send (
    ruuvi_interface_communication_message_t * messge);

/**
 * Sets the device firmware version into "FW" text field.
 *
 *  parameter version: Pointer on string representation of the version. ie. "FW: RuuviFW 3.17.0"
 *
 * returns RUUVI_DRIVER_SUCCESS on success
 * returns RUUVI_DRIVER_ERROR_NULL if version is NULL and length != 0
 * returns RUUVI_DRIVER_INVALID_LENGTH if name is over 32 bytes long
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_fw_version_set (
    const uint8_t * const version, const uint8_t length);

/**
 * Sets the device mac address into "ad" text field.
 *
 *  parameter address: Pointer on string representation of the mac address. ie. "MAC: 12:34:56:78:90:AB"
 *
 * returns RUUVI_DRIVER_SUCCESS on success
 * returns RUUVI_DRIVER_ERROR_NULL if address is NULL and length != 0
 * returns RUUVI_DRIVER_INVALID_LENGTH if name is over 32 bytes long
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_address_set (
    const uint8_t * const address, const uint8_t length);

/**
 * Sets the device id into "id" text field. Set NULL/0 to disable.
 *
 *  parameter id: Pointer on string representation of the id. ie. "ID: 12:34:56:78:90:AB:CD:EF"
 *
 * returns RUUVI_DRIVER_SUCCESS on success
 * returns RUUVI_DRIVER_ERROR_NULL if id is NULL and length != 0
 * returns RUUVI_DRIVER_INVALID_LENGTH if name is over 32 bytes long
 */
ruuvi_driver_status_t ruuvi_interface_communication_nfc_id_set (const uint8_t * const id,
        const uint8_t length);

// Not implemented
ruuvi_driver_status_t ruuvi_interface_communication_nfc_receive (
    ruuvi_interface_communication_message_t * messge);

#endif
