#ifndef RUUVI_INTERFACE_COMMUNICATION_BLE_ADVERTISING_H
#define RUUVI_INTERFACE_COMMUNICATION_BLE_ADVERTISING_H

/**
 * @file ruuvi_interface_communication_ble_advertising.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-03-26
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Common definitions and functions for all BLE advertising.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"
#include <stdint.h>

#if RI_ADV_ENABLED
#   define RUUVI_NRF5_SDK15_ADV_ENABLED RUUVI_NRF5_SDK15_ENABLED
#   define RUUVI_NRF5_SDK15_ADV_EXTENDED_ENABLED RI_ADV_EXTENDED_ENABLED
#endif

#define RUUVI_COMM_BLE_ADV_MAX_LENGTH  (RI_COMM_BLE_PAYLOAD_MAX_LENGTH)
#define RUUVI_COMM_BLE_ADV_SCAN_LENGTH (RUUVI_COMM_BLE_ADV_MAX_LENGTH)
#define RUUVI_COMM_BLE_ADV_SCAN_BUFFER (RUUVI_COMM_BLE_ADV_MAX_LENGTH)

/* @brief number of bytes in a MAC address */
#define BLE_MAC_ADDRESS_LENGTH 6

/** @brief Allowed advertisement types */
typedef enum
{
    NONCONNECTABLE_NONSCANNABLE, //!< Nonconnectable, nonscannable
    CONNECTABLE_NONSCANNABLE,    //!< Connectable, nonscannable
    CONNECTABLE_SCANNABLE,       //!< Connectable, scannable
    NONCONNECTABLE_SCANNABLE     //!< Nonconnectable, scannable
} ri_adv_type_t;                 //!< Allowed advertisement types. The implementation uses extended type if required.

typedef struct
{
    uint8_t addr[BLE_MAC_ADDRESS_LENGTH];  //<! MAC address, MSB first
    int8_t rssi;      //!< RSSI of advertisement
    uint8_t data[RUUVI_COMM_BLE_ADV_SCAN_LENGTH]; //!< Full payload of the advertisement
    size_t data_len;  //!< Length of received data
} ri_adv_scan_t;      //!< Advertisement report from scanner

/**
 * @brief Initialize Advertising module and scanning module.
 *
 * Initially channels 37, 38 and 39 are enabled. Modulation is decided by
 * radio initialization. If @ref RI_RADIO_BLE_125KBPS or @ref  RI_RADIO_BLE_1MBPS is set,
 * primary phy is the configured one and no secondary PHY is used. If modulation is
 * @ref  RI_RADIO_BLE_2MBPS primary PHY is @ref  RI_RADIO_BLE_1MBPS and secondary PHY is
 * @ref RI_RADIO_BLE_2MBPS.
 *
 * @param[out] channel Interface used for communicating through advertisements.
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_NULL Channel is NULL.
 * @retval RD_ERROR_INVALID_STATE if radio is not already initialized.
 *
 * @note Modulation used on the advertisement depends on how radio was initialized.
 */
rd_status_t ri_adv_init (ri_comm_channel_t * const channel);

/*
 * @brief Uninitializes advertising module and scanning module.
 *
 * @param[out] channel comm api to send and receive data via advertisements.
 * @retval RD_SUCCESS on success or if radio was not initialized.
 * @retval RD_ERROR_INVALID_STATE if radio hardware was initialized by another radio module.
 */
rd_status_t ri_adv_uninit (ri_comm_channel_t * const channel);

/*
 * @brief Setter for broadcast advertisement interval.
 *
 * Will take effect on next send. Has no effect on messages already queued.
 *
 * @param[in] ms Milliseconds, random delay of 0 - 10 ms will be added to the interval on
 *               every TX to avoid collisions. min 100 ms, max 10 000 ms.
 * @retval RD_SUCCESS on success,
 * @retval RD_ERROR_INVALID_PARAM if the parameter is outside allowed range.
 */
rd_status_t ri_adv_tx_interval_set (const uint32_t ms);

/**
 * @brief Getter for broadcast advertisement interval
 *
 * @param[out] ms Milliseconds between transmission, without the random delay.
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL If ms is NULL.
 * @retval RD_ERROR_INVALID_STATE if advertisement module is not initialized.
 */
rd_status_t ri_adv_tx_interval_get (uint32_t * ms);

/**
 * @brief Set manufacturer ID of manufacturer specific advertisement.
 *
 * Will take effect on next send. Has no effect on already queued messages.
 *
 * @param[in] id ID of manufacturer, MSB first. E.g. 0x0499 for Ruuvi Innovations.
 * @retval RD_SUCCESS
 */
rd_status_t ri_adv_manufacturer_id_set (const uint16_t id);

/**
 * @brief Set channels to use on radio. This has effect only on BLE advertising.
 *
 * The function call will not modify already queued advertisements, it will take effect
 * on advertisements queued after this call has finished.
 *
 * @param[in] channels Channels to advertise on.
 * @retval RD_SUCCESS
 */
rd_status_t ri_adv_channels_set (const ri_radio_channels_t channels);


/**
 * @brief Set radio TX power.
 *
 * Takes effect on next call to send, messages already in send queue are not
 * affected.
 *
 * @param[in,out] dbm Radio power. Supported values are board-dependent.
 *                    Value is interpreted as "at least", power is set to smallest
 *                    value which satisfies the desired level. Actual value is set to
 *                    pointed parameter as an output.
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_NULL if dbm is NULL.
 * @retval RD_ERROR_INVALID_STATE  if radio is not initialized.
 * @retval RD_ERROR_INVALID_PARAM if given power level is not supported.
 */
rd_status_t ri_adv_tx_power_set (int8_t * dbm);

/**
 * @brief Get radio TX power.
 *
 * @param[out] dbm Radio power.
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_NULL if dbm is NULL.
 * @retval RD_ERROR_INVALID_STATE  if radio is not initialized.
 */
rd_status_t ri_adv_tx_power_get (int8_t * dbm);

/**
 * @brief Configure advertising data with a scan response.
 *
 * The scan response must be separately enabled by setting advertisement type as
 * scannable. This setting has effect starting from the next time send is called.
 *
 * @param[in] name Name to advertise, NULL-terminated string.
 *                 Max 27 characters if not advertising NUS, 10 characters if NUS is
 *                 advertised. NULL won't be included in advertisement.
 * @param[in] advertise_nus True to include Nordic UART Service UUID in scan response.
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_NULL if name is NULL
 * @retval RD_ERROR_DATA_SIZE if name will be cut. However the abbreviated name will
 *              be set.
 */
rd_status_t ri_adv_scan_response_setup (const char * const name,
                                        const bool advertise_nus);

/**
 * @brief Configure the type of advertisement.
 *
 * Advertisement can be connectable, scannable, both or neither: @ref ri_adv_type_t.
 * It is possible to setup scannable advertisement before setting up scan response,
 * in this case scan response will be 0-length and empty until scan resonse if configured.
 * If the advertisement has to be extended type, the implementation will automatically select
 * extended advertisement.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if advertisements are not initialized.
 */
rd_status_t ri_adv_type_set (ri_adv_type_t type);

/**
 * @brief Stop ongoing advertisements.
 *
 * If advertisement was configured on repeat, calling this function stops advertising
 * before the repeat counter has been reached. Any queued advertisements are cancelled.
 * This function can be safely called even if no advertising is ongoing.
 *
 * @retval RD_SUCCESS if no more advertisements are ongoing.
 */
rd_status_t ri_adv_stop (void);

/** @brief setup scan window interval and window size.
 *
 *  The scan window interval must be larger or equivalent to window size.
 *  Example: Interval 1000 ms, window size 100 ms.
 *  The scanning will scan 100 ms at channel 37, wait 900 ms, scan 100 ms at channel 38,
 *  wait 900 ms, scan 100 ms at channel 39. After scan has finished RI_COMM_TIMEOUT event is triggered
 *  if initialized channel has event handler.
 *
 *  Scan is started immediately after calling this function and ended once timeout occurs or @ref ri_adv_scan_stop is called.
 *
 *  Raw scan results are passed to given event handler as on_received events with @ref ri_adv_scan_t
 *  as a parameter.
 *
 *  If the message contains Ruuvi-specific advertisement data, the payload is inserted into a
 *  ri_comm_message_t with repeat = 0 and it can be read via channel->read
 *
 *  This function is not suitable for establishing connections to peripherals, if central mode
 *  is supported in the future a separate connection initialization function is implemented.
 *
 *  @param[in] window_interval_ms interval of the windows. At most 10s.
 *  @param[in] window_size_ms     window size within interval. Smaller or equal to interval.
 *  @return RD_SUCCESS  on success.
 *  @return RD_ERROR_INVALID_STATE  If scan is ongoing or if trying to scan on other than 1 MBit / s
 *                                  PHY without extended advertising enabled.
 *  @return RD_ERROR_INVALID_PARAM if window is larger than interval or values are otherwise invalid.
 *
 */
rd_status_t ri_adv_scan_start (const uint32_t window_interval_ms,
                               const uint32_t window_size_ms);

/**
 * @brief Stop ongoing scanning.
 *
 * If advertisement was configured on repeat, calling this function stops advertising
 * before the repeat counter has been reached. This function can be safely called
 * even if no advertising is ongoing.
 *
 * @retval RD_SUCCESS if no more advertisements are ongoing.
 */
rd_status_t ri_adv_scan_stop (void);

/**
 * @brief Select active channels.
 *
 * If advertisement was configured on repeat, calling this function has no effect
 * to advertisements on send queue.
 *
 * @retval RD_SUCCESS if no more advertisements are ongoing.
 */
rd_status_t ri_adv_channels_enable (const ri_radio_channels_t channel);

/**
 * @brief Parse Manufacturer ID from given Bluetooth scan data.
 *
 * @param[in] data pointer to data to parse
 * @param[in] data_length length of data to parse
 * @retval 0 if argument is NULL of manufacturer ID not found
 * @return BLE Manufacturer ID, e.g. 0x0499 for Ruuvi Innovation
 */
uint16_t ri_adv_parse_manuid (uint8_t * const data,
                              const size_t data_length);

/**
 * @brief Set to true to enable advertising 16-bit service UUID in primary advertisement
 * packet.
 *
 * Bluetooth Service UUID lets scanner apps note that your firmware is providing a specific
 * service over BLE GATT. Be sure to configure @ref ri_adv_set_service_uuid, otherwise
 * your program violates Bluetooth license terms. You must have the license to use the
 * configured UUID.
 *
 * When enabled, the field takes 3 bytes of space in advertisement.
 *
 * @param[in] enable_uuid true to enable Service UUID advertisement, false to disable.
 */
void ri_adv_enable_uuid (const bool enable_uuid);

/**
 * @brief Configure Bluetooth GATT Service UUID to advertise in primary advertisement packet.
 *
 * Bluetooth Service UUID lets scanner apps note that your firmware is providing a specific
 * service over BLE GATT. Be sure to enable @ref ri_adv_enable_uuid.
 *
 * When enabled, the field takes 3 bytes of space in advertisement.
 *
 * @param[in] uuid 16-bit UUID to advertise, e.g. 0xFC98 for "Ruuvi Innovations Sensor Data"
 */
void ri_adv_set_service_uuid (const uint16_t uuid);

#endif
