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
#include "ruuvi_interface_flash.h"
#if RUUVI_NRF5_SDK15_FLASH_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_nrf5_sdk15_error.h"
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

/* Inspired by CODE_* macros in app_util.h */
#if defined(__SES_ARM)
/* In flash_placement.xml
 *
 * <ProgramSection alignment="4" keep="Yes" load="No" name=".storage_flash"
 *  start="0x60000" size="0x15000" address_symbol="__start_storage_flash"
 *  end_symbol="__stop_storage_flash" />
 *
 */
extern uint32_t __start_storage_flash;
extern uint32_t __stop_storage_flash;

#define FSTORAGE_SECTION_START ((uint32_t)&__start_storage_flash)
#define FSTORAGE_SECTION_END   ((uint32_t)&__stop_storage_flash)
#elif defined ( __GNUC__ )
/* In linker:
 *
 * .storage_flash 0x60000 (NOLOAD) :
 * {
 *  __start_storage_flash = .;
 *  KEEP(*(SORT(.storage_flash*)))
 *  __stop_storage_flash = . + 0x15000;
 * } > FLASH
 */
extern uint32_t __start_storage_flash;
extern uint32_t __stop_storage_flash;
#define FSTORAGE_SECTION_START ((uint32_t)&__start_storage_flash)
#define FSTORAGE_SECTION_END   ((uint32_t)&__stop_storage_flash)
#endif

#include <string.h>

/**
 * @addtogroup Flash Flash storage
 * @brief Interface and implementations for storing data into flash in a persistent manner.
 *
 */
/** @{ */
/**
 * @file ruuvi_nrf5_sdk15_flash.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-12-10
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @brief Implement persistent flash storage.
 *
 */

#define LOG_LEVEL RI_LOG_LEVEL_DEBUG

static size_t m_number_of_pages = 0;

/** @brief convert FDS error to ruuvi error */
static rd_status_t fds_to_ruuvi_error (ret_code_t err_code)
{
    switch (err_code)
    {
        case FDS_SUCCESS:
            return RD_SUCCESS;

        case FDS_ERR_OPERATION_TIMEOUT:
            return RD_ERROR_TIMEOUT;

        case FDS_ERR_NOT_INITIALIZED:
            return RD_ERROR_INVALID_STATE;

        case FDS_ERR_UNALIGNED_ADDR:
            return RD_ERROR_INTERNAL;

        case FDS_ERR_INVALID_ARG:
            return RD_ERROR_INVALID_PARAM;

        case FDS_ERR_NULL_ARG:
            return RD_ERROR_NULL;

        case FDS_ERR_NO_OPEN_RECORDS:
            return RD_ERROR_INVALID_STATE;

        case FDS_ERR_NO_SPACE_IN_FLASH:
            return RD_ERROR_NO_MEM;

        case FDS_ERR_NO_SPACE_IN_QUEUES:
            return RD_ERROR_BUSY;

        case FDS_ERR_RECORD_TOO_LARGE:
            return RD_ERROR_DATA_SIZE;

        case FDS_ERR_NOT_FOUND:
            return RD_ERROR_NOT_FOUND;

        case FDS_ERR_NO_PAGES:
            return RD_ERROR_NO_MEM;

        case FDS_ERR_USER_LIMIT_REACHED:
            return RD_ERROR_RESOURCES;

        case FDS_ERR_CRC_CHECK_FAILED:
            return RD_ERROR_SELFTEST;

        case FDS_ERR_BUSY:
            return RD_ERROR_BUSY;

        case FDS_ERR_INTERNAL:
        default:
            return RD_ERROR_INTERNAL;
    }

    return RD_ERROR_INTERNAL;
}


/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;
/* Flag to check fds processing status. */
static bool volatile m_fds_processing;
/* Flag to check fds callback registration. */
static bool m_fds_registered;
/** @brief Handle FDS events */
static void fds_evt_handler (fds_evt_t const * p_evt)
{
    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_evt->result == FDS_SUCCESS)
            {
                m_fds_initialized = true;
                ri_log (LOG_LEVEL, "FDS init\r\n");
            }

            break;

        case FDS_EVT_WRITE:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                ri_log (LOG_LEVEL, "Record written\r\n");
                m_fds_processing = false;
            }
        }
        break;

        case FDS_EVT_UPDATE:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                ri_log (LOG_LEVEL, "Record updated\r\n");
                m_fds_processing = false;
            }
        }
        break;

        case FDS_EVT_DEL_RECORD:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                ri_log (LOG_LEVEL, "Record deleted\r\n");
                m_fds_processing = false;
            }
        }
        break;

        case FDS_EVT_DEL_FILE:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                ri_log (LOG_LEVEL, "File deleted\r\n");
                m_fds_processing = false;
            }
        }
        break;

        case FDS_EVT_GC:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                ri_log (LOG_LEVEL, "Garbage collected\r\n");
                m_fds_processing = false;
            }
        }
        break;

        default:
            break;
    }
}

rd_status_t ri_flash_total_size_get (size_t * const size)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == size)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (false == m_fds_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        ri_flash_page_size_get (size);
        *size *= m_number_of_pages;
    }

    return err_code;
}

rd_status_t ri_flash_free_size_get (size_t * const size)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == size)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (false == m_fds_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        // Read filesystem status
        fds_stat_t stat = {0};
        ret_code_t rc = fds_stat (&stat);
        *size = stat.largest_contig * sizeof (uint32_t);
        err_code |= fds_to_ruuvi_error (rc);
    }

    return err_code;
}

rd_status_t ri_flash_page_size_get (size_t * size)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == size)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (false == m_fds_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        *size = FDS_VIRTUAL_PAGE_SIZE;
    }

    return err_code;
}

rd_status_t ri_flash_record_delete (const uint32_t page_id,
                                    const uint32_t record_id)
{
    rd_status_t err_code = RD_SUCCESS;

    if (false == m_fds_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        fds_record_desc_t desc = {0};
        fds_find_token_t  tok  = {0};
        ret_code_t rc = fds_record_find (page_id, record_id, &desc, &tok);

        if (FDS_SUCCESS == rc)
        {
            // If there is room in FDS queue, it will get executed right away and
            // processing flag is reset when record_delete exits.
            m_fds_processing = true;
            rc = fds_record_delete (&desc);

            // If operation was not queued, mark processing as false.
            if (FDS_SUCCESS != rc)
            {
                m_fds_processing = false;
            }
        }

        err_code |= fds_to_ruuvi_error (rc);
    }

    return err_code;
}

rd_status_t ri_flash_record_set (const uint32_t page_id,
                                 const uint32_t record_id, const size_t data_size, const void * const data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == data)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (false == m_fds_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (m_fds_processing)
    {
        err_code |= RD_ERROR_BUSY;
    }
    else
    {
        rd_status_t err_code = RD_SUCCESS;
        fds_record_desc_t desc = {0};
        fds_find_token_t  tok  = {0};
        /* A record structure. */
        fds_record_t const record =
        {
            .file_id           = page_id,
            .key               = record_id,
            .data.p_data       = data,
            /* The length of a record is always expressed in 4-byte units (words). */
            .data.length_words = (data_size + 3) / sizeof (uint32_t),
        };
        ret_code_t rc = fds_record_find (page_id, record_id, &desc, &tok);

        // If record was found
        if (FDS_SUCCESS == rc)
        {
            /* Start write */
            m_fds_processing = true;
            rc = fds_record_update (&desc, &record);
            err_code |= fds_to_ruuvi_error (rc);

            if (RD_SUCCESS != err_code)
            {
                m_fds_processing = false;
                return err_code;
            }
        }
        // If record was not found
        else
        {
            /* Start write */
            m_fds_processing = true;
            desc.record_id = record_id;
            rc = fds_record_write (&desc, &record);
            err_code |= fds_to_ruuvi_error (rc);

            if (RD_SUCCESS != err_code)
            {
                m_fds_processing = false;
                return err_code;
            }
        }
    }

    return err_code;
}

rd_status_t ri_flash_record_get (const uint32_t page_id,
                                 const uint32_t record_id, const size_t data_size, void * const data)
{
    rd_status_t err_code = RD_SUCCESS;
    ret_code_t rc = NRF_SUCCESS;

    if (NULL == data)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (false == m_fds_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (m_fds_processing)
    {
        err_code |= RD_ERROR_BUSY;
    }
    else
    {
        rd_status_t err_code = RD_SUCCESS;
        fds_record_desc_t desc = {0};
        fds_find_token_t  tok  = {0};
        rc = fds_record_find (page_id, record_id, &desc, &tok);
        err_code |= fds_to_ruuvi_error (rc);

        // If file was found
        if (FDS_SUCCESS == rc)
        {
            fds_flash_record_t record = {0};
            /* Open the record and read its contents. */
            rc = fds_record_open (&desc, &record);
            err_code |= fds_to_ruuvi_error (rc);

            // Check length
            if (record.p_header->length_words * 4 > data_size) { return RD_ERROR_DATA_SIZE; }

            /* Copy the data from flash into RAM. */
            memcpy (data, record.p_data, record.p_header->length_words * 4);
            /* Close the record when done reading. */
            rc = fds_record_close (&desc);
        }
    }

    return err_code | fds_to_ruuvi_error (rc);
}

rd_status_t ri_flash_gc_run (void)
{
    rd_status_t err_code = RD_SUCCESS;

    if (false == m_fds_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else if (m_fds_processing)
    {
        err_code |= RD_ERROR_BUSY;
    }
    else
    {
        m_fds_processing = true;
        ret_code_t rc = fds_gc();
        err_code |= fds_to_ruuvi_error (rc);
    }

    return err_code;
}

rd_status_t ri_flash_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ret_code_t rc = NRF_SUCCESS;

    if (m_fds_initialized)
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        /* Register first to receive an event when initialization is complete. */
        if (!m_fds_registered)
        {
            rc |= flash_bounds_set (FSTORAGE_SECTION_START, FSTORAGE_SECTION_END);
            (void) fds_register (fds_evt_handler);
            m_fds_registered = true;
        }

        rc = fds_init();
        err_code |= fds_to_ruuvi_error (rc);

        if (RD_SUCCESS == err_code)
        {
            // Wait for init ok
            while (!m_fds_initialized) {};

            // Read filesystem status
            fds_stat_t stat = {0};

            rc = fds_stat (&stat);

            m_number_of_pages = stat.pages_available;
        }
    }

    err_code |= fds_to_ruuvi_error (rc);
    return err_code;
}

rd_status_t ri_flash_uninit (void)
{
    rd_status_t err_code = RD_SUCCESS;
    m_fds_initialized = false;
    m_fds_processing = false;
    return err_code;
}

/** Erase FDS */
void ri_flash_purge (void)
{
    ret_code_t rc = NRF_SUCCESS;
#if   defined(NRF51)
    const int erase_unit = 1024;
#elif defined(NRF52_SERIES)
    const int erase_unit = 4096;
#endif
    const int total_pages = (FSTORAGE_SECTION_END - FSTORAGE_SECTION_START)
                            / erase_unit;

    for (int p = 0; (p < total_pages) && (NRF_SUCCESS == rc); p++)
    {
        int page = (FSTORAGE_SECTION_START / erase_unit) + p; // erase unit == virtual page size
        rc = sd_flash_page_erase (page);
        ri_delay_ms (200);
        RD_ERROR_CHECK (rc, RD_SUCCESS);
    }
}

bool ri_flash_is_busy()
{
    return m_fds_processing;
}

#endif
