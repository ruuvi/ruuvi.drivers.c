#ifndef RUUVI_TASK_NFC_H
#define RUUVI_TASK_NFC_H
/**
 * @addtogroup peripheral_tasks
 */
/*@{*/
/**
 * @defgroup nfc_tasks NFC tasks
 * @brief NFC tasks for application.
 *
 */
/*@}*/
/**
 * @addtogroup nfc_tasks
 */
/*@{*/
/**
 * @file ruuvi_task_nfc.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-02-21
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * @brief NFC control.
 *
 * Typical usage:
 *
 * @code{.c}
 *  rd_status_t err_code = RD_SUCCESS;
 *  ri_communication_dis_init_t dis = {
 *    .fw_version  = "FW version",
 *    .deviceid    = "00:11:22:33:44:55:66:77",
 *    .deviceaddr  = "FF:EE:DD:CC:BB:AA"
 *    };
 *  err_code = rt_nfc_init(&dis);
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 * @endcode
 */
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_nfc.h"
#include "ruuvi_interface_log.h"

/**
 * @brief Initializes NFC and configures FW, ADDR and ID records according to application_config.h constants.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL if init_data is NULL
 * @retval RD_ERROR_INVALID_STATE if NFC is already initialized.
 * @return error code from stack on error.
 */
rd_status_t rt_nfc_init (ri_comm_dis_init_t * const init_data);

/**
 * @brief Uninitializes NFC.
 *
 * After uninitialization device is no longer readable with NFC reader.
 *
 * @return RD_SUCCESS on success.
 * @return error code from stack on error.
 */
rd_status_t rt_nfc_uninit (void);

/**
 * @brief check that NFC is initialized.
 *
 * @return True if NFC is initialized, false otherwise.
 */
bool rt_nfc_is_init (void);

/**
 * @brief Sets given message to NFC RAM buffer. Clears previous message.
 *
 * NFC data cannot be sent while NFC is connected, setup data before reader enters range.
 *
 * @return RD_SUCCESS on success
 * @return error code from stack on error
 */
rd_status_t rt_nfc_send (ri_comm_message_t * message);

/**
 * @brief Handle Ruuvi communication events from NFC driver
 *
 * @param[in] evt Type of event.
 * @param[in] p_data pointer to data received. NULL if data was not received.
 * @param[in] data_len length of data received. 0 if data was NULL.
 *
 * @return RD_SUCCESS if no error occurred
 * @return error code from stack on error.
 */
rd_status_t rt_nfc_on_nfc (ri_comm_evt_t evt, void * p_data, size_t data_len);

/** @brief Setup connection event handler.
 *
 *  The event handler has signature of @code void(*rt_comm_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is NULL and event_size is 0.
 *  The event handler is called in interrupt context.
 *
 * @param[in] cb Callback which gets called on connection in interrupt context.
 */
void rt_nfc_set_on_connected_isr (const ri_comm_cb_t cb);

/** @brief Setup disconnection event handler.
 *
 *  The event handler has signature of @code void(*rt_comm_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is NULL and event_size is 0.
 *  The event handler is called in interrupt context.
 *
 * @param[in] cb Callback which gets called on disconnection in interrupt context.
 */
void rt_nfc_set_on_disconn_isr (const ri_comm_cb_t cb);

/** @brief Setup data received event handler.
 *
 *  The event handler has signature of @code void(*rt_comm_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is NULL and event_size is 0.
 *  The event handler is called in interrupt context.
 *
 * @param[in] cb Callback which gets called on data received in interrupt context.
 */
void rt_nfc_set_on_received_isr (const ri_comm_cb_t cb);

/** @brief Setup data sent event handler.
 *
 *  The event handler has signature of @code void(*rt_comm_cb_t)(void* p_event_data, uint16_t event_size) @endcode
 *  where event data is NULL and event_size is 0.
 *  The event handler is called in interrupt context.
 *
 * @param[in] cb Callback which gets called on data sent in interrupt context.
 */
void rt_nfc_set_on_sent_isr (const ri_comm_cb_t cb);

/**
 * @brief check if NFC is connected, i.e. a reader is in range.
 * NFC data cannot be changed during connection, setup data to be sent
 * while NFC is not connected.
 *
 * @return true if NFC is connected, false otherwise.
 */
bool rt_nfc_is_connected();

// Let Ceedling test internal functions
#ifdef CEEDLING
rd_status_t rt_nfc_isr (ri_comm_evt_t evt, void * p_data, size_t data_len);
rd_status_t sw_set (const char * const sw);
rd_status_t mac_set (const char * const mac);
rd_status_t id_set (const char * const id);

#endif


/*@}*/
#endif
