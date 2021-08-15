/**
 *
 * @file ruuvi_nrf5_sdk15_aes.c
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @date 2002-08-15
 * @author Otso Jousimaa <otso@ojousima.net>
 * @brief Ruuvi AES interface implementation on nRF5 SDK15
 * 
 */

#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_aes.h"
#if RUUVI_NRF5_SDK15_AES_ENABLED

#include "ruuvi_driver_error.h"
#include "nrf_crypto_aes.h"
#include "nrf_sdh.h"
#include "nrf_soc.h"

#define ECB_128_BLOCK_SIZE_BYTES (16U)

static void aes_ecb_128_encrypt_sdapi(const uint8_t* const cleartext,
                                      uint8_t* const ciphertext,
                                      const uint8_t* const key,
                                      const size_t data_length)
{
      const size_t crypt_rounds = (data_length / ECB_128_BLOCK_SIZE_BYTES);
      nrf_ecb_hal_data_t soc_ecb_data;
      memcpy(soc_ecb_data.key, key, ECB_128_BLOCK_SIZE_BYTES);
      for (size_t round = 0; round < crypt_rounds; round++)
      {
          memcpy(soc_ecb_data.cleartext,
                 cleartext + (round * ECB_128_BLOCK_SIZE_BYTES),
                 ECB_128_BLOCK_SIZE_BYTES);
          sd_ecb_block_encrypt(&soc_ecb_data);
          memcpy(cleartext + (round * ECB_128_BLOCK_SIZE_BYTES),
                 soc_ecb_data.ciphertext,
                 ECB_128_BLOCK_SIZE_BYTES);
      }
}

static rd_status_t 
aes_ecb_128_encrypt_nrfapi(const uint8_t* const cleartext,
                           uint8_t* const ciphertext,
                           const uint8_t* const key,
                           const size_t data_length)
{
  /*
ret_code_t nrf_crypto_aes_crypt(nrf_crypto_aes_context_t * const    p_context,
                                nrf_crypto_aes_info_t const * const p_info,
                                nrf_crypto_operation_t              operation,
                                uint8_t *                           p_key,
                                uint8_t *                           p_iv,
                                uint8_t *                           p_data_in,
                                size_t                              data_size,
                                uint8_t *                           p_data_out,
                                size_t *                            p_data_out_size)
                                */
      
ret_code_t err_code = NRF_SUCCESS;
  nrf_crypto_aes_info_t info = 
  {
    .mode = NRF_CRYPTO_AES_MODE_ECB,
    .key_size = NRF_CRYPTO_KEY_SIZE_128
  };
  err_code |= nrf_crypto_aes_crypt(NULL,
                                   &info,
                                   NRF_CRYPTO_ENCRYPT,
                                   key,
                                   NULL,
                                   cleartext,
                                   data_length,
                                   ciphertext,
                                   data_length);
}

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
rd_status_t ri_aes_ecb_128_encrypt (const uint8_t* const cleartext,
                                    uint8_t* const ciphertext,
                                    const uint8_t* const key,
                                    const size_t data_length)
{
  rd_status_t err_code = RD_SUCCESS;
  if((NULL == cleartext) || (NULL == ciphertext) || (NULL == key))
  {
      err_code |= RD_ERROR_NULL;
  }
  else if ((!data_length) || (data_length % ECB_128_BLOCK_SIZE_BYTES))
  {
    err_code |= RD_ERROR_INVALID_LENGTH;
  }
  // Softdevice enabled, use SD API
  else if (nrf_sdh_is_enabled())
  {
      aes_ecb_128_encrypt_sdapi(cleartext, ciphertext, key, data_length);
  }
  // Softdevice not enabled, use nRF Crypto API
  else
  {

    err_code |= aes_ecb_128_encrypt_nrfapi(cleartext, ciphertext, key, data_length);
  }
  return err_code;
}

#endif