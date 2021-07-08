/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ruuvi_driver_enabled_modules.h"
#if RI_SHTCX_ENABLED || DOXYGEN
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_yield.h"
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"
#include "shtc1.h"
#include <string.h>


/*
 * INSTRUCTIONS
 * ============
 *
 * Implement all functions where they are marked as IMPLEMENT.
 * Follow the function specification in the comments.
 */

/**
 * Select the current i2c bus by index.
 * All following i2c operations will be directed at that bus.
 *
 * THE IMPLEMENTATION IS OPTIONAL ON SINGLE-BUS SETUPS (all sensors on the same
 * bus)
 *
 * @param bus_idx   Bus index to select
 * @returns         0 on success, an error code otherwise
 */
int16_t sensirion_i2c_select_bus (uint8_t bus_idx)
{
    // IMPLEMENT or leave empty if all sensors are located on one single bus
    RD_ERROR_CHECK (RD_ERROR_NOT_SUPPORTED, ~RD_ERROR_FATAL);
    return 0;
}

/**
 * Driver file does not know about the board configuration, use ri_i2c_init instead.
 */
void sensirion_i2c_init (void)
{
    RD_ERROR_CHECK (RD_ERROR_NOT_SUPPORTED, ~RD_ERROR_FATAL);
}

/**
 * Release all resources initialized by sensirion_i2c_init().
 */
void sensirion_i2c_release (void)
{
    // IMPLEMENT or leave empty if no resources need to be freed
    RD_ERROR_CHECK (RD_ERROR_NOT_SUPPORTED, ~RD_ERROR_FATAL);
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_read (uint8_t address, uint8_t * data, uint16_t count)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= ri_i2c_read_blocking (address, data, count);
    return (RD_SUCCESS == err_code) ? 0 : -STATUS_ERR_BAD_DATA;
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_write (uint8_t address, const uint8_t * data,
                            uint16_t count)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t deepcpy[SENSIRION_COMMAND_SIZE];
    memcpy (deepcpy, data, SENSIRION_COMMAND_SIZE);
    err_code |= ri_i2c_write_blocking (address, deepcpy, 2, true);
    return (RD_SUCCESS == err_code) ? 0 : STATUS_ERR_BAD_DATA;
}

#endif
