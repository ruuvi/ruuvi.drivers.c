/**
 *  This file contains interface to SPI platform drivers. 
 *. Implementations are decided by compiler flags
 */

#ifndef SPI_H
#define SPI_H
#include "ruuvi_error.h"
#include <stdbool.h>
/**
 * @brief initialize SPI driver with default settings
 * @return 0 on success, NRF error code on error
 */
ruuvi_status_t spi_init(void);

/**
 * @brief uninitialize SPI driver with default settings
 * @return 0 on success, NRF error code on error
 */
ruuvi_status_t spi_uninit(void);

/**
 * @brief platform SPI write command for Bosch drivers
 */
int8_t spi_bosch_platform_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief platform SPI read command for Bosch drivers
 */
int8_t spi_bosch_platform_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief platform SPI write command for LIS2DH12 driver
 */
int32_t spi_lis2dh12_platform_write(void* dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief platform SPI read command for LIS2DH12 driver
 */
int32_t spi_lis2dh12_platform_read(void* dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief platform SPI write command for STM drivers
 */
int32_t spi_stm_platform_write(void* dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief platform SPI read command for STM drivers
 */
int32_t spi_stm_platform_read(void* dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief generic platform SPI tx command.
 *
 * @param ss_pin Slave select pin of target device
 * @param tx Pointer to TX data
 * @param tx_len size of tx data
 * @param rx Pointer to rx data. Might be incremented by one byte by this function
 * @param rx_len length of rx data. As input, number of bytes to read, including increment. As output, size of final data.
 * @param skip first. If true, rx buffer will be incremented by one and rx_len will be decremented by one. This is useful when reading addressed data from device.
 */
ruuvi_status_t spi_generic_platform_xfer_blocking(const uint8_t ss_pin, uint8_t* const tx, const size_t tx_len, uint8_t** rx, size_t* rx_len, bool skip_first);
#endif