#ifndef RUUVI_INTERFACE_AES_H
#define RUUVI_INTERFACE_AES_H

/**
 * @defgroup Crypto Crypto functions
 * @brief Functions for cryptography operations.
 *
 * These functions are in drivers because they are supposed to utilize hardware implementation in underlying ICs. A software fallback might be implemented later.
 *
 */
/** @{ */
/**
 * @file ruuvi_interface_aes.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2002-08-15
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Interface for AES encryption and decryption operations.
 *
 */

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_enabled_modules.h"

#if RI_AES_ENABLED
#  define RUUVI_NRF5_SDK15_AES_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

/**
 *  @brief encrypt a block with AES ECB 128 encryption
 *
 * This call takes 16-byte divisible data as input and encrypts it with AES Electronic Codebook.
 *
 * @param[in] cleartext Data to encrypt. Must have 16-byte divisible length.
 * @param[out] ciphertext Encryped data output
 * @param[in] key Key to encrypt data with. Must be exactly 16 bytes.
 * @param[in] data_length Length of data. Input must equal output, must be 16 bytes divisible.
 * @retval RD_SUCCESS data was encrypted.
 * @retval RD_ERROR_NULL if any pointer is NULL
 * @retval RD_ERROR_INVALID_LENGTH if data length is not a multiple of 16
 *
 */
rd_status_t ri_aes_ecb_128_encrypt (const uint8_t * const cleartext,
                                    uint8_t * const ciphertext,
                                    const uint8_t * const key,
                                    const size_t data_length);

/** @} */
#endif // RUUVI_INTERFACE_AES_H
