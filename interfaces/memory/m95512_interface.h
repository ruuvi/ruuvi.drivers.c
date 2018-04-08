#ifndef M95512_H
#define M95512_H
#include <stdlib.h>

#include "ruuvi_error.h"
#include "memory.h"

typedef struct 
{
  uint8_t handle; //Slave select pin or I2C address
  transfer_blocking_fp transfer_blocking;
}m95512_interface_t;


ruuvi_status_t m95512_init(void);

ruuvi_status_t m95512_selftest(void);

ruuvi_status_t m95512_write_register_blocking(const uint8_t address, uint8_t* const dataToWrite);

ruuvi_status_t m95512_read_register_blocking(const uint8_t address, uint8_t* const p_toRead);

ruuvi_status_t m95512_write_data_blocking(const uint8_t address, uint8_t* const dataToWrite);

ruuvi_status_t m95512_read_data_blocking(const uint8_t address, uint8_t* const p_toRead);

/** Asynchronous ops not supported, as this would require support in SPI/I2C drivers.  **/
// ruuvi_status_t m95512_write_register_asynchronous(const uint8_t address, uint8_t* const dataToWrite);
// ruuvi_status_t m95512_read_register_asynchronous(const uint8_t address, uint8_t* const p_toRead);
// ruuvi_status_t m95512_write_data_asynchronous(const uint8_t address, uint8_t* const dataToWrite);
// ruuvi_status_t m95512_read_data_asynchronous(const uint8_t address, uint8_t* const p_toRead);

#endif