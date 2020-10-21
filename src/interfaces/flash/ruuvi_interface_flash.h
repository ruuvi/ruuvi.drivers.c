#ifndef  RUUVI_INTERFACE_FLASH_H
#define  RUUVI_INTERFACE_FLASH_H

/**
 * @defgroup Flash Flash storage
 * @brief Interface and implementations for storing data into flash in a persistent manner.
 *
 */
/*@{*/
/**
 * @file ruuvi_interface_flash.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-10-11
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Interface functions to persistent flash storage.
 *
 * Flash is stored to pages which are ideally of same size as the underlying physical write/erase unit.
 * Each page may contain N records. Underlying driver is allowed to arrange pages and records in any manner
 * to implement wear leveling.
 * Underlying driver is allowed to trigger garbage collection on write, which makes runtime of operation
 * undeterministic.
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_driver_error.h"
#include <stddef.h>

/** @brief Enable implementation selected by application */
#if RI_FLASH_ENABLED
#  define RUUVI_NRF5_SDK15_FLASH_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

#ifdef APP_FLASH_PAGES
#   define RI_FLASH_PAGES APP_FLASH_PAGES
#else
#   define RI_FLASH_PAGES (10U)
#endif

/**
 * @brief Get total size of usable flash, excluding any overhead bytes
 *
 * @param[out] size Size of useable storage in bytes.
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if size is null
 * @return RD_ERROR_INVALID_STATE if flash storage is not initialized
 * @return error code from stack on other error
 */
rd_status_t ri_flash_total_size_get (size_t * size);

/**
 * @brief Get size of usable page, excluding any overhead bytes
 * If returned value is N, a record of N bytes must fit in one page
 *
 * @param[out] size Size of useable storage in bytes.
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if size is null
 * @return RD_ERROR_INVALID_STATE if flash storage is not initialized
 * @return error code from stack on other error
 */
rd_status_t ri_flash_page_size_get (size_t * size);

/**
 * @brief Get total size of free flash.
 *
 * @param[out] size  size of useable storage in bytes.
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if size is null
 * @return RD_ERROR_INVALID_STATE if flash storage is not initialized
 * @return error code from stack on other error
 */
rd_status_t ri_flash_free_size_get (size_t * size);

/**
 * @brief mark a record for deletion.
 *
 * Note that this only marks the record as freed
 * and does not physically overwrite the flash.
 * The function only starts the operation and returns immediately.
 *
 * @param[in] file_id ID of file which contains the record.
 * @param[in] record_id ID of record to delete.
 *
 * @return RD_SUCCESS if deletion was queued
 * @return RD_ERROR_INVALID_STATE if flash storage is not initialized
 * @return RD_ERROR_BUSY if another operation was ongoing
 * @return RD_ERROR_NOT_FOUND if give record was not found.
 */
rd_status_t ri_flash_record_delete (const uint32_t file_id,
                                    const uint32_t record_id);

/**
 * @brief Set data to record in page
 * Automatically runs garbage collection if record cannot fit on page.
 * Returns after data is successfully written or error has occured.
 *
 * @param[in] page_id ID of a page. Can be random number.
 * @param[in] record_id ID of a record. Can be a random number.
 * @param[in] data_size size data to store
 * @param[in] data pointer to data to store.
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if data is null
 * @return RD_ERROR_INVALID_STATE if flash storage is not initialized
 * @return RD_ERROR_DATA_SIZE if record is too large to fit on page
 * @return RD_ERROR_NO_MEM if this record cannot fit on page.
 * @return error code from stack on other error
 */
rd_status_t ri_flash_record_set (const uint32_t page_id,
                                 const uint32_t record_id, const size_t data_size, const void * const data);

/**
 * @brief Get data from record in page
 *
 * Returns after data is read and ready to be used or error has occured.
 *
 * @param[in] page_id: ID of a page. Can be random number.
 * @param[in] record_id: ID of a record. Can be a random number.
 * @param[in,out] data_size input: Maximum size of data to retrieve. Output: Number of bytes retrieved.
 * @param data[in] pointer to memory which will be filled with retrieved data
 * @return RD_SUCCESS on success
 * @return RD_ERROR_NULL if data is null
 * @return RD_ERROR_INVALID_STATE if flash storage is not initialized
 * @return RD_ERROR_NOT_FOUND if given page id does not exist or if given record_id does not exist on given page.
 * @return error code from stack on other error
 */
rd_status_t ri_flash_record_get (const uint32_t page_id,
                                 const uint32_t record_id, const size_t data_size, void * const data);

/**
 * @brief Run garbage collection.
 *
 * @return RD_SUCCESS on success
 * @return RD_INVALID_STATE if flash is not initialized
 * @return error code from stack on other error
 */
rd_status_t ri_flash_gc_run (void);

/**
 * Initialize flash.
 * After initialization other flash functions can be used.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if flash is already initialized.
 */
rd_status_t ri_flash_init (void);

/**
 * Unintialize flash.
 * After uninitialization only initialization can be used.
 *
 * @retval RD_SUCCESS on success.
 */
rd_status_t ri_flash_uninit (void);

/**
 * @brief Purge flash
 *
 * This function is used to "rescue" flash which cannot be initialized normally,
 * for example after data corruption. Completely erases all data content on flash.
 * Does not erase application files.
 *
 * This function is blocking, returns once flash is cleared.
 */
void ri_flash_purge (void);

/**
 * @brief Check if flash is busy
 *
 * @return True if flash is running an operation.
 * @return False if flash is idle.
 */
bool ri_flash_is_busy (void);
/*@}*/
#endif
