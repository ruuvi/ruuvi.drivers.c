#include "ruuvi_driver_enabled_modules.h"
#if RI_BME280_ENABLED || DOXYGEN
// Ruuvi headers
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_bme280.h"
#include "ruuvi_interface_environmental.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_spi.h"
#include "ruuvi_interface_spi_bme280.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_i2c_bme280.h"
#include "ruuvi_interface_yield.h"

#include <string.h>

// Bosch driver.
#include "bme280.h"
#include "bme280_defs.h"
#include "bme280_selftest.h"
#if !(BME280_FLOAT_ENABLE || DOXYGEN)
#error "Please #define BME280_FLOAT_ENABLE in makefile CFLAGS"
#endif

/**
 * @addtogroup BME280
 */
/*@{*/
/**
 * @file ruuvi_interface_bme280.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-04-28
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Requires Bosch BME280_driver, available under BSD-3 on GitHub.
 * Will only get compiled if RI_BME280_ENABLED is defined as true
 * Requires BME280_FLOAT_ENABLE defined in makefile or otherwise passed to preprocessor
 *
 */

/** @brief Macro for checking "ignored" parameters NO_CHANGE, MIN, MAX, DEFAULT */
#define RETURN_SUCCESS_ON_VALID(param) do {\
            if(RD_SENSOR_CFG_DEFAULT   == param ||\
               RD_SENSOR_CFG_MIN       == param ||\
               RD_SENSOR_CFG_MAX       == param ||\
               RD_SENSOR_CFG_NO_CHANGE == param   \
             ) return RD_SUCCESS;\
           } while(0)

/** @brief Macro for checking that sensor is in sleep mode before configuration */
#define VERIFY_SENSOR_SLEEPS() do { \
          uint8_t MACRO_MODE = 0; \
          ri_bme280_mode_get(&MACRO_MODE); \
          if(RD_SENSOR_CFG_SLEEP != MACRO_MODE) { return RD_ERROR_INVALID_STATE; } \
          } while(0)


/** State variables **/
static struct bme280_dev dev = {0};
static uint64_t tsample;
static const char m_sensor_name[] = "BME280";

/**
 * Convert error from BME280 driver to appropriate NRF ERROR
 */
static rd_status_t BME_TO_RUUVI_ERROR (int8_t rslt)
{
    rd_status_t err_code = RD_SUCCESS;

    if (BME280_E_DEV_NOT_FOUND == rslt)  { err_code = RD_ERROR_NOT_FOUND; }
    else if (BME280_E_NULL_PTR == rslt)  { err_code = RD_ERROR_NULL; }
    else if (BME280_E_COMM_FAIL == rslt) { err_code = RD_ERROR_BUSY; }
    else if (BME280_OK == rslt) { return RD_SUCCESS; }

    return err_code;
}

void bosch_delay_ms (uint32_t time_ms)
{
    ri_delay_ms (time_ms);
}

// BME280 datasheet Appendix B.
static uint32_t bme280_max_meas_time (uint8_t oversampling)
{
    // Time
    float time = 1.25F + \
                 2.3F * 3.0F * oversampling + \
                 2.0F * 0.575F;
    // Roundoff + margin
    return (uint32_t) (2U + (uint32_t) time);
}

/** Initialize BME280 into low-power mode **/
rd_status_t ri_bme280_init (rd_sensor_t *
                            environmental_sensor, rd_bus_t bus, uint8_t handle)
{
    if (NULL == environmental_sensor) { return RD_ERROR_NULL; }

    // dev is NULL at boot, if function pointers have been set sensor is initialized
    if (NULL != dev.write) { return RD_ERROR_INVALID_STATE; }

    rd_sensor_initialize (environmental_sensor);
    environmental_sensor->name = m_sensor_name;
    rd_status_t err_code = RD_SUCCESS;

    switch (bus)
    {
#if RI_BME280_SPI_ENABLED

        case RD_BUS_SPI:
            dev.dev_id = handle;
            dev.intf = BME280_SPI_INTF;
            dev.read = &ri_spi_bme280_read;
            dev.write = &ri_spi_bme280_write;
            dev.delay_ms = bosch_delay_ms;
            err_code |= BME_TO_RUUVI_ERROR (bme280_init (&dev));

            if (err_code != RD_SUCCESS) { return err_code; }

            break;
#endif
#if RI_BME280_I2C_ENABLED

        case RD_BUS_I2C:
            dev.dev_id = handle;
            dev.intf = BME280_I2C_INTF;
            dev.read = &ri_i2c_bme280_read;
            dev.write = &ri_i2c_bme280_write;
            dev.delay_ms = bosch_delay_ms;
            err_code |= BME_TO_RUUVI_ERROR (bme280_init (&dev));

            if (err_code != RD_SUCCESS) { return err_code; }

            break;
#endif

        case RD_BUS_NONE:
        default:
            return  RD_ERROR_INVALID_PARAM;
    }

    err_code |= BME_TO_RUUVI_ERROR (bme280_crc_selftest (&dev));
    err_code |= BME_TO_RUUVI_ERROR (bme280_soft_reset (&dev));
    // Setup Oversampling 1 to enable sensor
    uint8_t dsp = RD_SENSOR_DSP_OS;
    uint8_t dsp_parameter = 1;
    err_code |= ri_bme280_dsp_set (&dsp, &dsp_parameter);

    if (RD_SUCCESS == err_code)
    {
        environmental_sensor->init              = ri_bme280_init;
        environmental_sensor->uninit            = ri_bme280_uninit;
        environmental_sensor->samplerate_set    = ri_bme280_samplerate_set;
        environmental_sensor->samplerate_get    = ri_bme280_samplerate_get;
        environmental_sensor->resolution_set    = ri_bme280_resolution_set;
        environmental_sensor->resolution_get    = ri_bme280_resolution_get;
        environmental_sensor->scale_set         = ri_bme280_scale_set;
        environmental_sensor->scale_get         = ri_bme280_scale_get;
        environmental_sensor->dsp_set           = ri_bme280_dsp_set;
        environmental_sensor->dsp_get           = ri_bme280_dsp_get;
        environmental_sensor->mode_set          = ri_bme280_mode_set;
        environmental_sensor->mode_get          = ri_bme280_mode_get;
        environmental_sensor->data_get          = ri_bme280_data_get;
        environmental_sensor->configuration_set = rd_sensor_configuration_set;
        environmental_sensor->configuration_get = rd_sensor_configuration_get;
        environmental_sensor->provides.datas.temperature_c = 1;
        environmental_sensor->provides.datas.humidity_rh = 1;
        environmental_sensor->provides.datas.pressure_pa = 1;
        tsample = RD_UINT64_INVALID;
    }

    return err_code;
}

rd_status_t ri_bme280_uninit (rd_sensor_t * sensor,
                              rd_bus_t bus, uint8_t handle)
{
    if (NULL == sensor) { return RD_ERROR_NULL; }

    rd_status_t err_code = BME_TO_RUUVI_ERROR (bme280_soft_reset (&dev));

    if (RD_SUCCESS != err_code) { return err_code; }

    rd_sensor_uninitialize (sensor);
    memset (&dev, 0, sizeof (dev));
    tsample = RD_UINT64_INVALID;
    return err_code;
}

rd_status_t ri_bme280_samplerate_set (uint8_t * samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    VERIFY_SENSOR_SLEEPS();
    rd_status_t err_code = RD_SUCCESS;

    if (RD_SENSOR_CFG_DEFAULT == *samplerate)  { dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
    else if (*samplerate == 1U)                           { dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
    else if (*samplerate == 2U)                           { dev.settings.standby_time = BME280_STANDBY_TIME_500_MS; }
    else if (*samplerate <= 8U)                           { dev.settings.standby_time = BME280_STANDBY_TIME_125_MS; }
    else if (*samplerate <= 16U)                          { dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS; }
    else if (*samplerate <= 50U)                          { dev.settings.standby_time = BME280_STANDBY_TIME_20_MS; }
    else if (*samplerate <= 100U)                         { dev.settings.standby_time = BME280_STANDBY_TIME_10_MS; }
    else if (*samplerate <= 200U)                         { dev.settings.standby_time = BME280_STANDBY_TIME_0_5_MS; }
    else if (RD_SENSOR_CFG_MIN == *samplerate) { dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
    else if (RD_SENSOR_CFG_MAX == *samplerate) { dev.settings.standby_time = BME280_STANDBY_TIME_0_5_MS; }
    else if (RD_SENSOR_CFG_NO_CHANGE == *samplerate) {} // do nothing
    else { *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED; err_code |= RD_ERROR_NOT_SUPPORTED; }

    if (RD_SUCCESS == err_code)
    {
        // BME 280 must be in standby while configured
        err_code |=  BME_TO_RUUVI_ERROR (bme280_set_sensor_settings (BME280_STANDBY_SEL, &dev));
        err_code |= ri_bme280_samplerate_get (samplerate);
    }

    return err_code;
}

rd_status_t ri_bme280_samplerate_get (uint8_t * samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    rd_status_t err_code = BME_TO_RUUVI_ERROR (bme280_get_sensor_settings (&dev));

    if (RD_SUCCESS != err_code) { return err_code; }

    if (BME280_STANDBY_TIME_1000_MS == dev.settings.standby_time)      { *samplerate = 1;   }
    else if (BME280_STANDBY_TIME_500_MS == dev.settings.standby_time)  { *samplerate = 2;   }
    else if (BME280_STANDBY_TIME_125_MS == dev.settings.standby_time)  { *samplerate = 8;   }
    else if (BME280_STANDBY_TIME_62_5_MS == dev.settings.standby_time) { *samplerate = 16;  }
    else if (BME280_STANDBY_TIME_20_MS == dev.settings.standby_time)   { *samplerate = 50;  }
    else if (BME280_STANDBY_TIME_10_MS == dev.settings.standby_time)   { *samplerate = 100;  }
    else if (BME280_STANDBY_TIME_0_5_MS == dev.settings.standby_time)    { *samplerate = 200; }

    return err_code;
}

rd_status_t ri_bme280_resolution_set (uint8_t * resolution)
{
    if (NULL == resolution) { return RD_ERROR_NULL; }

    VERIFY_SENSOR_SLEEPS();
    uint8_t original = *resolution;
    *resolution = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID (original);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_bme280_resolution_get (uint8_t * resolution)
{
    if (NULL == resolution) { return RD_ERROR_NULL; }

    *resolution = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_bme280_scale_set (uint8_t * scale)
{
    if (NULL == scale) { return RD_ERROR_NULL; }

    VERIFY_SENSOR_SLEEPS();
    uint8_t original = *scale;
    *scale = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID (original);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_bme280_scale_get (uint8_t * scale)
{
    if (NULL == scale) { return RD_ERROR_NULL; }

    *scale = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_bme280_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter) { return RD_ERROR_NULL; }

    VERIFY_SENSOR_SLEEPS();

    // Validate configuration
    if (1U  != *parameter
            && 2U  != *parameter
            && 4U  != *parameter
            && 8U  != *parameter
            && 16U != *parameter
            && RD_SENSOR_CFG_DEFAULT != *parameter
            && RD_SENSOR_CFG_MIN     != *parameter
            && RD_SENSOR_CFG_MAX     != *parameter
            && RD_SENSOR_DSP_LAST != *dsp)
    {
        return RD_ERROR_NOT_SUPPORTED;
    }

    // Error if DSP is not last, and if dsp is something else than IIR or OS
    if ( (RD_SENSOR_DSP_LAST != *dsp) &&
            ~ (RD_SENSOR_DSP_LOW_PASS | RD_SENSOR_DSP_OS) & (*dsp))
    {
        return RD_ERROR_NOT_SUPPORTED;
    }

    // Clear setup
    uint8_t settings_sel = 0U;
    // Always 1x oversampling to keep sensing element enabled
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_1X;
    dev.settings.osr_t = BME280_OVERSAMPLING_1X;
    dev.settings.filter = BME280_FILTER_COEFF_OFF;
    settings_sel |= BME280_OSR_PRESS_SEL;
    settings_sel |= BME280_OSR_TEMP_SEL;
    settings_sel |= BME280_OSR_HUM_SEL;
    settings_sel |= BME280_FILTER_SEL;

    // Setup IIR
    if (RD_SENSOR_DSP_LOW_PASS & *dsp)
    {
        if (RD_SENSOR_CFG_DEFAULT == *parameter || \
                RD_SENSOR_CFG_MIN     == *parameter || \
                1U == *parameter
           )
        {
            dev.settings.filter = BME280_FILTER_COEFF_OFF;
            *parameter = 1U;
        }
        else if (2 == *parameter)
        {
            dev.settings.filter = BME280_FILTER_COEFF_2;
            *parameter = 2U;
        }
        else if (4U >= *parameter)
        {
            dev.settings.filter = BME280_FILTER_COEFF_4;
            *parameter = 4U;
        }
        else if (8U >= *parameter)
        {
            dev.settings.filter = BME280_FILTER_COEFF_8;
            *parameter = 8U;
        }
        else if (RD_SENSOR_CFG_MAX == *parameter || \
                 16U >= *parameter)
        {
            dev.settings.filter = BME280_FILTER_COEFF_16;
            *parameter = 16U;
        }
        else
        {
            *parameter = RD_SENSOR_ERR_NOT_SUPPORTED;
            return RD_ERROR_NOT_SUPPORTED;
        }
    }

    // Setup Oversampling
    if (RD_SENSOR_DSP_OS & *dsp)
    {
        if (RD_SENSOR_CFG_DEFAULT == *parameter || \
                RD_SENSOR_CFG_MIN     == *parameter || \
                1U == *parameter)
        {
            dev.settings.osr_h = BME280_OVERSAMPLING_1X;
            dev.settings.osr_p = BME280_OVERSAMPLING_1X;
            dev.settings.osr_t = BME280_OVERSAMPLING_1X;
            *parameter = 1U;
        }
        else if (2 == *parameter)
        {
            dev.settings.osr_h = BME280_OVERSAMPLING_2X;
            dev.settings.osr_p = BME280_OVERSAMPLING_2X;
            dev.settings.osr_t = BME280_OVERSAMPLING_2X;
            *parameter = 2U;
        }
        else if (4U >= *parameter)
        {
            dev.settings.osr_h = BME280_OVERSAMPLING_4X;
            dev.settings.osr_p = BME280_OVERSAMPLING_4X;
            dev.settings.osr_t = BME280_OVERSAMPLING_4X;
            *parameter = 4U;
        }
        else if (8U >= *parameter)
        {
            dev.settings.osr_h = BME280_OVERSAMPLING_8X;
            dev.settings.osr_p = BME280_OVERSAMPLING_8X;
            dev.settings.osr_t = BME280_OVERSAMPLING_8X;
            *parameter = 8U;
        }
        else if (16U >= *parameter || \
                 RD_SENSOR_CFG_MAX)
        {
            dev.settings.osr_h = BME280_OVERSAMPLING_16X;
            dev.settings.osr_p = BME280_OVERSAMPLING_16X;
            dev.settings.osr_t = BME280_OVERSAMPLING_16X;
            *parameter = 16U;
        }
        else
        {
            *parameter = RD_SENSOR_ERR_NOT_SUPPORTED;
            return RD_ERROR_NOT_SUPPORTED;
        }
    }

    //Write configuration
    return BME_TO_RUUVI_ERROR (bme280_set_sensor_settings (settings_sel, &dev));
}

// Read configuration
rd_status_t ri_bme280_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;
    err_code |= BME_TO_RUUVI_ERROR (bme280_get_sensor_settings (&dev));

    if (RD_SUCCESS != err_code) { return err_code; }

    // Assume default / 0,
    *dsp       = RD_SENSOR_CFG_DEFAULT;
    *parameter = RD_SENSOR_CFG_DEFAULT;

    // Check if IIR has been set. If yes, read DSP param from there.
    if (BME280_FILTER_COEFF_OFF != dev.settings.filter)
    {
        *dsp |= RD_SENSOR_DSP_LOW_PASS;

        switch (dev.settings.filter)
        {
            case BME280_FILTER_COEFF_2:
                *parameter = 2U;
                break;

            case BME280_FILTER_COEFF_4:
                *parameter = 4U;
                break;

            case BME280_FILTER_COEFF_8:
                *parameter = 8U;
                break;

            case BME280_FILTER_COEFF_16:
                *parameter = 16U;
                break;
        }
    }

    // Check if OS has been set. If yes, read DSP param from there.
    // Param should be same for OS and IIR if it is >1.
    // OSR is same for every element.
    if (BME280_NO_OVERSAMPLING != dev.settings.osr_h
            && BME280_OVERSAMPLING_1X != dev.settings.osr_h)
    {
        *dsp |= RD_SENSOR_DSP_OS;

        switch (dev.settings.osr_h)
        {
            case BME280_OVERSAMPLING_2X:
                *parameter = 2U;
                break;

            case BME280_OVERSAMPLING_4X:
                *parameter = 4U;
                break;

            case BME280_OVERSAMPLING_8X:
                *parameter = 8U;
                break;

            case BME280_OVERSAMPLING_16X:
                *parameter = 16U;
                break;
        }
    }

    return RD_SUCCESS;
}

rd_status_t ri_bme280_mode_set (uint8_t * mode)
{
    if (NULL == mode) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;
    uint8_t current_mode = RD_SENSOR_CFG_SLEEP;

    switch (*mode)
    {
        case RD_SENSOR_CFG_SLEEP:
            err_code = BME_TO_RUUVI_ERROR (bme280_set_sensor_mode (BME280_SLEEP_MODE, &dev));
            break;

        case RD_SENSOR_CFG_SINGLE:
            // Do nothing if sensor is in continuous mode
            ri_bme280_mode_get (&current_mode);

            if (RD_SENSOR_CFG_CONTINUOUS == current_mode)
            {
                *mode = RD_SENSOR_CFG_CONTINUOUS;
                return RD_ERROR_INVALID_STATE;
            }

            err_code = BME_TO_RUUVI_ERROR (bme280_set_sensor_mode (BME280_FORCED_MODE, &dev));
            // We assume that dev struct is in sync with the state of the BME280 and underlying interface
            // which has the number of settings as 2^OSR is not changed.
            // We also assume that each element runs same OSR
            uint8_t samples = 1U << (dev.settings.osr_h - 1U);
            ri_delay_ms (bme280_max_meas_time (samples));
            tsample = rd_sensor_timestamp_get();
            // BME280 returns to SLEEP after forced sample
            *mode = RD_SENSOR_CFG_SLEEP;
            break;

        case RD_SENSOR_CFG_CONTINUOUS:
            err_code = BME_TO_RUUVI_ERROR (bme280_set_sensor_mode (BME280_NORMAL_MODE, &dev));
            break;

        default:
            err_code = RD_ERROR_INVALID_PARAM;
            break;
    }

    return err_code;
}

rd_status_t ri_bme280_mode_get (uint8_t * mode)
{
    if (NULL == mode) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;
    uint8_t bme_mode = 0U;
    err_code = BME_TO_RUUVI_ERROR (bme280_get_sensor_mode (&bme_mode, &dev));

    if (RD_SUCCESS != err_code) { return err_code; }

    switch (bme_mode)
    {
        case BME280_SLEEP_MODE:
            *mode = RD_SENSOR_CFG_SLEEP;
            break;

        case BME280_FORCED_MODE:
            *mode = RD_SENSOR_CFG_SINGLE;
            break;

        case BME280_NORMAL_MODE:
            *mode = RD_SENSOR_CFG_CONTINUOUS;
            break;

        default:
            *mode = RD_SENSOR_ERR_INVALID;
            break;
    }

    return err_code;
}


rd_status_t ri_bme280_data_get (rd_sensor_data_t * const
                                p_data)
{
    if (NULL == p_data) { return RD_ERROR_NULL; }

    struct bme280_data comp_data;

    rd_status_t err_code = RD_SUCCESS;

    err_code = BME_TO_RUUVI_ERROR (bme280_get_sensor_data (BME280_ALL, &comp_data, &dev));

    if (RD_SUCCESS != err_code) { return err_code; }

    // Write tsample if we're in single mode, current time if we're in continuous mode
    // Leave sample time as invalid if forced mode is ongoing.
    uint8_t mode = 0U;
    err_code |= ri_bme280_mode_get (&mode);

    if (RD_SENSOR_CFG_SLEEP == mode)           { p_data->timestamp_ms = tsample; }
    else if (RD_SENSOR_CFG_CONTINUOUS == mode) { p_data->timestamp_ms = rd_sensor_timestamp_get(); }
    else { RD_ERROR_CHECK (RD_ERROR_INTERNAL, ~RD_ERROR_FATAL); }

    // If we have valid data, return it.
    if (RD_UINT64_INVALID != p_data->timestamp_ms)
    {
        rd_sensor_data_t d_environmental = {0};
        rd_sensor_data_fields_t env_fields = {.bitfield = 0};
        float env_values[3];
        env_values[0] = (float) comp_data.humidity;
        env_values[1] = (float) comp_data.pressure;
        env_values[2] = (float) comp_data.temperature;
        env_fields.datas.humidity_rh = 1U;
        env_fields.datas.pressure_pa = 1U;
        env_fields.datas.temperature_c = 1U;
        d_environmental.data = env_values;
        d_environmental.fields = env_fields;
        d_environmental.valid  = env_fields;
        rd_sensor_data_populate (p_data,
                                 &d_environmental,
                                 p_data->fields);
    }

    return err_code;
}
/*@}*/
#endif