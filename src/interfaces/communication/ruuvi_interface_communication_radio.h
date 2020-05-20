#ifndef RUUVI_INTERFACE_COMMUNICATION_RADIO_H
#define RUUVI_INTERFACE_COMMUNICATION_RADIO_H

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stdint.h>
#if RI_RADIO_ENABLED
#define RUUVI_NRF5_SDK15_RADIO_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/**
 * @file ruuvi_interface_communication_radio.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-05-20
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Commmon definitions and functions for all radio operations.
 *
 */

/**
 * @defgroup Radio Radio operations.
 */

/** @{ */
/**
 * @brief radio activity event type.
 */
typedef enum
{
    RI_RADIO_BEFORE, //!< Event is before radio goes active, i.e. radio turns on soon.
    RI_RADIO_AFTER   //!< Event is after radio activity, i.e. radio was turned off.
} ri_radio_activity_evt_t;

/**
 *  @brief type of radio modulation to be used.
 */
typedef enum
{
    RI_RADIO_BLE_125KBPS,       //!< Also known as BLE Long Range S=8
    RI_RADIO_BLE_1MBPS,         //!< "Normal" BLE 4 modulation
    /** @brief "Fast BLE". Advertising uses 1MBPS primary advertisement followed by 2 MBit/s extended advertisement. */
    RI_RADIO_BLE_2MBPS
} ri_radio_modulation_t;

/**
 * @brief Bitfield to describe related sensor data
 */
typedef struct
{
    unsigned int channel_37 : 1; //!< BLE channel 37, 2402 MHz.
    unsigned int channel_38 : 1; //!< BLE channel 38, 2426 MHz.
    unsigned int channel_39 : 1; //!< BLE channel 39, 2480 MHz.
} ri_radio_channels_t;

/**
 *  @brief Type of radio activity interrupt.
 *  This is common to all radio modules, i,e, the callback gets called for every radio action.
 *
 *  @param[in] evt Type of radio event
 */
typedef void (*ri_radio_activity_interrupt_fp_t) (const ri_radio_activity_evt_t evt);

/**
 * @brief Check how many radio channels are enabled.
 *
 * @param[in] channels Structure with enabled channels.
 * @return Number of enabled channels.
 */
uint8_t ri_radio_num_channels_get (const ri_radio_channels_t channels);

/**
 * @brief Check if radio supports given modulation.
 *
 * @param[in] modulation Modulation to check.
 * @retval true If given modulation is supported by radio.
 * @retval false If given modulation is not supported by radio.
 */
bool ri_radio_supports (ri_radio_modulation_t modulation);

/**
 *  @brief Enable radio stack for an user.
 *  This function also starts radio activity callbacks internally.
 *
 *  @param[in] modulation Modulation for radio operations.
 *                        If 2 MBPS is defined, primary advertising PHY is 1 MBPS and
 *                        secondary PHY is 2 MBPS.
 *                        Note: if other end of communication requests different speed,
 *                        the implementation should support it if applicable to board.
 *  @retval    RD_SUCCESS on success
 *  @retval    RD_ERROR_INVALID_STATE if radio is already initialized.
 *  @retval    RD_ERROR_INVALID_PARAM if trying to initialize radio with unsupported modulation.
 */
rd_status_t ri_radio_init (const ri_radio_modulation_t modulation);

/**
 *  @brief Release radio stack.
 *
 * This function also stops the internal radio activity callbacks
 *
 *  @retval    RD_SUCCESS on success
 *  @retval    RD_ERROR_FORBIDDEN if radio is in use by other user
 *  @note      It's not possible to uninitialize radio from another user than original initializer.
 */
rd_status_t ri_radio_uninit ();

/**
 * Writes maximum 64-bit unique address of the device to the pointer. This address
 * may be changed during runtime. The address is identifier of the device on radio network,
 * such as BLE MAC address.
 *
 * @param[out] address Value of radio address, i.e. MAC address.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_NOT_SUPPORTED if address cannot be returned on given platform
 */
rd_status_t ri_radio_address_get (uint64_t * const address);

/**
 * Configures maximum 64-bit unique address of the device to the radio.
 * The address is identifier of the device on radio network,
 * such as BLE MAC address.
 *
 * @param[in] address Address to configure, MSB first.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NOT_SUPPORTED if address cannot be configured on given platform.
 * @retval RD_ERROR_INVALID_ADDR if the address does not match protocol rules and cannot or is not converted automatically.
 * @retval RD_ERRO_BUSY if radio stack is doing some other operation, try later.
 * @retval RD_ERROR_INVALID_STATE if radio is in state where address is required, such as advertising, connected or scanning.
 * @note   The implementation is allowed to enforce protocol rules, for example BLE MAC will be masked with 0x00 00 [0b11xx]X XX XX XX XX XX XX and sent LSB first.
 */
rd_status_t ri_radio_address_set (uint64_t const address);

/**
 * @brief Setup radio activity interrupt
 *
 * This function allows driver to notify application on all radio activity.
 * @param[in] handler Function to call on radio event. Set to @c NULL to disable radio-level callback.
 */
void ri_radio_activity_callback_set (const ri_radio_activity_interrupt_fp_t handler);

/**
 * @brief Check if radio is initialized
 *
 * @return true if radio is initialized, false otherwise.
 */
bool ri_radio_is_init();

/**
 * @brief Get the modulation used by application.
 *
 * Modulation is decided at initialization and used by all radio modules.
 * However, other modules are allowed to switch to other modulation as a part
 * of their operation, for example GATT can use 1 MBit/s when setting up connection and
 * negotiate 2 MBit/s for the remainder of connection.
 *
 * @param[out] p_modulation Modulation used by default.
 * @retval RD_SUCCESS if modulation was written to pointer
 * @retval RD_ERROR_NULL if p_modulation is NULL
 * @retval RD_ERROR_INVALID_STATE if radio is not initialized.
 */
rd_status_t ri_radio_get_modulation (ri_radio_modulation_t * const p_modulation);

/** @} */
#endif
