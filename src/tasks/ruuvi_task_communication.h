/**
 * @file task_communication.h
 * @author Otso Jousimaa
 * @date 2019-12-16
 * @brief Control application via 2-way communication
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 */
/**
 * @addtogroup communication_tasks
 */
/*@{*/

#ifndef  TASK_COMMUNICATION_H
#define  TASK_COMMUNICATION_H

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_log.h"

/** @brief Function to get heartbeat data */
typedef rd_status_t (*heartbeat_data_fp_t) (uint8_t * const msg);

/**
 * @brief Start sending a "hearbeat" signal over given channel to a connected device(s).
 *
 * The heartbeats are continuous data transmissions, such as current sensor values.
 * It's a good practice to reset watchdog timer in heartbeat function, and nowhere else.
 * New heartbeat can be configured over old one without stopping.
 *
 * @param[in] interval_ms interval to send the data, in milliseconds. Set to 0 to stop the heartbeat
 * @param[in] max_len Maximum length of data to send.
 * @param[in] data_src function to generate the heartbeat message
 * @param[in] send function pointer to send the data through. May be NULL if interval is 0.
 *
 * @retval RD_SUCCESS if heartbeat was initialized (or stopped)
 * @retval RD_ERROR_INVALID_STATE if timer cannot be initialized.
 * @retval RD_ERROR_NULL if interval wasn't 0 and send is NULL
 * @retval error code from stack on other error.
 *
 */
rd_status_t rt_com_heartbeat_configure (const uint32_t interval_ms,
                                        const size_t max_len,
                                        const heartbeat_data_fp_t data_src,
                                        const ri_communication_xfer_fp_t send);

/**
 * @brief get MAC address of the device from radio driver and write it to given buffer.
 *
 * The MAC address (or Bluetooth Address) is presented as most significant byte first,
 * if your Bluetooth scanner shows "AA:BB:CC:DD:EE:FF" the mac_buffer will have
 * 0xAA, BB, CC, DD, EE, FF.
 *
 * @param[out] mac_buffer 6-character array
 * @retval RD_SUCCESS if mac buffer was written
 * @retval RD_ERROR_NULL if mac_buffer was NULL
 * @retval RD_ERROR_INVALID_STATE if radio is not initialized
 */
rd_status_t rt_com_get_mac (uint8_t * const mac_buffer);

/**
 * @brief Get Unique ID of the device.
 *
 * The ID will remain constant even if MAC is changed. The ID must remain same across
 * reboots and firmware updates.
 *
 * @param[out] id_buffer 8-character array
 * @retval RD_SUCCESS if mac buffer was written
 * @retval RD_ERROR_NULL if mac_buffer was NULL
 * @retval RD_ERROR_INVALID_STATE if radio is not initialized
 */
rd_status_t rt_com_get_id (uint8_t * const id_buffer);

/*@}*/

#endif