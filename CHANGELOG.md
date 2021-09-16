# Changelog

## 3.9.2
 - Fix GATT timer-related errors

## 3.9.1
 - Add configurable delay and retries to gatt param changes

## 3.9.0
 - Put SHTCX to sleep after reads
 - Poll data ready before reading TMP117 in single-shot mode
 - Add delay to TMP117 after soft reset
 - Add sink-only GPIO CFG options for I2C
 - Add RD_WARNING_DEPRECATED error code
 - Add AES-ECB-128 encryption function
 - Add GATT parameter renegotiation

## 3.8.0
 - Store boot counter in flash
 - Update SHTC driver to  5.3.0
 - Align memory access to 4-byte boundary in flash test

## 3.7.0 
 - Add TMP117 temperature sensor support

## 3.6.0
 - Add NRF52 temperature sensor support

## 3.5.0
 - Add partial DPS310 support (SPI only, no FIFO, no interrupts)

## 3.4.5
 - Fix GPIO port mapping on NRF5 SDK 15 when port > 0.
 - Fix Flash erase not clearing out everything on nRF5 SDK15.
 - Add +3 RH offset to BME280 to fix offset observed in devices.

## 3.4.4
 - Fix UART driver locking if TX interrupt is missed

## 3.4.3
 - Update NRF5 SDK15 overrides to include NFCT.c from SDK 17.0.2

## 3.4.2
 - Increase DIS string limit to 48 to fit build metadata, such as "Ruuvi FW 3.29.0-RC8+default"
 - Fix Bootloader initialization not exiting critical region on error. 

## 3.4.1
 - Optimize GATT throughput/energy consumption by using longer interval on connection
   + slave latency. Before: ~25 kBps between nRF52 devkits on 1 MBit / s and 2 MBit / s.
   After: ~20 kBps at 1 Mbit/s, 30 kBps at 2 Mbit / s. 
 - Use updated SDK overrides
    * Fixes crash when interrupt triggers while enabling bootloader service.
    * Update NFC binaries to SDK 17.2 version

## 3.4.0
 - Fix GPIO on ports > 0 on NRF SDK. 
 - Fix flash record delete on nRF5 SDK15 marking flash driver as busy after operation has already been executed.
 - Fix NFC data field language header, "da" -> "dt"

## 0.3.3 
 - Fix flash record delete on nRF5 SDK15 not marking flash driver as busy.

## 0.3.2
 - Add Generic Discoverable flag to advertisements. 
 - Do not configure a secondary PHY for 2 MBIT/s advertisement if data fits in primary adv.
 - Add GATT uninit task. 

## 0.3.1
 - Add explicit reserved bits to rd_sensor_data_bitfield_t to be zeroed in initialization.

## 0.3.0
 - Purge flash before flash integration test.
 - Require fixed flash area in nRF5_SDK15 implementation. 
 - Disable SPI MISO pull-up by default.
 - Fix UART read retriggering data received event.
 - Add yield uninit
 - Add helper functions for parsing sensor data.

## 0.2.6
 - Fix ADC task on multiple channel reads.

## 0.2.5
 - Return RD_ERROR_NOT_ENABLED from tasks not enabled.

## 0.2.4
 - Fix missing enable macro
 - Fix queue uninit
 - Add shorthands for accessing data fields in parse functions

## 0.2.3 
 - Add relative ADC, NTC, photodiode support.

## 0.2.2
 - Add PWM support.

## 0.2.1
 - Add I2C, SPI, LIS2DH12, SHTCX support.

## 0.2.0
 - Non-compatible changes to BLE interface and implementation.

## 0.1.6
 - Integration test DC/DC.
 - Integration test timer.
 - Integration test scheduler.
 - Unit test NFC task. Integration test NFC
 - NRF SDK 15: Use SD reset function if SD is enabled in power interface reset.

## 0.1.5
 - Add nrf15_sdk log enable macro to ruuvi_interface_log.h.
 - Fix nRF5 SDK15 watchdog reinitialization assert.
 - Unit test task_flash, integration test ri_flash.
 - Unit test and integration test GPIO.

## 0.1.4
 - Fix button task compilation when button task is not enabled.
 - Remove RTC, Power tasks as they unnecessarily wrap interface.
 - Add watchdog.

## 0.1.3
 - Add unit tests for tasks.
 - Support multiple button initialization.

## 0.1.2
 - Fix some globally visible names not following the refactored scheme.

## 0.1.1
 - Fix some globally visible names not following the refactored scheme.
 - Pass RI_COMMUNICATION_TIMEOUT to application from BLE Scan.

## 0.1.0 
 - Change to 0.x Semver to signal that project is in alpha.

## 3.3.0
 - Add semantic versioning string.

## 3.0.0 ... 3.2.0 
Alpha versions, do not use for anything.