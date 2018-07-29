# ruuvi.drivers.c
Ruuvi embedded drivers used across various platforms. Generally you should not use this repository as-is, but rather as a submodule included in your project.
Repository is under active development, expect breaking changes.

# Structure
## Folders
The drivers project has other drivers - suchs as Bosch and STM official drivers - as submodules at root level. 

Interfaces folder provides platform-independent access to the peripherals of the underlying microcontroller, for example function `int8_t spi_bosch_platform_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);` is defined, but not implemented.

Implementation is in `*_platform`-folders.

External platform-independent requirements are in `ruuvi_drivers_external_includes.h` -file. Platform specific external requirements are in `ruuvi_platform_external_includes.h` file.

## File and variable naming
Files should be named `ruuvi_module_name`, for example `ruuvi_interface_spi.h`
Globally visible functions, variables and definitions should be likewise named `ruuvi_module_file_name`, for example  `ruuvi_platform_yield_init()`

# Progress
The repository is under active development and major refactors are to be expected. The roadmap for releases is: 

```
0: Introduction
1: Sleep / Yield
2: Logging
3: Led blinking
4: Button - Interrupt
5: Environmental sensing / BME280
6: Accelerometer - polling / LIS2DH12
7: Battery measurement - naive approach / ADC
8: RTC
9: BLE broadcasting
10: NFC reading
11: Scheduler
12: Bootloader
13: Battery measurement - synchronize to radio
14: Accelerometer - interrupt
15: BLE  GATT connection
16: NFC writing
```

You can follow more detailed development blog at [Ruuvi Blog](https://blog.ruuvi.com). Repository will have tagged releases such as 3.1.0 to keep track of proggress in the roadmap.

# Licenses
All Ruuvi code is BSD-3 licensed.
Drivers from other sources (as git submodules) have their own licenses, please check the specific submodule for details.
Platforms have license of the platform providers, for example Nordic semiconductor platform files follow Nordic SDK License. 

# How to contribute
All contributions are welcome, from typographical fixes to feedack on design and naming schemes.
If you're a first time contributor, please leave a note saying that BSD-3 licensing is ok for you.

# Changelog
## 3.2.0
Adds logging and error code handling

## 3.1.1
 Clear up files not presented in Ruuvi Blog at the time of writing. Add GPIO and yield. 