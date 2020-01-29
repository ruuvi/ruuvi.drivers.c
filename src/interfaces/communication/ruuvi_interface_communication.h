/**
 * Ruuvi communication interface
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef RUUVI_INTERFACE_COMMUNICATION_H
#define RUUVI_INTERFACE_COMMUNICATION_H
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @brief Application message definition. */
typedef struct ri_communication_message_t
{
    uint8_t data[RI_COMMUNICATION_MESSAGE_MAX_LENGTH]; //!< Data payload.
    uint8_t data_length;                               //!< Length of data
    uint8_t repeat;                                    //!< Number of times to repeat the message, 0 for infinite sends, 1 for send once.
} ri_communication_message_t;

/** @brief Communication event type */
typedef enum
{
    RI_COMMUNICATION_CONNECTED,    //!< Connection established, OK to send, may receive data.
    RI_COMMUNICATION_DISCONNECTED, //!< Connection lost, cannot send, may not receive data.
    RI_COMMUNICATION_SENT,         //!< One queued message was sent with all repetitions.
    RI_COMMUNICATION_RECEIVED,     //!< New data received, available to read with read function
    RI_COMMUNICATION_TIMEOUT       //!< Operation timed out.
} ri_communication_evt_t;

typedef struct ri_communication_t ri_communication_t; //!< forward declaration *and* typedef

/** @brief Asynchronous transfer function. Puts/gets message in driver queue
 *  @param[in, out] A message to put/get to/from driver queue
 *  @return RD_MORE_AVAILABLE if data was read from queue and there is more data available.
 *  @return RD_SUCCESS if queue operation was successful.
 *  @return RD_ERROR_NULL if message is NULL.
 *  @return RD_ERROR_NO_MEM if queue is full and new data cannot be queued.
 *  @return RD_ERROR_NOT_FOUND if queue is empty and no more data can be read.
 */
typedef rd_status_t (*ri_communication_xfer_fp_t) (ri_communication_message_t * const);

/** @brief (Un-)Initialization function.
 *  @param[in, out] A control API. Event handler must be set by application.
 *  @return RD_SUCCESS if operation was successful.
 *  @return RD_ERROR_NULL if API is NULL.
 *  @return error driver from stack on other error
 */
typedef rd_status_t (*ri_communication_init_fp_t) (
    ri_communication_t * const);

/** @brief Application event handler for communication events.
 *  @param[in] evt Type of event, @ref ri_communication_evt_t.
 *  @param[in] data Data associated with the event. May be NULL.
 *  @param[in] data_len Length of event data. Must be 0 if data is NULL. Must be at maximum @ref RI_COMMUNICATION_MESSAGE_MAX_LENGTH.
 *  @return RD_SUCCESS if operation was successful.
 *  @return error driver from stack on other error
 */
typedef rd_status_t (*ri_communication_evt_handler_fp_t) (
    const ri_communication_evt_t evt, void * p_data, size_t data_len);

/** @brief control API for communication via outside world */
struct ri_communication_t
{
    ri_communication_xfer_fp_t send;    //!< Asynchronous send function
    ri_communication_xfer_fp_t read;    //!< Asynchronous read function
    ri_communication_init_fp_t init;    //!< Initialize and populate channel api control
    ri_communication_init_fp_t uninit;  //!< Uninitialize and depopulate channel api control
    ri_communication_evt_handler_fp_t
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
rd_status_t ri_communication_id_get (uint64_t * const id);

#endif