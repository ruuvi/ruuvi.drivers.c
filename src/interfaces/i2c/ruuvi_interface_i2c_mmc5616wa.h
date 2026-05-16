#ifndef RUUVI_INTERFACE_I2C_MMC5616WA_H
#define RUUVI_INTERFACE_I2C_MMC5616WA_H

#include <stdint.h>

int32_t ri_i2c_mmc5616wa_write (void * handle, uint8_t reg_addr,
                                const uint8_t * reg_data, uint16_t len);
int32_t ri_i2c_mmc5616wa_read (void * handle, uint8_t reg_addr,
                               uint8_t * reg_data, uint16_t len);

#endif
