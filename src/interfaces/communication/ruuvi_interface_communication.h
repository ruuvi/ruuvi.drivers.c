#ifndef RUUVI_INTERFACE_COMMUNICATION_H
#define RUUVI_INTERFACE_COMMUNICATION_H
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"

/**
 * @file ruuvi_interface_communication.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-03-26
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Commmon definitions and functions for all radio operations.
 *
 */

#if RI_COMM_ENABLED
#  define RUUVI_NRF5_SDK15_COMMUNICATION_ENABLED  RUUVI_NRF5_SDK15_ENABLED
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @brief Maximum length for device information strings. */
#define RI_COMM_DIS_STRLEN 48
/** @brief Set ri_comm_message_t->repeat_count to this value to e.g. advertise fixed data
           until explicitly stopped. */
#define RI_COMM_MSG_REPEAT_FOREVER (0U)

/** @brief Application message definition. */
typedef struct ri_comm_message_t
{
    uint8_t data[RI_COMM_MESSAGE_MAX_LENGTH]; //!< Data payload.
    uint8_t data_length;                      //!< Length of data
    uint8_t repeat_count;                     //!< Number of times to repeat the message,
    // RI_COMM_MSG_REPEAT_FOREVER for infinite sends, 1 for send once.
} ri_comm_message_t;

/** @brief Communication event type */
typedef enum
{
    RI_COMM_CONNECTED,    //!< Connection established, OK to send, may receive data.
    RI_COMM_DISCONNECTED, //!< Connection lost, cannot send, may not receive data.
    RI_COMM_SENT,         //!< One queued message was sent with all repetitions.
    RI_COMM_RECEIVED,     //!< New data received, available to read with read function
    RI_COMM_TIMEOUT,      //!< Operation timed out.
    RI_COMM_ABORTED       //!< Operation aborted, e.g. advertising on connection.
} ri_comm_evt_t;          //!< Events @ref ri_comm_channel_t may receive.

typedef struct
{
    char fw_version[RI_COMM_DIS_STRLEN];   //!< Human readable firmware version.
    char model[RI_COMM_DIS_STRLEN];        //!< Human readable board model.
    char hw_version[RI_COMM_DIS_STRLEN];   //!< Human readable hardware version.
    char manufacturer[RI_COMM_DIS_STRLEN]; //!< Human readable manufacturer name.
    char deviceid[RI_COMM_DIS_STRLEN];     //!< Human readable device ID.
    char deviceaddr[RI_COMM_DIS_STRLEN];   //!< Human readable device address, e.g. MAC.
} ri_comm_dis_init_t;                      //!< Basic device information structure.

typedef enum
{
    RI_GATT_TURBO,
    RI_GATT_STANDARD,
    RI_GATT_LOW_POWER
} ri_gatt_params_t;

typedef struct ri_comm_channel_t ri_comm_channel_t;

/* @brief Callback handler for communication events */
typedef void (*ri_comm_cb_t) (void * p_data, size_t data_len);

/**
 *  @brief Asynchronous transfer function. Puts/gets message in driver queue
 *
 *  @param[in, out] msg A message to put/get to/from driver queue
 *  @return RD_MORE_AVAILABLE if data was read from queue and there is more data available.
 *  @return RD_SUCCESS if queue operation was successful.
 *  @return RD_ERROR_NULL if message is NULL.
 *  @return RD_ERROR_DATA_SIZE if message length is larger than queue supports.
 *  @return RD_ERROR_NO_MEM if queue is full and new data cannot be queued.
 *  @return RD_ERROR_NOT_FOUND if queue is empty and no more data can be read.
 */
typedef rd_status_t (*ri_comm_xfer_fp_t) (ri_comm_message_t * const
        msg);

/**
 *  @brief (Un-)Initialization function.
 *  @param[in, out] A control API. Event handler must be set by application.
 *  @return RD_SUCCESS if operation was successful.
 *  @return RD_ERROR_NULL if API is NULL.
 *  @return error driver from stack on other error
 */
typedef rd_status_t (*ri_comm_init_fp_t) (ri_comm_channel_t * const channel);

/** @brief Application event handler for communication events.
 *  @param[in] evt Type of event, @ref ri_comm_evt_t.
 *  @param[in] data Data associated with the event. May be NULL.
 *  @param[in] data_len Length of event data. Must be 0 if data is NULL.
 *                      Must be at maximum @ref RI_COMM_MESSAGE_MAX_LENGTH.
 *  @return RD_SUCCESS if operation was successful.
 *  @return error driver from stack on other error
 */
typedef rd_status_t (*ri_comm_evt_handler_fp_t) (const ri_comm_evt_t
        evt, void * p_data, size_t data_len);

/** @brief control API for communication via outside world */
struct ri_comm_channel_t
{
    ri_comm_xfer_fp_t send;    //!< Asynchronous send function
    ri_comm_xfer_fp_t read;    //!< Asynchronous read function
    ri_comm_init_fp_t init;    //!< Initialize and populate channel api control
    ri_comm_init_fp_t uninit;  //!< Uninitialize and depopulate channel api control
    ri_comm_evt_handler_fp_t
    on_evt;  //!< Callback to application-level event handler, must be set in application.
};

/**
 * Writes maximum 64-bit unique id of the device to the pointer. This ID
 * must remain same across reboots and reflashes of the device
 *
 * param id: Output, value of id.
 * return RD_SUCCESS on success
 * return RD_ERROR_NOT_SUPPORTED if ID cannot be returned on given platform
 *
 */
rd_status_t ri_comm_id_get (uint64_t * const id);

#endif
