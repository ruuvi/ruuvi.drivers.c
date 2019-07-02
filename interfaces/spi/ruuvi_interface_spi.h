#ifndef RUUVI_INTERFACE_SPI_H
#define RUUVI_INTERFACE_SPI_H
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup SPI SPI functions
 * @brief Functions for using SPI bus
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_spi.h
* @brief Interface for SPI operations
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-05-30
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 */

/**
 * SPI modes. Defines clock polarity and phase
 */
typedef enum
{
  RUUVI_INTERFACE_SPI_MODE_0, //!< CPOL = 0, CPHA = 0
  RUUVI_INTERFACE_SPI_MODE_1, //!< CPOL = 0, CPHA = 1
  RUUVI_INTERFACE_SPI_MODE_2, //!< CPOL = 1, CPHA = 0
  RUUVI_INTERFACE_SPI_MODE_3  //!< CPOL = 1, CPHA = 1
} ruuvi_interface_spi_mode_t;

/**
 * Clock speed
 */
typedef enum
{
  RUUVI_INTERFACE_SPI_FREQUENCY_1M, //!< 1 Mbps
  RUUVI_INTERFACE_SPI_FREQUENCY_2M, //!< 2 Mbps
  RUUVI_INTERFACE_SPI_FREQUENCY_4M, //!< 4 Mbps
  RUUVI_INTERFACE_SPI_FREQUENCY_8M, //!< 8 Mbps
} ruuvi_interface_spi_frequency_t;

/**
 * Configuration for initializing SPI
 */
typedef struct
{
  ruuvi_interface_gpio_id_t mosi;           //!< pin number of MOSI
  ruuvi_interface_gpio_id_t miso;           //!< pin number of MISO
  ruuvi_interface_gpio_id_t sclk;           //!< pin number of SCLK
  ruuvi_interface_gpio_id_t*
  ss_pins;       //!< array of SPI pins, can be freed after function exits
  size_t ss_pins_number;                    //!< sizeof ss_pins
  ruuvi_interface_spi_frequency_t
  frequency; //!< Frequency of SPI Bus, see @ref ruuvi_interface_spi_frequency_t
  ruuvi_interface_spi_mode_t
  mode;           //!< Mode of SPI Bus, see @ref ruuvi_interface_spi_mode_t
} ruuvi_interface_spi_init_config_t;

/**
 * @brief Initialize SPI driver with given settings
 *
 * This function also handles configuring the SS and SPI GPIO pins as outputs
 *
 * @param config Configuration of the SPI peripheral. Will setup given slave select pins as outputs.
 * @return RUUVI_DRIVER_SUCCESS if no error occurred
 * @return RUUVI_DRIVER_ERROR_INVALID_STATE if SPI was already initialized
 **/
ruuvi_driver_status_t ruuvi_interface_spi_init(const ruuvi_interface_spi_init_config_t*
    const config);

/**
 * @brief check if SPI interface is already initialized.
 * 
 * @return @c true if SPI is initialized
 * @return @c false otherwise
 */
bool ruuvi_interface_spi_is_init();

/**
 * @brief Uninitialize SPI driver.
 * 
 * This function might not uninitialize the SPI GPIO pins, only the underlying peripheral.
 * Uninitialized GPIOs explicitly if that is required.
 *
 * @return RUUVI_DRIVER_SUCCESS
 * @warning Uninitializes the SPI peripheral, may or may not uninitialize the associated gpio pins
 **/
ruuvi_driver_status_t ruuvi_interface_spi_uninit();

/**
 * @brief SPI transfer function.
 *
 * Full-duplex SPI. Clocks out <tt>MAX(tx_len, rx_len)</tt> bytes. It is allowed to send different
 * length transactions, tx will clock out @c 0xFF if there is
 * less bytes in TX than RX. Does not use slave select pins.
 * RX will start at the same time as TX, i.e. one byte address + read commands will generally have
 * {0x00, data} in rx buffer. Function is blocking and will not sleep while transaction is ongoing.
 *
 * @param p_tx pointer to data to be sent, can be NULL if tx_len is 0.
 * @param tx_len length of data to be sent
 * @param p_rx pointer to data to be received, can be NULL if rx_len is 0.
 * @param rx_len length of data to be received
 * @warning First byte in RX is generally @c 0x00 if you're reading external sensor.
 **/
ruuvi_driver_status_t ruuvi_interface_spi_xfer_blocking(const uint8_t* const p_tx,
    const size_t tx_len, uint8_t* const p_rx, const size_t rx_len);
/* @} */
#endif