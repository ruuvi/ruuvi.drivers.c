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
#include "ruuvi_driver_enabled_modules.h"

#if RI_GATT_ENABLED
#   define RUUVI_NRF5_SDK15_GATT_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/**
 * @brief Initializes GATT stack.
 * Uses default values from sdk_config.h, these can be overridden in nrf5_sdk15_application_config.h
 *
 * @return RD_SUCCESS on success
 * @return RD_ERROR_INVALID_STATE if radio module is not initialized with handle RI_COMMUNICATION_RADIO_GATT
 */
rd_status_t ri_gatt_init (void);

/**
 * @brief Uninitializes GATT stack.
 *
 *
 *
 * @return RD_SUCCESS on success
 * @return RD_ERROR_INVALID_STATE if radio module is not initialized with handle RI_COMMUNICATION_RADIO_GATT
 */
rd_status_t ri_gatt_uninit (void);

/**
 * @brief Initialize Nordic UART Service as a communication channel.
 * ri_communication_radio_init(RI_COMMUNICATION_RADIO_GATT) must be called before initializing service
 *
 * @param[in] channel: Pointer to communication interface which will be populated. Pointer will be copied, the structure
 *                     must be retained. Adding any event handler to structure after initialization will take effect immediately
 * @return RD_SUCCESS on success.
 * @return error code from stack in case there is other error.
 */
rd_status_t ri_gatt_nus_init (ri_comm_channel_t * const channel);

/**
 * @brief Uninitialize Nordic UART Service as a communication channel.
 *
 * @param[in] channel: Pointer to communication interface which will be depopulated.
 * @return RD_SUCCESS on success
 * @return error code from stack in case there is other error.
 */
rd_status_t ri_gatt_nus_uninit (ri_comm_channel_t * const _channel);

/**
 * @brief Initialize BLE4 Device firmware update service.
 *
 * @return RD_SUCCESS on success
 * @return error code from stack in case there is  error.
 */
rd_status_t ri_gatt_dfu_init (void);

/**
 * @brief Initialize BLE4 Device Information service
 *
 * @param[in] dis pointer to data which should be presented over DIS. Memory will be deep-copied
 * @return RD_SUCCESS on success
 * @return error code from stack in case there is  error.
 */
rd_status_t ri_gatt_dis_init (const ri_comm_dis_init_t * const dis);

#endif