# ruuvi.drivers.c v3.12.0

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=alert_status)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=bugs)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=code_smells)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=coverage)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=ruuvi_ruuvi.drivers.c&metric=sqale_rating)](https://sonarcloud.io/dashboard?id=ruuvi_ruuvi.drivers.c)

Ruuvi embedded drivers used across various platforms. This repository is designed to be
included as a submodule in your project rather than used standalone.

Version 3.4.0 and later are considered stable. No breaking changes will be introduced
to interfaces or tasks without a major version bump. Non-tested functionality and
failing tests are subject to change.

## Structure

### Folders

The drivers project includes third-party drivers (Bosch, STMicroelectronics, etc.)
as submodules at the root level. Ruuvi code is under the `src` folder.

### Interfaces

The `interfaces` folder provides platform-independent access to microcontroller
peripherals. Functions are defined but not implemented, enabling easy mocking for
unit tests. For example:

```c
int8_t spi_bosch_platform_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len);
```

### Platform Implementations

Interface implementations are in `*_platform` folders.

Currently the only supported platform is Nordic SDK 15.3. Unpack or symlink the SDK
at the project root to run tests.

Since platforms contain hardware dependencies, interfaces and their implementations
are integration tested rather than unit tested.

### Tasks

Tasks provide higher-level functionality such as "get battery voltage" or "broadcast
this data". To remain unit testable, tasks reference only interfaces, allowing
hardware dependencies to be mocked.

### Naming Conventions

- **Files**: `ruuvi_module_name` (e.g., `ruuvi_interface_spi.h`)
- **Functions/Variables**: `ruuvi_type_module_name` abbreviated (e.g., `ri_yield_init()`, `rt_adc_vdd_get()`)

## Usage

### Enabling Modules

Your application should contain `app_config.h` which includes platform-specific
headers like `nrf5_sdk15_app_config.h`.

Required preprocessor definitions:
- `APPLICATION_DRIVER_CONFIGURED` — allows drivers to include `app_config.h`
- `RUUVI_NRF5_SDK15_ENABLED` — includes `nrf5_sdk15_app_config.h`

Enable specific implementations with `PLATFORM_MODULE_ENABLED 1`
(e.g., `RUUVI_NRF5_SDK15_LOG_ENABLED 1`).

Leaving unused modules disabled conserves flash and RAM.

### Error Codes

See `ruuvi_driver_error.h` for common error code definitions.

### Sensor Interface

Sensors share a common interface with setters and getters for:
- Scale
- Resolution
- Sample rate
- DSP function

The initialization function takes:
- Pointer to sensor interface type
- Bus type (I2C or SPI)
- Handle (SPI: chip select GPIO pin; I2C: device address)

#### Initializing a Sensor

Call `ri_sensor_init(rd_sensor_t*, rd_bus_t, uint8_t)` before using any sensor.

**Parameters:**
- `rd_sensor_t*` — pointer to sensor struct (populated with function pointers)
- `rd_bus_t` — bus type: I2C, SPI, or NONE (for MCU internal peripherals)
- `uint8_t` — handle: I2C address, SPI chip select GPIO, or ADC channel

**Behavior requirements:**
- First init must return `RD_SUCCESS`
- No function pointers may be NULL after init
- Re-initializing returns `RD_ERROR_INVALID_STATE`
- First uninit must return `RD_SUCCESS`
- All function pointers must be NULL after uninit
- Sensor enters lowest-power state after init and uninit
- Configuration is undefined before init and after uninit
- Init must succeed after uninit
- NULL sensor pointer returns `RD_ERROR_NULL`

### Building Documentation

```bash
doxygen
```

### Formatting Code

Format a single file:
```bash
astyle --project=.astylerc ./target_file
```

Format the entire project:
```bash
make astyle
```

## Testing

### Unit Tests

Unit tests run via Ceedling and are hardware-independent. Located in the `test` folder.

```bash
ceedling test:all
```

### Integration Tests

Integration tests run on actual hardware and are located in `src/integration_tests`.
The main application is responsible for executing these tests.

### System and Acceptance Testing

System and acceptance testing are handled by the main application.

## Licenses

- **Ruuvi code**: BSD-3
- **Submodules**: Check individual submodule licenses
- **Platform files**: Follow platform provider licenses (e.g., Nordic SDK License)

## Contributing

All contributions are welcome, from typo fixes to design feedback.
First-time contributors: please confirm BSD-3 licensing is acceptable.

## Changelog

See [CHANGELOG.md](CHANGELOG.md)