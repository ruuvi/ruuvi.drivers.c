/**
 * Ruuvi BLE GATT profile
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_COMMUNICATION_BLE4_GATT_H
#define RUUVI_INTERFACE_COMMUNICATION_BLE4_GATT_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"

#define HCI_ERROR_CODE_CONN_TERM_BY_LOCAL_HOST 0x16

typedef struct
{
  char fw_version[32];
  char model[32];
  char hw_version[32];
  char manufacturer[32];
  char deviceid[32];
} ruuvi_interface_communication_ble4_gatt_dis_init_t;

/**
 * @brief Initializes GATT stack.
 * Uses default values from sdk_config.h, these can be overridden in nrf5_sdk15_application_config.h
 *
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if radio module is not initialized with handle RUUVI_INTERFACE_COMMUNICATION_RADIO_GATT
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_gatt_init(void);

/**
 * @brief Initialize Nordic UART Service as a communication channel.
 * ruuvi_interface_communication_radio_init(RUUVI_INTERFACE_COMMUNICATION_RADIO_GATT) must be called before initializing service
 *
 * @param[in] channel: Pointer to communication interface which will be populated. Pointer will be copied, the structure
 *                     must be retained. Adding any event handler to structure after initialization will take effect immediately
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if radio module is not initialized with handle RUUVI_INTERFACE_COMMUNICATION_RADIO_GATT
 *                                          or if ruuvi_interface_communication_ble4_gatt_init has not been called.
 * @return error code from stack in case there is other error.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_gatt_nus_init(
  ruuvi_interface_communication_t* const channel);

/**
 * @brief Initialize BLE4 Device firmware update service.
 *
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return error code from stack in case there is  error.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_gatt_dfu_init(void);

/**
 * @brief Initialize BLE4 Device Information service
 *
 * @param[in] dis pointer to data which should be presented over DIS. Memory will be deep-copied
 * @return RUUVI_DRIVER_SUCCESS on success
 * @return error code from stack in case there is  error.
 */
ruuvi_driver_status_t ruuvi_interface_communication_ble4_gatt_dis_init(
  const ruuvi_interface_communication_ble4_gatt_dis_init_t* const dis);

#endif