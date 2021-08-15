#ifdef TEST

#include "unity.h"

#include "ruuvi_interface_aes.h"

void setUp(void)
{
}

void tearDown(void)
{
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
void test_ri_aes_ecb_128_encrypt_ok (void)
{

rd_status_t ri_aes_ecb_128_encrypt (const uint8_t* const cleartext,
                                    uint8_t* const ciphertext,
                                    const uint8_t* const key,
                                    const size_t data_length)
  

}
#endif // TEST
