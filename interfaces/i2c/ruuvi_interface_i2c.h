#ifndef RUUVI_INTERFACE_I2C_H
#define RUUVI_INTERFACE_I2C_H
#include "ruuvi_driver_error.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup I2C I2C functions
 * @brief Functions for using I2C bus
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_spi.h
* @brief Interface for I2C operations
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-01-31
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */

/**
 * Clock speed
 */
typedef enum
{
  RUUVI_INTERFACE_I2C_FREQUENCY_100k, //!< 100 kbps
  RUUVI_INTERFACE_I2C_FREQUENCY_250k, //!< 250 kbps
  RUUVI_INTERFACE_I2C_FREQUENCY_400k, //!< 400 kbps
} ruuvi_interface_i2c_frequency_t;

/**
 * Configuration for initializing I2C
 */
typedef struct
{
  uint8_t sda;                              //!< pin number of SDA
  uint8_t scl;                              //!< pin number of SCL
  ruuvi_interface_i2c_frequency_t
  frequency; //!< Frequency of I2C Bus, see @ref ruuvi_interface_i2c_frequency_t
} ruuvi_interface_i2c_init_config_t;

/**
 * @brief Initialize I2C driver with given settings
 *
 * @param[in] config Configuration of the I2C peripheral.
 * @return error code from the stack, RUUVI_DRIVER_SUCCESS if no error occurred
 **/
ruuvi_driver_status_t ruuvi_interface_i2c_init(const ruuvi_interface_i2c_init_config_t*
    const config);

/**
 * @brief Check if i2c driver is initialized
 *
 * @return true if I2C is initialized, false otherwise.
 **/
bool ruuvi_interface_i2c_is_init();

/**
 * @brief I2C read function.
 *
 * Function is blocking and will not sleep while transaction is ongoing.
 *
 * @param[in] address 7-bit I2C address of the device, without R/W bit.
 * @param[out] p_rx pointer to data to be received
 * @param[in] rx_len length of data to be received
 **/
ruuvi_driver_status_t ruuvi_interface_i2c_read_blocking(const uint8_t address,
    uint8_t* const p_rx, const size_t rx_len);

/**
 * @brief I2C read function.
 *
 * Function is blocking and will not sleep while transaction is ongoing.
 *
 * @param[in] address 7-bit I2C address of the device, without R/W bit.
 * @param[out] p_tx pointer to data to be transmitted
 * @param[in] tx_len length of data to be transmitted
 * @param[in] stop @c true to transmit stop condition after read, @c false to hold bus active.
 **/
ruuvi_driver_status_t ruuvi_interface_i2c_write_blocking(const uint8_t address,
    uint8_t* const p_tx, const size_t tx_len, const bool stop);
/* @} */
#endif