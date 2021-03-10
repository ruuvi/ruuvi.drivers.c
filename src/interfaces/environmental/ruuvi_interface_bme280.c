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
#define BME280_MEAS_TIME_CONST1         (1.25F)
#define BME280_MEAS_TIME_CONST2         (2.3F)
#define BME280_MEAS_TIME_CONST3         (3.0F)
#define BME280_MEAS_TIME_CONST4         (2.0F)
#define BME280_MEAS_TIME_CONST5         (0.575F)
#define BME280_MEAS_TIME_CONST6         (2U)

#define BME280_SAMPLERATE_1000MS         (1U)
#define BME280_SAMPLERATE_500MS          (2U)
#define BME280_SAMPLERATE_125MS          (8U)
#define BME280_SAMPLERATE_62_5MS         (16U)
#define BME280_SAMPLERATE_20MS           (50U)
#define BME280_SAMPLERATE_10MS           (100U)
#define BME280_SAMPLERATE_0_5MS          (200U)

#define BME280_DSP_MODE_0                (1U)
#define BME280_DSP_MODE_1                (2U)
#define BME280_DSP_MODE_2                (4U)
#define BME280_DSP_MODE_3                (8U)
#define BME280_DSP_MODE_4                (16U)

#define BME280_HUMIDITY                  (0)
#define BME280_PRESSURE                  (1)
#define BME280_TEMPERATURE               (2)
#define BME280_SENS_NUM                  (3)

#define BME280_HUMIDITY_MAX_VALUE        (100.0f)

typedef float bme_float;

/** State variables **/
#ifndef CEEDLING
static
#endif
struct bme280_dev dev = {0};
static uint64_t tsample;
static const char m_sensor_name[] = "BME280";

/** @brief Function for checking that sensor is in sleep mode before configuration */
#ifndef CEEDLING
static
#endif
rd_status_t bme280_verify_sensor_sleep (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = 0;
    ri_bme280_mode_get (&mode);

    if (RD_SENSOR_CFG_SLEEP != mode)
    {
        err_code = RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

/** @brief Function for checking "ignored" parameters NO_CHANGE, MIN, MAX, DEFAULT */
static rd_status_t bme280_success_on_valid (uint8_t param)
{
    rd_status_t err_code = RD_ERROR_INVALID_PARAM;

    if ( (RD_SENSOR_CFG_DEFAULT   == param) ||
            (RD_SENSOR_CFG_MIN       == param) ||
            (RD_SENSOR_CFG_MAX       == param) ||
            (RD_SENSOR_CFG_NO_CHANGE == param))
    {
        err_code = RD_SUCCESS;
    }

    return err_code;
}

/**
 * Convert error from BME280 driver to appropriate NRF ERROR
 */
static rd_status_t BME_TO_RUUVI_ERROR (int8_t rslt)
{
    rd_status_t err_code = RD_SUCCESS;

    if (BME280_OK == rslt) { err_code = RD_SUCCESS; }
    else if (BME280_E_DEV_NOT_FOUND == rslt)  { err_code = RD_ERROR_NOT_FOUND; }
    else if (BME280_E_NULL_PTR == rslt)  { err_code = RD_ERROR_NULL; }
    else if (BME280_E_COMM_FAIL == rslt) { err_code = RD_ERROR_BUSY; }
    else { err_code = RD_ERROR_INTERNAL; }

    return err_code;
}

void bosch_delay_ms (uint32_t time_ms)
{
    ri_delay_ms (time_ms);
}

// BME280 datasheet Appendix B.
#ifndef CEEDLING
static
#endif
uint32_t bme280_max_meas_time (const uint8_t oversampling)
{
    // Time
    bme_float rd_time = BME280_MEAS_TIME_CONST1 + \
                        BME280_MEAS_TIME_CONST2 * BME280_MEAS_TIME_CONST3 * oversampling + \
                        BME280_MEAS_TIME_CONST4 * BME280_MEAS_TIME_CONST5;
    // Roundoff + margin
    uint32_t ret_time = (uint32_t) rd_time;
    ret_time += BME280_MEAS_TIME_CONST6;
    return ret_time;
}

#ifndef CEEDLING
static
#endif
rd_status_t bme280_spi_init (const struct bme280_dev * const p_dev, const uint8_t handle)
{
    (void) (p_dev);
    rd_status_t err_code = RD_ERROR_NOT_ENABLED;
#if RI_BME280_SPI_ENABLED
    dev.dev_id = handle;
    dev.intf = BME280_SPI_INTF;
    dev.read = &ri_spi_bme280_read;
    dev.write = &ri_spi_bme280_write;
    dev.delay_ms = bosch_delay_ms;
    err_code = BME_TO_RUUVI_ERROR (bme280_init (&dev));
#endif
    return err_code;
}

#ifndef CEEDLING
static
#endif
rd_status_t bme280_i2c_init (const struct bme280_dev * const p_dev, const uint8_t handle)
{
    (void) (p_dev);
    rd_status_t err_code = RD_ERROR_NOT_ENABLED;
#if RI_BME280_I2C_ENABLED
    dev.dev_id = handle;
    dev.intf = BME280_I2C_INTF;
    dev.read = &ri_i2c_bme280_read;
    dev.write = &ri_i2c_bme280_write;
    dev.delay_ms = bosch_delay_ms;
    err_code = BME_TO_RUUVI_ERROR (bme280_init (&dev));
#endif
    return err_code;
}

static void bme280_ri_setup (rd_sensor_t * const environmental_sensor)
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

/** Initialize BME280 into low-power mode **/
rd_status_t ri_bme280_init (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == p_sensor)
    {
        err_code = RD_ERROR_NULL;
    }
    else if (rd_sensor_is_init (p_sensor))
    {
        err_code = RD_ERROR_INVALID_STATE;
    }
    else
    {
        rd_sensor_initialize (p_sensor);
        p_sensor->name = m_sensor_name;

        if (RD_BUS_SPI == bus)
        {
            err_code |= bme280_spi_init (&dev, handle);
        }
        else if (RD_BUS_I2C == bus)
        {
            err_code |= bme280_i2c_init (&dev, handle);
        }
        else
        {
            err_code |= RD_ERROR_INVALID_PARAM;
        }

        if (RD_SUCCESS == err_code)
        {
            err_code |= BME_TO_RUUVI_ERROR (bme280_soft_reset (&dev));
            // Setup Oversampling 1 to enable sensor
            uint8_t dsp = RD_SENSOR_DSP_OS;
            uint8_t dsp_parameter = 1;
            err_code |= ri_bme280_dsp_set (&dsp, &dsp_parameter);
        }

        if (RD_SUCCESS == err_code)
        {
            bme280_ri_setup (p_sensor);
        }
    }

    return err_code;
}

rd_status_t ri_bme280_uninit (rd_sensor_t * sensor,
                              rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
    (void) bus;
    (void) handle;

    if (NULL == sensor)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        err_code = BME_TO_RUUVI_ERROR (bme280_soft_reset (&dev));

        if (RD_SUCCESS == err_code)
        {
            rd_sensor_uninitialize (sensor);
            memset (&dev, 0, sizeof (dev));
            tsample = RD_UINT64_INVALID;
        }
    }

    return err_code;
}

#ifndef CEEDLING
static
#endif
rd_status_t ri2bme_rate (struct bme280_dev * p_dev, uint8_t * const samplerate)
{
    rd_status_t err_code  = RD_SUCCESS;

    if (RD_SENSOR_CFG_DEFAULT == *samplerate)  { p_dev->settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
    else if (BME280_SAMPLERATE_1000MS == *samplerate) { p_dev->settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
    else if (BME280_SAMPLERATE_500MS == *samplerate)  { p_dev->settings.standby_time = BME280_STANDBY_TIME_500_MS; }
    else if (*samplerate <= BME280_SAMPLERATE_125MS)  { p_dev->settings.standby_time = BME280_STANDBY_TIME_125_MS; }
    else if (*samplerate <= BME280_SAMPLERATE_62_5MS) { p_dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS; }
    else if (*samplerate <= BME280_SAMPLERATE_20MS)   { p_dev->settings.standby_time = BME280_STANDBY_TIME_20_MS; }
    else if (*samplerate <= BME280_SAMPLERATE_10MS)   { p_dev->settings.standby_time = BME280_STANDBY_TIME_10_MS; }
    else if (*samplerate <= BME280_SAMPLERATE_0_5MS)  { p_dev->settings.standby_time = BME280_STANDBY_TIME_0_5_MS; }
    else if (RD_SENSOR_CFG_MIN == *samplerate) { p_dev->settings.standby_time = BME280_STANDBY_TIME_1000_MS; }
    else if (RD_SENSOR_CFG_MAX == *samplerate) { p_dev->settings.standby_time = BME280_STANDBY_TIME_0_5_MS; }
    else
    {
        if (RD_SENSOR_CFG_NO_CHANGE != *samplerate)
        {
            *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED;
            err_code |= RD_ERROR_NOT_SUPPORTED;
        }
    }

    return err_code;
}

rd_status_t ri_bme280_samplerate_set (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == samplerate)
    {
        err_code = RD_ERROR_NULL;
    }
    else if (RD_SUCCESS == bme280_verify_sensor_sleep())
    {
        err_code = ri2bme_rate (&dev, samplerate);

        if (RD_SUCCESS == err_code)
        {
            err_code |= BME_TO_RUUVI_ERROR (bme280_set_sensor_settings (BME280_STANDBY_SEL, &dev));
            err_code |= ri_bme280_samplerate_get (samplerate);
        }
    }
    else
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

rd_status_t ri_bme280_samplerate_get (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == samplerate)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        err_code = BME_TO_RUUVI_ERROR (bme280_get_sensor_settings (&dev));

        if (RD_SUCCESS == err_code)
        {
            if (BME280_STANDBY_TIME_1000_MS == dev.settings.standby_time)
            {
                *samplerate = BME280_SAMPLERATE_1000MS;
            }
            else if (BME280_STANDBY_TIME_500_MS == dev.settings.standby_time)
            {
                *samplerate = BME280_SAMPLERATE_500MS;
            }
            else if (BME280_STANDBY_TIME_125_MS == dev.settings.standby_time)
            {
                *samplerate = BME280_SAMPLERATE_125MS;
            }
            else if (BME280_STANDBY_TIME_62_5_MS == dev.settings.standby_time)
            {
                *samplerate = BME280_SAMPLERATE_62_5MS;
            }
            else if (BME280_STANDBY_TIME_20_MS == dev.settings.standby_time)
            {
                *samplerate = BME280_SAMPLERATE_20MS;
            }
            else if (BME280_STANDBY_TIME_10_MS == dev.settings.standby_time)
            {
                *samplerate = BME280_SAMPLERATE_10MS;
            }
            else if (BME280_STANDBY_TIME_0_5_MS == dev.settings.standby_time)
            {
                *samplerate = BME280_SAMPLERATE_0_5MS;
            }
            else
            {
                *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED;
            }
        }
    }

    return err_code;
}

rd_status_t ri_bme280_resolution_set (uint8_t * resolution)
{
    rd_status_t err_code = RD_ERROR_NOT_SUPPORTED;

    if (NULL == resolution)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        err_code = bme280_verify_sensor_sleep();

        if (RD_SUCCESS == err_code)
        {
            uint8_t original = *resolution;
            *resolution = RD_SENSOR_CFG_DEFAULT;
            err_code = bme280_success_on_valid (original);

            if (RD_SUCCESS != err_code)
            {
                err_code = RD_ERROR_NOT_SUPPORTED;
            }
        }
    }

    return err_code;
}

rd_status_t ri_bme280_resolution_get (uint8_t * resolution)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == resolution)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        *resolution = RD_SENSOR_CFG_DEFAULT;
    }

    return err_code;
}

rd_status_t ri_bme280_scale_set (uint8_t * scale)
{
    rd_status_t err_code = RD_ERROR_NOT_SUPPORTED;

    if (NULL == scale)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        err_code = bme280_verify_sensor_sleep();

        if (RD_SUCCESS == err_code)
        {
            uint8_t original = *scale;
            *scale = RD_SENSOR_CFG_DEFAULT;
            err_code = bme280_success_on_valid (original);

            if (RD_SUCCESS != err_code)
            {
                err_code = RD_ERROR_NOT_SUPPORTED;
            }
        }
    }

    return err_code;
}

rd_status_t ri_bme280_scale_get (uint8_t * scale)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == scale)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        *scale = RD_SENSOR_CFG_DEFAULT;
    }

    return err_code;
}

static rd_status_t ri_bme280_dsp_setup_over (uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (RD_SENSOR_CFG_DEFAULT == *parameter) || \
            (RD_SENSOR_CFG_MIN     == *parameter) || \
            (BME280_DSP_MODE_0 == *parameter))
    {
        dev.settings.osr_h = BME280_OVERSAMPLING_1X;
        dev.settings.osr_p = BME280_OVERSAMPLING_1X;
        dev.settings.osr_t = BME280_OVERSAMPLING_1X;
        *parameter = BME280_DSP_MODE_0;
    }
    else if (BME280_DSP_MODE_1 == *parameter)
    {
        dev.settings.osr_h = BME280_OVERSAMPLING_2X;
        dev.settings.osr_p = BME280_OVERSAMPLING_2X;
        dev.settings.osr_t = BME280_OVERSAMPLING_2X;
        *parameter = BME280_DSP_MODE_1;
    }
    else if (BME280_DSP_MODE_2 >= *parameter)
    {
        dev.settings.osr_h = BME280_OVERSAMPLING_4X;
        dev.settings.osr_p = BME280_OVERSAMPLING_4X;
        dev.settings.osr_t = BME280_OVERSAMPLING_4X;
        *parameter = BME280_DSP_MODE_2;
    }
    else if (BME280_DSP_MODE_3 >= *parameter)
    {
        dev.settings.osr_h = BME280_OVERSAMPLING_8X;
        dev.settings.osr_p = BME280_OVERSAMPLING_8X;
        dev.settings.osr_t = BME280_OVERSAMPLING_8X;
        *parameter = BME280_DSP_MODE_3;
    }
    else if ( (BME280_DSP_MODE_4 >= *parameter) || \
              (RD_SENSOR_CFG_MAX == *parameter))
    {
        dev.settings.osr_h = BME280_OVERSAMPLING_16X;
        dev.settings.osr_p = BME280_OVERSAMPLING_16X;
        dev.settings.osr_t = BME280_OVERSAMPLING_16X;
        *parameter = BME280_DSP_MODE_4;
    }
    else
    {
        *parameter = RD_SENSOR_ERR_NOT_SUPPORTED;
        err_code = RD_ERROR_NOT_SUPPORTED;
    }

    return err_code;
}

static rd_status_t ri_bme280_dsp_setup (uint8_t * p_settings_sel, const uint8_t * dsp,
                                        uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;

    // Error if DSP is not last, and if dsp is something else than IIR or OS
    if ( (RD_SENSOR_DSP_LAST != *dsp) &&
            (~ (RD_SENSOR_DSP_LOW_PASS | RD_SENSOR_DSP_OS) & (*dsp)) != 0)
    {
        err_code = RD_ERROR_NOT_SUPPORTED;
    }
    else
    {
        // Always 1x oversampling to keep sensing element enabled
        dev.settings.osr_h = BME280_OVERSAMPLING_1X;
        dev.settings.osr_p = BME280_OVERSAMPLING_1X;
        dev.settings.osr_t = BME280_OVERSAMPLING_1X;
        dev.settings.filter = BME280_FILTER_COEFF_OFF;
        *p_settings_sel |= BME280_OSR_PRESS_SEL;
        *p_settings_sel |= BME280_OSR_TEMP_SEL;
        *p_settings_sel |= BME280_OSR_HUM_SEL;
        *p_settings_sel |= BME280_FILTER_SEL;

        // Setup IIR
        if (RD_SENSOR_DSP_LOW_PASS & *dsp)
        {
            if ( (RD_SENSOR_CFG_DEFAULT == *parameter) || \
                    (RD_SENSOR_CFG_MIN     == *parameter) || \
                    (BME280_DSP_MODE_0 == *parameter))
            {
                dev.settings.filter = BME280_FILTER_COEFF_OFF;
                *parameter = BME280_DSP_MODE_0;
            }
            else if (BME280_DSP_MODE_1 == *parameter)
            {
                dev.settings.filter = BME280_FILTER_COEFF_2;
                *parameter = BME280_DSP_MODE_1;
            }
            else if (BME280_DSP_MODE_2 >= *parameter)
            {
                dev.settings.filter = BME280_FILTER_COEFF_4;
                *parameter = BME280_DSP_MODE_2;
            }
            else if (BME280_DSP_MODE_3 >= *parameter)
            {
                dev.settings.filter = BME280_FILTER_COEFF_8;
                *parameter = BME280_DSP_MODE_3;
            }
            else if ( (RD_SENSOR_CFG_MAX == *parameter) || \
                      (BME280_DSP_MODE_4 >= *parameter))
            {
                dev.settings.filter = BME280_FILTER_COEFF_16;
                *parameter = BME280_DSP_MODE_4;
            }
            else
            {
                *parameter = RD_SENSOR_ERR_NOT_SUPPORTED;
                err_code = RD_ERROR_NOT_SUPPORTED;
            }
        }

        if ( (RD_SUCCESS == err_code) &&
                ( (RD_SENSOR_DSP_OS & *dsp) != 0))
        {
            err_code = ri_bme280_dsp_setup_over (parameter);
        }
    }

    return err_code;
}

rd_status_t ri_bme280_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t settings_sel = 0U;

    if ( (NULL == dsp) || (NULL == parameter))
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        err_code = bme280_verify_sensor_sleep();

        if (RD_SUCCESS == err_code)
        {
            // Validate configuration
            if ( (RD_SENSOR_CFG_DEFAULT != *parameter)
                    && (RD_SENSOR_CFG_MIN     != *parameter)
                    && (RD_SENSOR_CFG_MAX     != *parameter))
            {
                err_code = RD_ERROR_NOT_SUPPORTED;
            }

            if ( (RD_SUCCESS != err_code)  &&
                    ( (RD_SENSOR_DSP_LAST == *dsp) ||
                      (BME280_DSP_MODE_0 == *parameter) ||
                      (BME280_DSP_MODE_1 == *parameter)))
            {
                err_code = RD_SUCCESS;
            }

            if ( (RD_SUCCESS != err_code) &&
                    ( (BME280_DSP_MODE_2 == *parameter) ||
                      (BME280_DSP_MODE_3 == *parameter) ||
                      (BME280_DSP_MODE_4 == *parameter)))
            {
                err_code = RD_SUCCESS;
            }

            if (RD_SUCCESS == err_code)
            {
                err_code = ri_bme280_dsp_setup (&settings_sel, dsp, parameter);
            }
        }
    }

    if (RD_SUCCESS == err_code)
    {
        //Write configuration
        err_code = BME_TO_RUUVI_ERROR (bme280_set_sensor_settings (settings_sel, &dev));
    }

    return err_code;
}

static void ri_bme280_dsp_get_param (uint8_t * dsp, uint8_t * parameter)
{
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
                *parameter = BME280_DSP_MODE_1;
                break;

            case BME280_FILTER_COEFF_4:
                *parameter = BME280_DSP_MODE_2;
                break;

            case BME280_FILTER_COEFF_8:
                *parameter = BME280_DSP_MODE_3;
                break;

            case BME280_FILTER_COEFF_16:
                *parameter = BME280_DSP_MODE_4;
                break;

            default:
                *parameter = BME280_DSP_MODE_1;
                break;
        }
    }

    // Check if OS has been set. If yes, read DSP param from there.
    // Param should be same for OS and IIR if it is >1.
    // OSR is same for every element.
    if ( (BME280_NO_OVERSAMPLING != dev.settings.osr_h)
            && (BME280_OVERSAMPLING_1X != dev.settings.osr_h))
    {
        *dsp |= RD_SENSOR_DSP_OS;

        switch (dev.settings.osr_h)
        {
            case BME280_OVERSAMPLING_2X:
                *parameter = BME280_DSP_MODE_1;
                break;

            case BME280_OVERSAMPLING_4X:
                *parameter = BME280_DSP_MODE_2;
                break;

            case BME280_OVERSAMPLING_8X:
                *parameter = BME280_DSP_MODE_3;
                break;

            case BME280_OVERSAMPLING_16X:
                *parameter = BME280_DSP_MODE_4;
                break;

            default:
                *parameter = BME280_DSP_MODE_1;
                break;
        }
    }
}

// Read configuration
rd_status_t ri_bme280_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == dsp) || (NULL == parameter))
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        err_code |= BME_TO_RUUVI_ERROR (bme280_get_sensor_settings (&dev));

        if (RD_SUCCESS == err_code)
        {
            ri_bme280_dsp_get_param (dsp, parameter);
        }
    }

    return err_code;
}

static rd_status_t ri_bme280_mode_set_single (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t current_mode = RD_SENSOR_CFG_SLEEP;
    // Do nothing if sensor is in continuous mode
    ri_bme280_mode_get (&current_mode);

    if (RD_SENSOR_CFG_CONTINUOUS == current_mode)
    {
        *mode = RD_SENSOR_CFG_CONTINUOUS;
        err_code = RD_ERROR_INVALID_STATE;
    }
    else
    {
        err_code = BME_TO_RUUVI_ERROR (bme280_set_sensor_mode (BME280_FORCED_MODE, &dev));
        // We assume that dev struct is in sync with the state of the BME280 and underlying interface
        // which has the number of settings as 2^OSR is not changed.
        // We also assume that each element runs same OSR
        uint8_t samples = (uint8_t) (1U << (dev.settings.osr_h - 1U));
        ri_delay_ms (bme280_max_meas_time (samples));
        tsample = rd_sensor_timestamp_get();
        // BME280 returns to SLEEP after forced sample
        *mode = RD_SENSOR_CFG_SLEEP;
    }

    return err_code;
}

rd_status_t ri_bme280_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        switch (*mode)
        {
            case RD_SENSOR_CFG_SLEEP:
                err_code = BME_TO_RUUVI_ERROR (bme280_set_sensor_mode (BME280_SLEEP_MODE, &dev));
                break;

            case RD_SENSOR_CFG_SINGLE:
                err_code = ri_bme280_mode_set_single (mode);
                break;

            case RD_SENSOR_CFG_CONTINUOUS:
                err_code = BME_TO_RUUVI_ERROR (bme280_set_sensor_mode (BME280_NORMAL_MODE, &dev));
                break;

            default:
                err_code = RD_ERROR_INVALID_PARAM;
                break;
        }
    }

    return err_code;
}

rd_status_t ri_bme280_mode_get (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        uint8_t bme_mode = 0U;
        err_code = BME_TO_RUUVI_ERROR (bme280_get_sensor_mode (&bme_mode, &dev));

        if (RD_SUCCESS == err_code)
        {
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
        }
    }

    return err_code;
}

static void ri_bme280_check_humiduty (float * p_value)
{
    if (*p_value > BME280_HUMIDITY_MAX_VALUE)
    {
        *p_value = BME280_HUMIDITY_MAX_VALUE;
    }
}

rd_status_t ri_bme280_data_get (rd_sensor_data_t * const
                                p_data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == p_data)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        struct bme280_data comp_data;
        err_code = BME_TO_RUUVI_ERROR (bme280_get_sensor_data (BME280_ALL, &comp_data, &dev));

        if (RD_SUCCESS == err_code)
        {
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
                float env_values[BME280_SENS_NUM];
                env_values[BME280_HUMIDITY] = (float) (comp_data.humidity - BME280_HUMIDITY_OFFSET);
                ri_bme280_check_humiduty (&env_values[BME280_HUMIDITY]);
                env_values[BME280_PRESSURE] = (float) comp_data.pressure;
                env_values[BME280_TEMPERATURE] = (float) comp_data.temperature;
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
        }
    }

    return err_code;
}
/*@}*/
#endif
