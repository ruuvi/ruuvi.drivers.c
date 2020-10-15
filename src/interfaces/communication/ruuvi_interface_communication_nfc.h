/**
 * Ruuvi NFC interface.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_COMMUNICATION_NFC_H
#define RUUVI_INTERFACE_COMMUNICATION_NFC_H
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include <stdint.h>

#if RI_NFC_ENABLED
#  define RUUVI_NRF5_SDK15_NFC_ENABLED  RUUVI_NRF5_SDK15_ENABLED
#endif

#define RI_NFC_ID_FIELD_CODE {'i', 'd'}
#define RI_NFC_ADDR_FIELD_CODE {'a', 'd'}
#define RI_NFC_SW_FIELD_CODE {'s', 'w'}
#define RI_NFC_DATA_FIELD_CODE {'d', 't'}


/*
 * Initializes NFC hardware.
 *
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_INVALID_STATE if NFC is already initialized
 */
rd_status_t ri_nfc_init (ri_comm_channel_t * const channel);

/*
 * Uninitializes NFC hardware.
 *
 * Returns RD_SUCCESS on success.
 */
rd_status_t ri_nfc_uninit (ri_comm_channel_t * const channel);

// Encodes the given data fields into NFC buffer. Clears previous data.
rd_status_t ri_nfc_data_set (void);

/**
 * Send data as ascii-encoded binary.
 *
 * Returns RD_SUCCESS if the data was placed in buffer
 * Returns error code from the stack if data could not be placed to the buffer
 */
rd_status_t ri_nfc_send (ri_comm_message_t * messge);

/**
 * Sets the device firmware version into "FW" text field.
 *
 *  parameter version: Pointer on string representation of the version. ie. "FW: RuuviFW 3.17.0"
 *
 * returns RD_SUCCESS on success
 * returns RD_ERROR_NULL if version is NULL and length != 0
 * returns RD_INVALID_LENGTH if name is over 32 bytes long
 */
rd_status_t ri_nfc_fw_version_set (const uint8_t * const version, const uint8_t length);

/**
 * Sets the device mac address into "ad" text field.
 *
 *  parameter address: Pointer on string representation of the mac address. ie. "MAC: 12:34:56:78:90:AB"
 *
 * returns RD_SUCCESS on success
 * returns RD_ERROR_NULL if address is NULL and length != 0
 * returns RD_INVALID_LENGTH if name is over 32 bytes long
 */
rd_status_t ri_nfc_address_set (const uint8_t * const address, const uint8_t length);

/**
 * Sets the device id into "id" text field. Set NULL/0 to disable.
 *
 *  parameter id: Pointer on string representation of the id. ie. "ID: 12:34:56:78:90:AB:CD:EF"
 *
 * returns RD_SUCCESS on success
 * returns RD_ERROR_NULL if id is NULL and length != 0
 * returns RD_INVALID_LENGTH if name is over 32 bytes long
 */
rd_status_t ri_nfc_id_set (const uint8_t * const id,
                           const uint8_t length);

// Not implemented
rd_status_t ri_nfc_receive (ri_comm_message_t * messge);

#endif
