#ifndef  RUUVI_TASK_COMMUNICATION_H
#define  RUUVI_TASK_COMMUNICATION_H

/**
 * @file ruuvi_task_communication.h
 * @author Otso Jousimaa
 * @date 2020-04-29
 * @brief Helper functions for communication.
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 */
/**
 * @addtogroup communication_tasks
 */
/** @{ */


#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_log.h"

#define RT_COMM_MAC_STRLEN (18U)
#define RT_COMM_ID_STRLEN  (24U)

/**
 * @brief Get MAC address of the device from radio driver and write it to given string.
 *
 * The MAC address (or Bluetooth Address) is presented as most significant byte first,
 * if your Bluetooth scanner shows "AA:BB:CC:DD:EE:FF" the mac_str will have
 * "AA:BB:CC:DD:EE:FF".
 *
 * @param[out] mac_str 18-character array.
 * @param[in]  mac_len Length of mac_str, must be at least 18.
 * @retval RD_SUCCESS if mac_str was written.
 * @retval RD_ERROR_NULL if mac_str was NULL.
 * @retval RD_ERROR_INVALID_STATE if radio is not initialized.
 */
rd_status_t rt_com_get_mac_str (char * const mac_str, const size_t mac_len);

/**
 * @brief Get Unique ID of the device and write it to given string.
 *
 * The ID will remain constant even if MAC is changed. The ID must remain same across
 * reboots and firmware updates.
 *
 * @param[out] id_str 24-character array
 * @param[in]  id_len Length of id_str, must be at least 24.
 * @retval RD_SUCCESS if id buffer was written
 * @retval RD_ERROR_NULL if id_buffer was NULL
 */
rd_status_t rt_com_get_id_str (char * const id_str, const size_t id_len);

/** @} */

#endif