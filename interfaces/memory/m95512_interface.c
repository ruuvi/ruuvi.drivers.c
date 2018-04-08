
#include "boards.h"

#if M95512_FLASH
#include "m95512.h"
#include "ruuvi_error.h"
#include "memory.h"

#include "spi.h"

static m95512_interface_t iface;


ruuvi_status_t m95512_init(void)
{
  ret_code_t err_code = RUUVI_SUCCESS;
  iface.transfer_blocking = spi_generic_platform_xfer_blocking;
  iface.handle = SPIM0_SS_MEMORY_PIN; 
    
  err_code |= m95512_selftest();

  return err_code;
}

ret_code_t m95512_selftest(void)
{
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  uint8_t value[2] = {0xAA, 0x55}; //Dummy values
  uint8_t** rx_addr = &value;
  size_t rx_len = sizeof(value);
  //Read STATUS
  m95512_read_register_blocking(iface, 0x05, rx_addr, &rx_len);
  //Expect write not enabled, no write protection, no block, no write in progress
  if(0!=(*rx_addr)[0])
  {
    err_code = RUUVI_INVALID_STATE;
    PLATFORM_LOG_ERROR("Unexpected value from status: %x\r\n", value[0]);
  }

  return err_code;
}

/**
 * Read registers
 *
 * Read one or more registers from the sensor
 *
 */
ret_code_t m95512_read_register_blocking(const uint8_t address, uint8_t** p_toRead, size_t* count)
{
  if(NULL == p_toRead) { return RUUVI_ERROR_NULL; }
  ruuvi_status_t err_code = RUUVI_SUCCESS;
  uint8_t tx[] = {address};
  size_t tx_len = 1;
  // Transfer address of register. Increment read buffer by one to skip empty first byte.
  err_code = iface->transfer_blocking(iface->handle, tx, tx_len, p_toRead, count, true);
  return err_code;
}

/**
 * Write a register
 */
ret_code_t m95512_write_register_blocking(uint8_t address, uint8_t* const dataToWrite, size_t count)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t m95512_write_data_blocking(const uint8_t address, uint8_t* const dataToWrite)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}

ruuvi_status_t m95512_read_data_blocking(const uint8_t address, uint8_t* const p_toRead)
{
  return RUUVI_ERROR_NOT_IMPLEMENTED;
}
#endif