/**
 * Interface functions to persistent flash storage.
 *
 * Flash is stored to pages which are ideally of same size as the underlying physical write/erase unit.
 * Each page may contain N records. Underlying driver is allowed to arrange pages and records in any manner
 * to implement wear leveling.
 * Underlying driver is allowed to trigger garbage collection on write, which makes runtime of operation
 * undeterministic.
 *
 * License: BSD-3
 * Author: Otso Jousimaa <otso@ojousima.net>
 */
#ifndef  RUUVI_INTERFACE_FLASH_H
#define  RUUVI_INTERFACE_FLASH_H

#include "ruuvi_driver_error.h"

#include <stddef.h>

/**
 * Get total size of usable flash, excluding any overhead bytes
 *
 * parameter size: size of useable storage in bytes.
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_ERROR_NULL if size is null
 * return: RUUVI_DRIVER_ERROR_INVALID_STATE if flash storage is not initialized
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_total_size_get(size_t* size);

/**
 * Get size of usable page, excluding any overhead bytes
 * If returned value is N, a record of N bytes must fit in one page
 * and 2 records of size (N/2)-1 etc.
 *
 * parameter size: size of useable storage in bytes.
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_ERROR_NULL if size is null
 * return: RUUVI_DRIVER_ERROR_INVALID_STATE if flash storage is not initialized
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_page_size_get(size_t* size);

ruuvi_driver_status_t ruuvi_interface_flash_free_size_get(size_t* size);

/**
 * Set data to record in page
 * Automatically runs garbage collection if record cannot fit on page.
 *
 * parameter page_id: ID of a page. Can be random number.
 * parameter record_id: ID of a record. Can be a random number.
 * parameter data_size: size data to store
 * parameter data: pointer to data to store.
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_ERROR_NULL if data is null
 * return: RUUVI_DRIVER_ERROR_INVALID_STATE if flash storage is not initialized
 * return: RUUVI_DRIVER_ERROR_DATA_SIZE if record is too large to fit on page
 * return: RUUVI_DRIVER_ERROR_NO_MEM if this record cannot fit on page.
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_record_set(const uint32_t page_id,
    const uint32_t record_id, const size_t data_size, const void* const data);

/**
 * Get data from record in page
 *
 * parameter page_id: ID of a page. Can be random number.
 * parameter record_id: ID of a record. Can be a random number.
 * parameter data_size: size data to store
 * parameter data: pointer to data to store.
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_ERROR_NULL if data is null
 * return: RUUVI_DRIVER_ERROR_INVALID_STATE if flash storage is not initialized
 * return: RUUVI_DRIVER_ERROR_NOT_FOUND if given page id does not exist or if given record_id does not exist on given page.
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_record_get(const uint32_t page_id,
    const uint32_t record_id, const size_t data_size, void* const data);

/**
 * Run garbage collection.
 *
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_INVALID_STATE if flash is not initialized
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_gc_run(void);

/**
 * Initialize flash
 *
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_init(void);
#endif