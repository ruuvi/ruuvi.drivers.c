/**
 * Copyright (c) 2017 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "ruuvi_driver_enabled_modules.h"
#if RUUVI_NRF5_SDK15_FLASH_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
#include "ruuvi_interface_flash.h"
#include "ruuvi_interface_log.h"
#include "ruuvi_interface_yield.h"

#include "fds.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "sdk_config.h"
#include "nrf_fstorage.h"
#include "fds_internal_defs.h"
#if (FDS_BACKEND == NRF_FSTORAGE_SD)
#include "nrf_fstorage_sd.h"
#elif (FDS_BACKEND == NRF_FSTORAGE_NVMC)
#include "nrf_fstorage_nvmc.h"
#else
#error Invalid FDS backend.
#endif

#include <string.h>

#define LOG_LEVEL RUUVI_INTERFACE_LOG_DEBUG

NRF_FSTORAGE_DEF(nrf_fstorage_t m_fs1) =
{
    // The flash area boundaries are set in fds_init().
    .evt_handler = NULL,
};

static size_t m_number_of_pages = 0;

// convert FDS error to ruuvi error
static ruuvi_driver_status_t fds_to_ruuvi_error(ret_code_t err_code)
{
  switch(err_code)
  {
    case FDS_SUCCESS:
      return RUUVI_DRIVER_SUCCESS;

    case FDS_ERR_OPERATION_TIMEOUT:
      return RUUVI_DRIVER_ERROR_TIMEOUT;

    case FDS_ERR_NOT_INITIALIZED:
      return RUUVI_DRIVER_ERROR_INVALID_STATE;

    case FDS_ERR_UNALIGNED_ADDR:
      return RUUVI_DRIVER_ERROR_INTERNAL;

    case FDS_ERR_INVALID_ARG:
      return RUUVI_DRIVER_ERROR_INVALID_PARAM;

    case FDS_ERR_NULL_ARG:
      return RUUVI_DRIVER_ERROR_NULL;

    case FDS_ERR_NO_OPEN_RECORDS:
      return RUUVI_DRIVER_ERROR_INVALID_STATE;

    case FDS_ERR_NO_SPACE_IN_FLASH:
      return RUUVI_DRIVER_ERROR_NO_MEM;

    case FDS_ERR_NO_SPACE_IN_QUEUES:
      return RUUVI_DRIVER_ERROR_BUSY;

    case FDS_ERR_RECORD_TOO_LARGE:
      return RUUVI_DRIVER_ERROR_DATA_SIZE;

    case FDS_ERR_NOT_FOUND:
      return RUUVI_DRIVER_ERROR_NOT_FOUND;

    case FDS_ERR_NO_PAGES:
      return RUUVI_DRIVER_ERROR_NO_MEM;

    case FDS_ERR_USER_LIMIT_REACHED:
      return RUUVI_DRIVER_ERROR_RESOURCES;

    case FDS_ERR_CRC_CHECK_FAILED:
      return RUUVI_DRIVER_ERROR_SELFTEST;

    case FDS_ERR_BUSY:
      return RUUVI_DRIVER_ERROR_BUSY;

    case FDS_ERR_INTERNAL:
    default:
      return RUUVI_DRIVER_ERROR_INTERNAL;
  }

  return RUUVI_DRIVER_ERROR_INTERNAL;
}


/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;
/* Flag to check fds processing status. */
static bool volatile m_fds_processing;

static void fds_evt_handler(fds_evt_t const* p_evt)
{
  switch(p_evt->id)
  {
    case FDS_EVT_INIT:
      if(p_evt->result == FDS_SUCCESS)
      {
        m_fds_initialized = true;
        ruuvi_interface_log(LOG_LEVEL, "FDS init\r\n");
      }

      break;

    case FDS_EVT_WRITE:
    {
      if(p_evt->result == FDS_SUCCESS)
      {
        ruuvi_interface_log(LOG_LEVEL, "Record written\r\n");
        m_fds_processing = false;
      }
    }
    break;

    case FDS_EVT_UPDATE:
    {
      if(p_evt->result == FDS_SUCCESS)
      {
        ruuvi_interface_log(LOG_LEVEL, "Record updated\r\n");
        m_fds_processing = false;
      }
    }
    break;

    case FDS_EVT_DEL_RECORD:
    {
      if(p_evt->result == FDS_SUCCESS)
      {
        ruuvi_interface_log(LOG_LEVEL, "Record deleted\r\n");
        m_fds_processing = false;
      }
    }
    break;

    case FDS_EVT_DEL_FILE:
    {
      if(p_evt->result == FDS_SUCCESS)
      {
        ruuvi_interface_log(LOG_LEVEL, "File deleted\r\n");
        m_fds_processing = false;
      }
    }
    break;

    case FDS_EVT_GC:
    {
      if(p_evt->result == FDS_SUCCESS)
      {
        ruuvi_interface_log(LOG_LEVEL, "Garbage collected\r\n");
        m_fds_processing = false;
      }
    }
    break;

    default:
      break;
  }
}

/**
 * Get total size of usable flash, including any overhead bytes
 *
 * parameter size: size of useable storage in bytes.
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_ERROR_NULL if size is null
 * return: RUUVI_DRIVER_ERROR_INVALID_STATE if flash storage is not initialized
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_total_size_get(size_t* size)
{
  if(NULL == size) { return RUUVI_DRIVER_ERROR_NULL; }

  if(false == m_fds_initialized) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ruuvi_interface_flash_page_size_get(size);
  *size *= m_number_of_pages;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_flash_free_size_get(size_t* size)
{
  if(NULL == size) { return RUUVI_DRIVER_ERROR_NULL; }

  if(false == m_fds_initialized) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  // Read filesystem status
  fds_stat_t stat = {0};
  ret_code_t rc = fds_stat(&stat);
  *size = stat.largest_contig * 4;
  return fds_to_ruuvi_error(rc);
}

/**
 * Get size of usable page, including any overhead bytes
 *
 * parameter size: size of useable storage in bytes.
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_ERROR_NULL if size is null
 * return: RUUVI_DRIVER_ERROR_INVALID_STATE if flash storage is not initialized
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_page_size_get(size_t* size)
{
  if(NULL == size) { return RUUVI_DRIVER_ERROR_NULL; }

  if(false == m_fds_initialized) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  *size = FDS_VIRTUAL_PAGE_SIZE;
  return RUUVI_DRIVER_SUCCESS;
}

ruuvi_driver_status_t ruuvi_interface_flash_record_delete(const uint32_t page_id,
    const uint32_t record_id)
{
  if(false == m_fds_initialized) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};
  ret_code_t rc = fds_record_find(page_id, record_id, &desc, &tok);

  if(FDS_SUCCESS == rc)
  {
    rc = fds_record_delete(&desc);
  }

  return fds_to_ruuvi_error(rc);
}

/**
 * Set data to record in page. Writes a new record if given record ID does not exist in page.
 * Updates record if it already exists.
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
    const uint32_t record_id, const size_t data_size, const void* const data)
{
  if(NULL == data) { return RUUVI_DRIVER_ERROR_NULL; }

  if(false == m_fds_initialized) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};
  /* A record structure. */
  fds_record_t const record =
  {
    .file_id           = page_id,
    .key               = record_id,
    .data.p_data       = data,
    /* The length of a record is always expressed in 4-byte units (words). */
    .data.length_words = (data_size + 3) / sizeof(uint32_t),
  };
  ret_code_t rc = fds_record_find(page_id, record_id, &desc, &tok);

  // If record was found
  if(FDS_SUCCESS == rc)
  {
    /* Start write */
    m_fds_processing = true;
    rc = fds_record_update(&desc, &record);
    err_code |= fds_to_ruuvi_error(rc);

    if(RUUVI_DRIVER_SUCCESS != err_code)
    {
      m_fds_processing = false;
      return err_code;
    }

    /* Wait for process to complete */
    while(m_fds_processing);
  }
  // If record was not found
  else
  {
    /* Start write */
    m_fds_processing = true;
    desc.record_id = record_id;
    rc = fds_record_write(&desc, &record);
    err_code |= fds_to_ruuvi_error(rc);

    if(RUUVI_DRIVER_SUCCESS != err_code)
    {
      m_fds_processing = false;
      return err_code;
    }

    /* Wait for process to complete */
    while(m_fds_processing);
  }

  return err_code;
}

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
    const uint32_t record_id, const size_t data_size, void* const data)
{
  if(NULL == data) { return RUUVI_DRIVER_ERROR_NULL; }

  if(false == m_fds_initialized) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};
  ret_code_t rc = fds_record_find(page_id, record_id, &desc, &tok);
  err_code |= fds_to_ruuvi_error(rc);

  // If file was found
  if(FDS_SUCCESS == rc)
  {
    fds_flash_record_t record = {0};
    /* Open the record and read its contents. */
    rc = fds_record_open(&desc, &record);
    err_code |= fds_to_ruuvi_error(rc);

    // Check length
    if(record.p_header->length_words * 4 > data_size) { return RUUVI_DRIVER_ERROR_DATA_SIZE; }

    /* Copy the data from flash into RAM. */
    memcpy(data, record.p_data, record.p_header->length_words * 4);
    /* Close the record when done reading. */
    rc = fds_record_close(&desc);
    err_code |= fds_to_ruuvi_error(rc);
  }

  return err_code;
}

/**
 * Run garbage collection.
 *
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: RUUVI_DRIVER_INVALID_STATE if flash is not initialized
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_gc_run(void)
{
  if(false == m_fds_initialized) { return RUUVI_DRIVER_ERROR_INVALID_STATE; }

  m_fds_processing = true;
  ret_code_t rc = fds_gc();

  while(m_fds_processing);

  return fds_to_ruuvi_error(rc);
}

/**
 * Initialize flash
 *
 * return: RUUVI_DRIVER_SUCCESS on success
 * return: error code from stack on other error
 */
ruuvi_driver_status_t ruuvi_interface_flash_init(void)
{
  ruuvi_driver_status_t err_code = RUUVI_DRIVER_SUCCESS;
  ret_code_t rc = NRF_SUCCESS;
  /* Register first to receive an event when initialization is complete. */
  (void) fds_register(fds_evt_handler);
  rc = fds_init();
  err_code |= fds_to_ruuvi_error(rc);
  // Crash here in case of error to avoid looping forever
  RUUVI_DRIVER_ERROR_CHECK(err_code, RUUVI_DRIVER_SUCCESS);

  // Wait for init ok
  while(!m_fds_initialized);

  // Read filesystem status
  fds_stat_t stat = {0};
  rc = fds_stat(&stat);
  m_number_of_pages = stat.pages_available;
  err_code |= fds_to_ruuvi_error(rc);
  return err_code;
}

static uint32_t flash_end_addr(void)
{
    uint32_t const bootloader_addr = NRF_UICR->NRFFW[0];
    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;
#ifndef NRF52810_XXAA
    uint32_t const code_sz         = NRF_FICR->CODESIZE;
#else
    // Number of flash pages, necessary to emulate the NRF52810 on NRF52832.
    uint32_t const code_sz         = 48;
#endif

    return (bootloader_addr != 0xFFFFFFFF) ? bootloader_addr : (code_sz * page_sz);
}


static void flash_bounds_set(void)
{
    uint32_t flash_size  = (FDS_PHY_PAGES * FDS_PHY_PAGE_SIZE * sizeof(uint32_t));
    m_fs1.end_addr   = flash_end_addr();
    m_fs1.start_addr = m_fs1.end_addr - flash_size;
}

/** Erase FDS in case of error */
void ruuvi_interface_flash_purge(void)
{
	flash_bounds_set();

	for (int p = 0; p < FDS_VIRTUAL_PAGES; p++)
	{

		#if   defined(NRF51)
			int erase_unit = 1024;
		#elif defined(NRF52_SERIES)
			int erase_unit = 4096;
		#endif

		int page = m_fs1.start_addr/erase_unit + p; // erase unit == virtual page size

		ret_code_t rc = sd_flash_page_erase(page);
                ruuvi_interface_delay_ms(200);
                RUUVI_DRIVER_ERROR_CHECK(rc, RUUVI_DRIVER_SUCCESS);
	}
		
}

#endif