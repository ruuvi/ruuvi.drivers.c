# ruuvi.drivers.c
[![Build Status](https://travis-ci.org/ruuvi/ruuvi.drivers.c.svg?branch=master)](https://travis-ci.org/ruuvi/ruuvi.drivers.c)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=alert_status)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=bugs)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=code_smells)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=coverage)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=duplicated_lines_density)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=ncloc)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=sqale_rating)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=sqale_index)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)

Ruuvi embedded drivers used across various platforms. Generally you should not use this 
repository as-is, but rather as a submodule included in your project.
Repository is under active development (alpha), expect breaking changes.

# Structure
## Folders
The drivers project has other drivers - suchs as Bosch and STM official drivers - as 
submodules at root level.

Ruuvi code is under `src` folder. 

## Interfaces
`Interfaces` folder provides platform-independent access to the peripherals of the 
underlying microcontroller, for example function 
`int8_t spi_bosch_platform_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);` 
is defined, but not implemented. This eases mocking the tasks for unit tests. 

## Platform implementations
Implementation of interfaces is in `*_platform`-folders.

Currently only supported platform is Nordic SDK15.3. Unpack (or softlink) the SDK on the
root of the project for running the tests. 

Because the platforms must contain hardware dependencies, the interfaces and platforms
which implement them are not unit tested but integration tested instead.  

## Tasks
Tasks are larger functionalities, such as "get battery voltage" or "Broadcast this data".
Top keep the tasks unit testable, they must contain only references to interfaces which
allows mocking the hardware dependencies. 

## File and variable naming
Files should be named `ruuvi_module_name`, for example `ruuvi_interface_spi.h`
Globally visible functions, variables and definitions should be likewise named with
abbreviation of `ruuvi_type_module_name`, for example `ri_yield_init()` or
`rt_adc_vdd_get()`

# Usage
## Enabling modules
The application should contain `app_config.h` which includes platform specific includes
such as `nrf5_sdk15_app_config.h`. Preprocessor must define `APPLICATION_DRIVER_CONFIGURED`
to let drivers know they can include the `app_config.h` and `RUUVI_NRF5_SDK15_ENABLED` 
for including the `nrf5_sdk15_app_config.h`.

As the repository may contain several different implementations of interface functions the
desired implementation is enabled by definining `PLATFORM_MODULE_ENABLED 1`, 
for example `RUUVI_NRF5_SDK15_LOG_ENABLED 1`.

Leaving unused modules unenabled will conserve some flash and RAM as they're not 
linked into given application. 

## Error codes
There are common error code definitions for the drivers, please see `ruuvi_driver_error.h`
 for details.

## Sensor interface
Sensors have a common interface which has setter and getter functions for common 
parameters such as scale, resolution, sample rate and dsp function.
Initialization function takes a pointer to sensor interface type, as well as used bus 
(I2C or SPI) and a handle which helps firmware to select the desired on board.
On SPI the handle is GPIO pin of Chip Select, on I2C the handle is I2C address of the 
sensor.

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
 * Sensor configuration is not defined before init and after uninit
 * Sensor initialization must be successful after uninit.
 * Init and Uninit must return RUUVI_DRIVER_ERROR_NULL if pointer to the sensor struct is NULL.

## Building documentation
Run `doxygen`

## Formatting code
Run `astyle --project=.astylerc ./target_file`. To format the entire project,
```
astyle --project=.astylerc --recursive "./interfaces/*.c"
astyle --project=.astylerc --recursive "./interfaces/*.h"
astyle --project=.astylerc --recursive "./nrf5_sdk15_platform/*.c"
astyle --project=.astylerc --recursive "./nrf5_sdk15_platform/*.h"
```

# Testing
## Unit tests
Unit tests are run by Ceedling and they are completely hardware-independent. 
They can be run by e.g. Travis. The unit tests are in `test` folder.

## Integration tests
Integration tests are run on actual hardware and cannot be run on the cloud.
Integration tests are in `src/integration_tests` folder.

## System and acceptance testing. 
System testing and acceptance testing are in the scope of main application and 
not handled here.

# Progress
The repository is under active development and major refactors are to be expected.
You can follow more detailed development blog at [Ruuvi Blog](https://blog.ruuvi.com).
Currently the drivers are being refactored for more consistent naming, better test 
coverage and Doxygen support.

## Upcoming refactors
 - ~~Add unit to sensor configuration fields, e.g. `resolution`-> `resolution_bits`~~
 - ~~Rewrite BLE advertising module~~
 - Add default configuration value to header of each sensor
 - ~~Add a function to return sensor name to each sensor~~
 - ~~Add dummy initialization for sensor struct.~~

## Upcoming implementations
 - ~~Support more ADC inputs,~~ differential ADC inputs.
 - 


# Licenses
All Ruuvi code is BSD-3 licensed.
Drivers from other sources (as git submodules) have their own licenses, please check the 
specific submodule for details. Platforms have license of the platform providers, 
for example Nordic semiconductor platform files follow Nordic SDK License.

# How to contribute
All contributions are welcome, from typographical fixes to feedback on design and naming schemes.
If you're a first time contributor, please leave a note saying that BSD-3 licensing is ok for you.

# Changelog

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