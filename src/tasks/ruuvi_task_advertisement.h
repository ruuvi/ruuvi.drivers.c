#ifndef RUUVI_TASK_ADVERTISEMENT_H
#define RUUVI_TASK_ADVERTISEMENT_H

/**
 * @defgroup communication_tasks Sending and receiving data.
 */
/** @{ */
/**
 * @defgroup advertisement_tasks Advertisement tasks
 * @brief Bluetooth Low Energy advertising
 *
 */
/** @} */
/**
 * @addtogroup advertisement_tasks
 */
/** @{ */
/**
 * @file ruuvi_task_advertisement.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-06-01
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Advertise data and GATT connection if available.
 *
 *
 * Typical usage:
 *
 * @code{.c}
 *  rd_status_t err_code = RD_SUCCESS;
 *  err_code |= ri_radio_init(RI_RADIO_BLE_2MBPS);
 *  ri_radio_channels_t channels;
 *  channels.channel_37 = 1;
 *  channels.channel_38 = 1;
 *  channels.channel_39 = 1;
 *  rt_adv_init_t adv_settings;
 *  adv_settings.adv_interval_ms = APP_ADV_INTERVAL_MS;
 *  adv_settings.adv_pwr_dbm = RB_TX_POWER_MAX;
 *  adv_settings.channels = channels;
 *  adv_settings.manufacturer_id = RB_BLE_MANUFACTURER_ID;
 *  err_code |= rt_adv_init(&adv_settings);
 *  err_code |= ri_adv_type_set(NONCONNECTABLE_NONSCANNABLE);
 *  err_code |= rt_adv_send_data (&my_message);
 * @endcode
 */

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication_ble_advertising.h"

/** @brief Longer name gets truncated when advertised with UUID. */
#define SCAN_RSP_NAME_MAX_LEN (11U)
/** @brief Window of one scan per channel. Scan takes this * num_channels time */
#define RT_ADV_SCAN_WINDOW_MS (7000U)
//!< @brief Interval of one scan within a window. Can be at most equal to scan window. */
#define RT_ADV_SCAN_INTERVAL_MS (7000U)

/** @brief Initial configuration for advertisement. PHY will be transferred to GATT.  */
typedef struct
{
    ri_radio_channels_t channels; //!< Radio channels, typically 37, 38, and/or 39.
    uint16_t adv_interval_ms;     //!< ms / advertisement, not counting random delay
    int8_t   adv_pwr_dbm;         //!< Power to antenna, dBm.
    uint16_t manufacturer_id;     //!< BLE SIG id of board manufacturer
} rt_adv_init_t;

/**
 * @brief Initializes data advertising.
 *
 * The function setups advertisement interval, advertisement power, advertisement type,
 * manufacturer ID for manufacturer specific data according to constants in
 * application_config.h and ruuvi_boards.h.
 *
 * It also configures a callback to be executed after advertisement for internal use.
 * After calling this function advertisement data can be queued into advertisement buffer.
 *
 * @param[in,out] adv_init_settings Input: Desired setup. Output: Configured setup.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if advertising is already initialized.
 * @retval RD_ERROR_INVALID_PARAM if configuration constant is invalid. Not initialized.
 */
rd_status_t rt_adv_init (rt_adv_init_t * const adv_init_settings);

/**
 * @brief Uninitializes data advertising.
 *
 * Can be called even if advertising was not initialized.
 * Clears previous advertisement data if there was any.
 *
 * @retval RD_SUCCESS on success
 * @return error code from stack on error
 */
rd_status_t rt_adv_uninit (void);

/**
 * @brief Stops advertising. This is relevant only if the message was on repeat.
 *
 * @retval RD_SUCCESS on success
 * @return error code from stack on error
 */
rd_status_t rt_adv_stop (void);

/** @brief Send given message as a BLE advertisement.
 *
 *  This function configures the primary advertisement packet with the flags and manufacturer specific data.
 *  Payload of the msg will be sent as the manufacturer specific data payload.
 *  Manufacturer ID is defined by RUUVI_BOARD_BLE_MANUFACTURER_ID in ruuvi_boards.h.
 *
 *  If the device is connectable, call @code rd_adv_connectability_set @endcode to setup the
 *  scan response and flags to advertise connectability.
 *
 *  Call @ref rt_adv_stop to stop advertisements on repeat.
 *
 *  @param[in] msg message to be sent as manufacturer specific data payload.
 *  @retval    RD_ERROR_NULL if msg is NULL.
 *  @retval    RD_ERROR_INVALID_STATE if advertising isn't initialized or started.
 *  @retval    RD_ERROR_DATA_SIZE if payload size is larger than 24 bytes.
 *  @return    error code from stack on other error.
 */
rd_status_t rt_adv_send_data (ri_comm_message_t * const msg);

/** @brief Start advertising BLE GATT connection
 *
 *  This function configures the primary advertisement to be SCANNABLE_CONNECTABLE and
 *  sets up a scan response which has given device name (max 10 characters + NULL)
 *  and UUID of Nordic UART Service.
 *
 *  Be sure to configure the GATT before calling this function, as behaviour is undefined
 *  if someone tries to connect to tag while GATT is not configured.
 *
 *  @param[in] enable true to enable connectability, false to disable.
 *  @param[in] device_name NULL-terminated string representing device name, max 10 Chars + NULL.
 *  @retval    RD_SUCCESS if operation was finished as expected.
 *  @retval    RD_ERROR_NULL if name is NULL and trying to enable the scan response
 *  @retval    RD_ERROR_INVALID_STATE if advertising isn't initialized or started.
 *  @retval    RD_ERROR_INVALID_LENGTH if name size exceeds 10 bytes + NULL
 *  @return    error code from stack on other error.
 */
rd_status_t rt_adv_connectability_set (const bool enable,
                                       const char * const device_name);

/** @brief check if advertisement is initialized
 *  @return true if advertisement is initialized, false otherwise.
 */
bool rt_adv_is_init (void);

/** @brief Start scanning BLE advertisements.
 *
 * This is non-blocking, you'll need to handle incoming events.
 *
 * PHY to be scanned is determined by radio initialization.
 * If you have selected 2 MBit / s PHY, primary advertisements
 * are scanned at 1 Mbit / s and secondary extended advertisement
 * can be scanned at at 2 MBit / s.
 *
 * Scan is done at 7000 ms window and interval, this consumes
 * a lot of power and may collapse a coin cell battery.
 *
 * Events are:
 *   - on_evt(RI_COMM_RECEIVED, scan, sizeof(ri_adv_scan_t));
 *   - on_evt(RI_COMM_TIMEOUT, NULL, 0);
 *
 *  @param[in] on_evt Event handler for scan results.
 *  @retval    RD_SUCCESS Scanning was started.
 *  @retval    RD_ERROR_INVALID_STATE Advertising isn't initialized.
 *  @return    error code from stack on other error.
 *
 * @note Scanning is stopped on timeout, you can restart the scan on event handler.
 * @warning Event handler is called in interrupt context.
 */
rd_status_t rt_adv_scan_start (const ri_comm_evt_handler_fp_t on_evt);

/**
 * @brief Abort scanning.
 *
 * After calling this function scanning is immediately stopped.
 *
 */
rd_status_t rt_adv_scan_stop (void);


/** @} */

#endif
