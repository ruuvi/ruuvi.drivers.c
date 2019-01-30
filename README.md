# ruuvi.drivers.c {#mainpage}
Ruuvi embedded drivers used across various platforms. Generally you should not use this repository as-is, but rather as a submodule included in your project.
Repository is under active development (alpha), expect breaking changes.

# Structure
## Folders
The drivers project has other drivers - suchs as Bosch and STM official drivers - as submodules at root level.

Interfaces folder provides platform-independent access to the peripherals of the underlying microcontroller, for example function `int8_t spi_bosch_platform_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);` is defined, but not implemented.

Implementation is in `*_platform`-folders.

External platform-independent requirements are in `ruuvi_drivers_external_includes.h` -file. Platform specific external requirements are in `ruuvi_platform_external_includes.h` file.

## File and variable naming
Files should be named `ruuvi_module_name`, for example `ruuvi_interface_spi.h`
Globally visible functions, variables and definitions should be likewise named `ruuvi_module_file_name`, for example  `ruuvi_interface_yield_init()`

# Usage
## Enabling modules
Se the repository may contain several different implementations of interface functions the desired implementation is enabled by definining
`PLATFORM_MODULE_ENABLED 1`, for example `NRF5_SDK15_LOG_ENABLED 1`.

## Error codes
There are common error code definitions for the drivers, please see `ruuvi_driver_error.h` for details.

## Sensor interface
Sensors have a common interface which has setter and getter functions for common parameters such as scale, resolution, sample rate and dsp function.
Initialization function takes a pointer to sensor interface type, as well as used bus (I2C or SPI) and a handle which helps firmware to select the desired on board.
On SPI the handle is GPIO pin of Chip Select, on I2C the handle is I2C address of the sensor.

### Initializing sensor
All sensors have `ruuvi_interface_sensor_init(ruuvi_driver_sensor_t*, ruuvi_driver_bus_t, uint8_t)` -function which should be called before usage. Check the interface definition of detailed explanation of any initialization parameters.
 * ruuvi_driver_sensor_t* is a pointer to sensor struct which will get initialized with proper function pointers
 * bus is the bus being used for sensor, such as I2C, SPI or NONE for MCU internal peripherals
 * uint8_t is a handle for the sensor. For I2C it's the device address, for SPI it's GPIO which controls the peripheral sensor and for NONE it could be ADC channel.
 * Sensor must return RUUVI_DRIVER_SUCCESS on first init.
 * None of the sensor function pointers may be NULL after init
 * Sensor should return RUUVI_DRIVER_ERROR_INVALID_STATE when initializing sensor which is already init. May return other error if check for it triggers first.
 * Sensor must return RUUVI_DRIVER_SUCCESS on first uninit
 * All of sensor function pointers must be NULL after uninit
 * Sensor must be in lowest-power state possible after init
 * Sensor must be in lowest-power state available after uninit.
 * Sensor configuration is not defined after init and uninit
 * Sensor initialization must be successful after uninit.
 * Init and Uninit should return RUUVI_DRIVER_ERROR_NULL if pointer to the sensor struct is NULL. May return other error if check for it triggers first.

# Progress
The repository is under active development and major refactors are to be expected.
You can follow more detailed development blog at [Ruuvi Blog](https://blog.ruuvi.com). Repository will have tagged releases such as 3.1.0 to keep track of proggress in the roadmap.

Currently the drivers are being refactored for more consistent naming, better test coverage and Doxygen support

# Licenses
All Ruuvi code is BSD-3 licensed.
Drivers from other sources (as git submodules) have their own licenses, please check the specific submodule for details.
Platforms have license of the platform providers, for example Nordic semiconductor platform files follow Nordic SDK License.

# How to contribute
All contributions are welcome, from typographical fixes to feedack on design and naming schemes.
If you're a first time contributor, please leave a note saying that BSD-3 licensing is ok for you.

# Changelog
## 3.19.0
 * Doxygen support started.
 * Rename yield and delay interface function to -interface, was -platform

## 3.18.0
 * Add Flash storage

## 3.17.0
 * Add NFC

## 3.16.0
 * Add BLE GATT connection

## 3.15.0
 * Add watchdog

## 3.14.0
 * Add FIFO and threshold interrupt support to LIS2DH12 
 * Fix bugs found by unit tests

## 3.13.0
 * Skip 3.12
 * Add callbacks to radio activity

## 3.11.0
 * Add Timer and scheduler
 * Fix bug in BME280 sample rate setter

## 3.10.0
  * Add NFC read functionality

## 3.9.0
  * Add BLE advertising functionality

## 3.8.0
 * Add RTC support and timestamping
 * Fix issues in sensor interface implementations

## 3.7.0
Add battery voltage measurement with nRF52 ADC

## 3.6.0
ADD LIS2DH12 support

## 3.5.0
Add SPI driver, BME280 support, nRF52 temperature sensing support

## 3.4.0
Add interrupts to GPIO

## 3.2.0
Adds logging and error code handling

## 3.1.1
 Clear up files not presented in Ruuvi Blog at the time of writing. Add GPIO and yield.