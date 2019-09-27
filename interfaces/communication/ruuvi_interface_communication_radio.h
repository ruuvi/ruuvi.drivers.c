#ifndef RUUVI_INTERFACE_COMMUNICATION_RADIO_H
#define RUUVI_INTERFACE_COMMUNICATION_RADIO_H
/**
 * @file ruuvi_interface_communication_radio.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-09-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Commmon definitions and functions for all radio operations. 
 *
 */

#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief radio activity event type.
 */
typedef enum
{
  RUUVI_INTERFACE_COMMUNICATION_RADIO_BEFORE,         //!< Event is before radio goes active, i.e. radio turns on soon.
  RUUVI_INTERFACE_COMMUNICATION_RADIO_AFTER           //!< Event is after radio activity, i.e. radio was turned off recently.
} ruuvi_interface_communication_radio_activity_evt_t;

/**
 * @brief a definition of radio user. - TODO: setup a bitfield for multiusers?
 */
typedef enum
{
  RUUVI_INTERFACE_COMMUNICATION_RADIO_UNINIT = 0,    //!< Radio is not in use
  RUUVI_INTERFACE_COMMUNICATION_RADIO_ADVERTISEMENT, //!< Radio is used for advertising
  RUUVI_INTERFACE_COMMUNICATION_RADIO_GATT,          //!< Radio is used for GATT connections
  RUUVI_INTERFACE_COMMUNICATION_RADIO_MESH           //!< Radio is used by a mesh protocol, standard or proprietary
} ruuvi_interface_communication_radio_user_t;

/**
 *  @brief Type of radio activity interrupt. 
 *  This is common to all radio modules, i,e, the callback gets called for every radio action.
 *
 *  @param[in] evt Type of radio event 
 */
typedef void(*ruuvi_interface_communication_radio_activity_interrupt_fp_t)(
  const ruuvi_interface_communication_radio_activity_evt_t evt);

/**
 *  @brief Enable radio stack for an user
 *  This function also starts radio activity callbacks internally
 *
 *  @param[in] handle User ID for radio.
 *  @return    RUUVI_DRIVER_SUCCESS on success
 *  @return    RUUVI_DRIVER_ERROR_INVALID_STATE if radio is already initialized.
 *  @note      it's possible to use radio even if the radio is conserved by other user, but there may be concurrency issues. 
 */
ruuvi_driver_status_t ruuvi_interface_communication_radio_init(
  const ruuvi_interface_communication_radio_user_t handle);
/**
 *  @brief Release radio stack
 *
 * This function also stops the internal radio activity callbacks
 *
 *  @param[in] handle User ID for radio.
 *  @return    RUUVI_DRIVER_SUCCESS on success
 *  @return    RUUVI_DRIVER_ERROR_FORBIDDEN if radio is in use by other user
 *  @note      It's not possible to uninitialize radio from another user than original initializer.
 */
ruuvi_driver_status_t ruuvi_interface_communication_radio_uninit(
  const ruuvi_interface_communication_radio_user_t handle);

/**
 * Writes maximum 64-bit unique address of the device to the pointer. This address
 * may be changed during runtime. The address is identifier of the device on radio network,
 * such as BLE MAC address.
 *
 * parameter address: Output, value of address.
 *
 * return RUUVI_DRIVER_SUCCESS on success
 * return RUUVI_DRIVER_ERROR_NOT_SUPPORTED if address cannot be returned on given platform
 */
ruuvi_driver_status_t ruuvi_interface_communication_radio_address_get(
  uint64_t* const address);

/**
 * Configures maximum 64-bit unique address of the device to the radio. 
 * The address is identifier of the device on radio network,
 * such as BLE MAC address.
 *
 * @param[in] address Address to configure, MSB first.
 *
 * @return RUUVI_DRIVER_SUCCESS on success.
 * @return RUUVI_DRIVER_ERROR_NOT_SUPPORTED if address cannot be configured on given platform.
 * @return RUUVI_DRIVER_ERROR_INVALID_ADDR if the address does not match protocol rules and cannot or is not converted automatically.
 * @return RUUVI_DRIVER_ERRO_BUSY if radio stack is doing some other operation, try later.
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if radio is in state where address is required, such as advertising, connected or scanning. 
 * @note   The implementation is allowed to enforce protocol rules, for example BLE MAC will be masked with 0x00 00 [0b11xx]X XX XX XX XX XX XX and sent LSB first.
 */
ruuvi_driver_status_t ruuvi_interface_communication_radio_address_set(
  uint64_t const address);

/**
 * @brief Setup radio activity interrupt
 *
 * This function allows driver to notify application on all radio activity. Calling this function
 * will not start the callbacks, callbacks are started on radio initialization. 
 * @param[in] handler Function to call on radio event. Set to @c NULL to disable radio-level callback, however module-level callbacks (advertising, GATT etc) will be called.
 */
void ruuvi_interface_communication_radio_activity_callback_set(
  const ruuvi_interface_communication_radio_activity_interrupt_fp_t handler);

/**
 * @brief Check if radio is initialized
 *
 * @return true if radio is initialized, false otherwise.
 */
bool ruuvi_interface_communication_radio_is_init();

#endif