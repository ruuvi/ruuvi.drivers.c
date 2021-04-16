/**
 * @addtogroup communication_tasks
 */
/*@{*/
/**
 * @file ruuvi_task_gatt.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-01-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 *
 *
 */
#ifndef  RUUVI_TASK_GATT_H
#define  RUUVI_TASK_GATT_H

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_ble_gatt.h"

#ifdef CEEDLING
// Assist function for unit tests.
void rt_gatt_mock_state_reset();

// Expose callback to Ceedling
rd_status_t rt_gatt_on_nus_isr (ri_comm_evt_t evt,
                                void * p_data, size_t data_len);

#endif

/**
 * @brief Send given message via NUS
 *
 * This function queues a message to be sent and returns immediately.
 * There is no guarantee on when the data is actually sent, and
 * there is no acknowledgement or callback after the data has been sent.
 *
 * @retval RD_SUCCESS if data was placed in send buffer
 * @retval RD_ERROR_INVALID_STATE if NUS is not connected
 * @retval RD_ERROR_NO_MEM if tx buffer is full
 * @retval error code from stack on other error
 *
 */
rd_status_t rt_gatt_send_asynchronous (ri_comm_message_t * const msg);

/**
 * @brief Initialize Device Firmware Update service
 *
 * GATT must be initialized before calling this function, and once initialized the DFU
 * service cannot be uninitialized.
 *
 * Call will return successfully even if the device doesn't have useable bootloader, however
 * program will reboot if user tries to enter bootloader in that case.
 *
 * To use the DFU service advertisement module must send connectable (and preferably scannable) advertisements.
 *
 * @retval RD_SUCCESS GATT was initialized successfully
 * @retval RD_ERROR_INVALID_STATE DFU was already initialized or GATT is not initialized
 */
rd_status_t rt_gatt_dfu_init (void);

/**
 * @brief Initialize Device Information Update service
 *
 * GATT must be initialized before calling this function, and once initialized the DIS
 * service cannot be uninitialized.
 *
 * DIS service lets user read basic information, such as firmware version and hardware model over GATT in a standard format.
 *
 * To use the DIS service advertisement module must send connectable (and preferably scannable) advertisements.
 *
 * @param[in] dis structure containing data to be copied into DIS, can be freed after call finishes.
 *
 * @retval RD_SUCCESS GATT was initialized successfully
 * @retval RD_ERROR_NULL if given NULL as the information.
 * @retval RD_ERROR_INVALID_STATE DIS was already initialized or GATT is not initialized
 */
rd_status_t rt_gatt_dis_init (const ri_comm_dis_init_t * const dis);

/**
 * @brief Initialize Nordic UART Service
 *
 * GATT must be initialized before calling this function, and once initialized the NUS
 * service cannot be uninitialized.
 *
 * NUS service lets user do bidirectional communication with the application.
 *
 * To use the NUS service advertisement module must send connectable (and preferably scannable) advertisements.
 *
 *
 * @retval RD_SUCCESS GATT was initialized successfully
 * @retval RD_ERROR_NULL if given NULL as the information.
 * @retval RD_ERROR_INVALID_STATE DIS was already initialized or GATT is not initialized
 *
 * @note To actually use the data in application, user must setup at least data received callback with @ref rt_gatt_set_on_received_isr
 */
rd_status_t rt_gatt_nus_init();

/**
 * @brief Initialize GATT. Must be called as a first function in rt_gatt.
 *
 * After calling this function underlying software stack is ready to setup GATT services.
 *
 * @param[in] name Full name of device to be advertised in scan responses. Maximum 11 chars + trailing NULL. Must not be NULL, 0-length string is valid.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL if name is NULL (use 0-length string instead)
 * @retval RD_ERROR_INVALID_LENGTH if name is longer than @ref SCAN_RSP_NAME_MAX_LEN
 * @retval RD_ERROR_INVALID_STATE if GATT is already initialized or advertisements are not initialized.
 *
 */
rd_status_t rt_gatt_init (const char * const name);

/**
 * @brief Uninitialize GATT.
 *
 * After calling this function callbacks, characteristics and services are cleared.
 *
 * @note Nordic SDK requires radio uninitialization to reset GATT service states.
 *       If any other task is using radio, this function will return error.
 *       This function will re-initialize radio after GATT is uninitialized with original
 *       modulation.
 *
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if GATT cannot be uninitialized.
 *
 */
rd_status_t rt_gatt_uninit (void);

/**
 * @brief Start advertising GATT connection to devices.
 *
 * Calling this function is not enough to let users to connect, you must also update advertised data
 * to add the scan response to data being advertised. This makes sure that advertised data stays valid.
 * This function has no effect if called while already enabled.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if GATT is not initialized.
 */
rd_status_t rt_gatt_adv_enable();

/**
 * @brief Stop advertising GATT connection to devices.
 *
 * Calling this function is not enough to stop advertising connection, you must also update advertised data
 * to remove the scan response from data being advertised. This makes sure that advertised data stays valid.
 * This function has not effect if called while already disabled
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if GATT is not initialized.
 */
rd_status_t rt_gatt_adv_disable();

/**
 * @brief check if GATT task is initialized
 *
 * @return true if GATT is initialized, false otherwise.
 */
bool rt_gatt_is_init();

/**
 * @brief check if NUS is connected, i.e. central has registered to TX notifications.
 *
 * @return true if NUS is connected is initialized, false otherwise.
 */
bool rt_gatt_nus_is_connected();

/** @brief Check if Nordic UART Service is enabled.
 *
 *  The event handler has signature of
 *  @code void(*rt_gatt_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is NULL and event_size is 0.
 *  The event handler is called in interrupt context.
 *
 * @return true if GATT is initialized and ready to accept connection, false otherwise.
 */
bool rt_gatt_is_nus_enabled();

/** @brief Setup connection event handler.
 *
 *  The event handler has signature of @code void(*rt_gatt_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is NULL and event_size is 0.
 *  The event handler is called in interrupt context.
 *
 * @param[in] cb Callback which gets called on connection in interrupt context.
 */
void rt_gatt_set_on_connected_isr (const ri_comm_cb_t cb);

/** @brief Setup disconnection event handler.
 *
 *  The event handler has signature of @code void(*rt_gatt_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is NULL and event_size is 0.
 *  The event handler is called in interrupt context.
 *
 * @param[in] cb Callback which gets called on disconnection in interrupt context.
 */
void rt_gatt_set_on_disconn_isr (const ri_comm_cb_t cb);

/** @brief Setup data received event handler.
 *
 *  The event handler has signature of @code void(*rt_gatt_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is pointer to raw bytes and event_size is length of received data.
 *  The event handler is called in interrupt context.
 *
 * @param[in] cb Callback which gets called on data received in interrupt context.
 */
void rt_gatt_set_on_received_isr (const ri_comm_cb_t cb);

/** @brief Setup data sent event handler.
 *
 *  The event handler has signature of @code void(*rt_gatt_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is NULL and event_size is 0.
 *  The event handler is called in interrupt context.
 *
 * @param[in] cb Callback which gets called on data sent in interrupt context.
 */
void rt_gatt_set_on_sent_isr (const ri_comm_cb_t cb);

#endif
/*@}*/