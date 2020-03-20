#ifndef RUUVI_INTERFACE_COMMUNICATION_BLE_ADVERTISING_H
#define RUUVI_INTERFACE_COMMUNICATION_BLE_ADVERTISING_H

/**
 * @file ruuvi_interface_communication_ble_advertising.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-03-02
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Commmon definitions and functions for all BLE advertising.
 *
 */
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_communication.h"
#include "ruuvi_interface_communication_radio.h"
#include <stdint.h>

#if RI_ADV_ENABLED
#define RUUVI_NRF5_SDK15_ADV_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/* @brief number of bytes in a MAC address */
#define BLE_MAC_ADDRESS_LENGTH 6

/* @brief Number of bytes in a BLE scan data.
 *
 * @note This van be up to 256 if using extended advertising.
  */
#define BLE_SCAN_DATA_LENGTH   31

/** @brief Alloved advertisement types */
typedef enum
{
    NONCONNECTABLE_NONSCANNABLE, //!< Nonconnectable, nonscannable
    CONNECTABLE_NONSCANNABLE,    //!< Connectable, nonscannable
    CONNECTABLE_SCANNABLE,       //!< Connectable, scannable
    NONCONNECTABLE_SCANNABLE     //!< Nonconnectable, scannable
} ri_adv_type_t;

/**
 * @brief Advertisement report from scanner
 *
 */
typedef struct
{
    uint8_t addr[BLE_MAC_ADDRESS_LENGTH];  //<! MAC address, MSB first
    int8_t rssi;      //!< RSSI of advertisement
    uint8_t data[BLE_SCAN_DATA_LENGTH]; //!< Full payload of the advertisement
    size_t data_len;  //!< Length of received data
} ri_adv_scan_t;

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
rd_status_t ri_adv_init (ri_communication_t * const channel);

/*
 * @brief Uninitializes radio hardware, advertising module and scanning module.
 *
 * @param[out] channel Communication api to send and receive data via advertisements.
 *
 * @retval RD_SUCCESS on success or if radio was not initialized.
 * @retval RD_ERROR_INVALID_STATE if radio hardware was initialized by another radio module.
 */
rd_status_t ri_adv_uninit (ri_communication_t * const channel);

/*
 * @brief Setter for broadcast advertisement interval.
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
 * @brief Set manufacturer ID of manufacturer specific advertisement
 *
 * @param[in] id ID of manufacturer, MSB first. E.g. 0x0499 for Ruuvi Innovations.
 * @retval RD_SUCCESS
 */
rd_status_t ri_adv_manufacturer_id_set (const uint16_t id);

/**
 * @brief Set radio TX power.
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

/** @brief setup scan window interval and window size.
 *
 *  The scan window interval must be larger or equivalent to window size.
 *  Example: Interval 1000 ms, window size 100 ms.
 *  The scanning will scan 100 ms at channel 37, wait 900 ms, scan 100 ms at channel 38,
 *  wait 900 ms, scan 100 ms at channel 39, wait 900 ms and start again at channel 37.
 *
 *  @param[in] window_interval_ms interval of the window.
 *  @param[in] window_size_ms     window size within interval.
 *  @return RD_SUCCESS  on success.
 *  @return RD_ERROR_INVALID_STATE if scan is ongoing.
 *  @return RD_ERROR_INVALID_PARAM if window is larger than interval or values are otherwise invalid.
 */
rd_status_t ri_adv_rx_interval_set (const uint32_t window_interval_ms,
                                    const uint32_t window_size_ms);

/** @brief get scan window interval and window size.
 *
 *
 *  @param[out] window_interval_ms interval of the window.
 *  @param[out] window_size_ms     window size within interval.
 *  @return RD_SUCCESS  on success.
 *  @return RD_ERROR_NULL if either pointer is NULL.
 *  @return RD_ERROR_INVALID_PARAM if window is larger than interval or values are
 *                                 otherwise invalid.
 */
rd_status_t ri_adv_rx_interval_get (uint32_t * window_interval_ms,
                                    uint32_t * window_size_ms);

/**
 * @brief Configure advertising data with a scan response.
 * The scan response must be separately enabled by setting advertisement type as
 * scannable.
 *
 * @param[in] name Name to advertise, NULL-terminated string.
 *                 Max 27 characters if not advertising NUS, 10 characters if NUS is
 *                 advertised. NULL won't be included in advertisement.
 * @param[in] advertise_nus True to include Nordic UART Service UUID in scan response.
 * @retval @ref RD_SUCCESS on success
 * @retval @ref RD_ERROR_NULL if name is NULL
 * @retval @ref RD_ERROR_DATA_SIZE if name will be cut. However the abbreviated name will
 *              be set.
 */
rd_status_t ri_adv_scan_response_setup (const char * const name,
                                        const bool advertise_nus);

/**
 * @brief Configure the type of advertisement.
 *
 * Advertisement can be connectable, scannable, both or neither.
 * It is possible to setup scannable advertisement before setting up scan response,
 * in this case scan response will be 0-length and empty until scan resonse if configured.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if advertisements are not initialized.
 */
rd_status_t ri_adv_type_set (ri_adv_type_t type);

/**
 * @brief Stop ongoing advertisements.
 *
 * If advertisement was configured on repeat, calling this function stops advertising
 * before the repeat counter has been reached. This function can be safely called
 * even if no advertising is ongoing.
 *
 * @retval RD_SUCCESS if no more advertisements are ongoing.
 */
rd_status_t ri_adv_stop (void);

/**
 * @brief Select active channels.
 *
 * If advertisement was configured on repeat, calling this function stops advertising
 * before the repeat counter has been reached. This function can be safely called
 * even if no advertising is ongoing.
 *
 * @retval RD_SUCCESS if no more advertisements are ongoing.
 */
rd_status_t ri_adv_channels_enable(const ri_radio_channels_t channel);

#endif