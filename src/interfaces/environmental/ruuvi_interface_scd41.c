/**
 * @file ruuvi_interface_scd41.c
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @author TheSomeMan
 * @date 2023-05-25
 *
 * Sensirion SCD41 (Environmental Sensor Node for HVAC and Air Quality Applications) driver.
 */

#include "ruuvi_driver_enabled_modules.h"
#if RI_SCD4X_ENABLED || DOXYGEN
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
// Ruuvi headers
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_scd41.h"
#include "ruuvi_interface_environmental.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_interface_log.h"

/**
 * @addtogroup SCD41
 */
/** @{ */

// Sensirion driver.
#include "scd4x_i2c.h"
#include "ruuvi_interface_i2c_sen5x_scd4x.h"

#define SCD41_DELAY_AFTER_POWER_UP_MS                   (1000U)
#define SCD41_DELAY_AFTER_WAKE_UP_MS                    (20U)
#define SCD41_DELAY_AFTER_STOP_PERIODIC_MEASUREMENTS_MS (500U)
#define SCD41_DELAY_BETWEEN_CHECKING_READY_FLAG_MS      (100U)
#define SCD41_DELAY_MEASUREMENT_READY_MS                (5000U)

#define SCD41_LOG_BUF_SIZE_SERIAL       (80U)
#define SCD41_LOG_BUF_SIZE_MEASUREMENTS (100U)

#define SCD4X_NUM_MEASUREMENTS  (3U)

#define SCD4X_MEASUREMENT_IDX_CO2          (0U)
#define SCD4X_MEASUREMENT_IDX_HUMIDITY     (1U)
#define SCD4X_MEASUREMENT_IDX_TEMPERATURE  (2U)

#define SCD41_SCALE_FACTOR_TEMPERATURE  (1000.0f)
#define SCD41_SCALE_FACTOR_HUMIDITY     (1000.0f)

typedef float scd4x_float_t;
typedef int scd5x_printf_int_t;

/** @brief Check for "ignored" parameters NO_CHANGE, MIN, MAX, DEFAULT */
static inline bool check_is_param_valid (const uint8_t param)
{
    if ( (RD_SENSOR_CFG_DEFAULT   == param) ||
            (RD_SENSOR_CFG_MIN       == param) ||
            (RD_SENSOR_CFG_MAX       == param) ||
            (RD_SENSOR_CFG_NO_CHANGE == param)
       )
    {
        return true;
    }

    return false;
}

/** @brief Check that sensor is in sleep mode before configuration */
static bool check_is_sensor_in_sleep_mode (void)
{
    uint8_t mode = 0;
    ri_scd41_mode_get (&mode);

    if (RD_SENSOR_CFG_SLEEP == mode)
    {
        return true;
    }

    return false;
}

static uint64_t m_tsample; //!< Timestamp of sample.
static bool m_autorefresh; //!< Flag to refresh data on data_get.
static int32_t m_co2; //!< Last measured CO2 concentration.
static int32_t m_ambient_temperature; //!< Last measured temperature
static int32_t m_ambient_humidity; //!< Last measured humidity
static bool m_is_init; //!< Flag, is sensor init.
static const char m_sensor_name[] = "SCD41"; //!< Human-readable name of the sensor.

typedef enum scd41_status_e
{
    STATUS_OK = 0,         //!< SCD4X driver ok
    STATUS_UNKNOWN_DEVICE, //!< Invalid WHOAMI
    STATUS_ERR_BAD_DATA,   //!< Bad data
} scd41_status_e;

/**
 * @brief Convert error from SCD4X driver to appropriate NRF ERROR
 *
 * @param[in] rslt error code from SCD4X driver
 * @return    Ruuvi error code corresponding to SCD4X error code
 */
static rd_status_t SCD4X_TO_RUUVI_ERROR (const scd41_status_e status)
{
    switch (status)
    {
        case STATUS_OK:
            return RD_SUCCESS;

        case STATUS_UNKNOWN_DEVICE:
            return RD_ERROR_NOT_FOUND;

        case STATUS_ERR_BAD_DATA:
            return RD_ERROR_INVALID_DATA;
    }

    return RD_ERROR_INTERNAL;
}

static rd_status_t ri_scd41_check_sensor (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_delay_ms (SCD41_DELAY_AFTER_POWER_UP_MS);

    if (RD_SUCCESS == err_code)
    {
        err_code |= SCD4X_TO_RUUVI_ERROR (scd4x_wake_up());

        if (RD_SUCCESS != err_code)
        {
            ri_log (RI_LOG_LEVEL_ERROR, "sen5x: scd4x_wake_up: error\n");
        }
    }

    if (RD_SUCCESS == err_code)
    {
        err_code |= SCD4X_TO_RUUVI_ERROR (scd4x_stop_periodic_measurement());

        if (RD_SUCCESS != err_code)
        {
            ri_log (RI_LOG_LEVEL_ERROR, "scd4x: scd4x_stop_periodic_measurement: error\n");
        }
        else
        {
            ri_delay_ms (SCD41_DELAY_AFTER_STOP_PERIODIC_MEASUREMENTS_MS);
        }
    }

    if (RD_SUCCESS == err_code)
    {
        ri_log (RI_LOG_LEVEL_DEBUG, "scd4x: scd4x_reinit\n");
        err_code |= SCD4X_TO_RUUVI_ERROR (scd4x_reinit());

        if (RD_SUCCESS != err_code)
        {
            ri_log (RI_LOG_LEVEL_ERROR, "scd4x: scd4x_reinit: error\n");
        }
    }

    uint16_t serial_0 = 0;
    uint16_t serial_1 = 0;
    uint16_t serial_2 = 0;

    if (RD_SUCCESS == err_code)
    {
        err_code |= SCD4X_TO_RUUVI_ERROR (scd4x_get_serial_number (&serial_0,
                                          &serial_1,
                                          &serial_2));

        if (RD_SUCCESS != err_code)
        {
            ri_log (RI_LOG_LEVEL_ERROR, "scd4x: scd4x_get_serial_number: error\n");
        }
    }

    if (RD_SUCCESS == err_code)
    {
        char log_buf[SCD41_LOG_BUF_SIZE_SERIAL];
        (void) snprintf (log_buf, sizeof (log_buf),
                         "scd4x: Serial: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
        ri_log (RI_LOG_LEVEL_INFO, log_buf);
    }

    return err_code;
}

rd_status_t ri_scd41_init (rd_sensor_t * sensor, rd_bus_t bus, uint8_t handle)
{
    (void) handle;

    if (NULL == sensor)
    {
        return RD_ERROR_NULL;
    }

    if (rd_sensor_is_init (sensor))
    {
        return RD_ERROR_INVALID_STATE;
    }

    rd_sensor_initialize (sensor);
    sensor->name = m_sensor_name;

    if (RD_BUS_I2C != bus)
    {
        return RD_ERROR_INVALID_PARAM;
    }

    const rd_status_t err_code = ri_scd41_check_sensor();

    if (RD_SUCCESS != err_code)
    {
        return RD_ERROR_NOT_FOUND;
    }

    sensor->init              = ri_scd41_init;
    sensor->uninit            = ri_scd41_uninit;
    sensor->samplerate_set    = ri_scd41_samplerate_set;
    sensor->samplerate_get    = ri_scd41_samplerate_get;
    sensor->resolution_set    = ri_scd41_resolution_set;
    sensor->resolution_get    = ri_scd41_resolution_get;
    sensor->scale_set         = ri_scd41_scale_set;
    sensor->scale_get         = ri_scd41_scale_get;
    sensor->dsp_set           = ri_scd41_dsp_set;
    sensor->dsp_get           = ri_scd41_dsp_get;
    sensor->mode_set          = ri_scd41_mode_set;
    sensor->mode_get          = ri_scd41_mode_get;
    sensor->data_get          = ri_scd41_data_get;
    sensor->configuration_set = rd_sensor_configuration_set;
    sensor->configuration_get = rd_sensor_configuration_get;
    sensor->provides.datas.co2_ppm       = 1;
    sensor->provides.datas.humidity_rh   = 1;
    sensor->provides.datas.temperature_c = 1;
    m_tsample = RD_UINT64_INVALID;
    m_is_init = true;
    return RD_SUCCESS;
}

rd_status_t ri_scd41_uninit (rd_sensor_t * sensor, rd_bus_t bus, uint8_t handle)
{
    (void) bus;
    (void) handle;

    if (NULL == sensor) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;
    err_code |= SCD4X_TO_RUUVI_ERROR (scd4x_stop_periodic_measurement());
    ri_delay_ms (SCD41_DELAY_AFTER_STOP_PERIODIC_MEASUREMENTS_MS);
    rd_sensor_uninitialize (sensor);
    m_tsample = RD_UINT64_INVALID;
    m_co2 = RD_INT32_INVALID;
    m_ambient_temperature = RD_INT32_INVALID;
    m_ambient_humidity = RD_INT32_INVALID;
    m_is_init = false;
    m_autorefresh = false;
    return err_code;
}

rd_status_t ri_scd41_samplerate_set (uint8_t * samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    if (!check_is_sensor_in_sleep_mode())
    {
        return RD_ERROR_INVALID_STATE;
    }

    rd_status_t err_code = RD_SUCCESS;

    if (RD_SENSOR_CFG_DEFAULT == *samplerate)  { *samplerate = RD_SENSOR_CFG_DEFAULT; }
    else if (RD_SENSOR_CFG_NO_CHANGE == *samplerate) { *samplerate = RD_SENSOR_CFG_DEFAULT; }
    else if (RD_SENSOR_CFG_MIN == *samplerate) { *samplerate = RD_SENSOR_CFG_DEFAULT; }
    else if (RD_SENSOR_CFG_MAX == *samplerate) {*samplerate = RD_SENSOR_CFG_DEFAULT; }
    else { *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED; err_code |= RD_ERROR_NOT_SUPPORTED; }

    return err_code;
}

rd_status_t ri_scd41_samplerate_get (uint8_t * samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    *samplerate = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_scd41_resolution_set (uint8_t * resolution)
{
    if (NULL == resolution)
    {
        return RD_ERROR_NULL;
    }

    if (!check_is_sensor_in_sleep_mode())
    {
        return RD_ERROR_INVALID_STATE;
    }

    uint8_t original = *resolution;
    *resolution = RD_SENSOR_CFG_DEFAULT;

    if (!check_is_param_valid (original))
    {
        return RD_ERROR_NOT_SUPPORTED;
    }

    return RD_SUCCESS;
}

rd_status_t ri_scd41_resolution_get (uint8_t * resolution)
{
    if (NULL == resolution)
    {
        return RD_ERROR_NULL;
    }

    *resolution = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_scd41_scale_set (uint8_t * scale)
{
    if (NULL == scale)
    {
        return RD_ERROR_NULL;
    }

    if (!check_is_sensor_in_sleep_mode())
    {
        return RD_ERROR_INVALID_STATE;
    }

    uint8_t original = *scale;
    *scale = RD_SENSOR_CFG_DEFAULT;

    if (check_is_param_valid (original))
    {
        return RD_SUCCESS;
    }

    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_scd41_scale_get (uint8_t * scale)
{
    if (NULL == scale)
    {
        return RD_ERROR_NULL;
    }

    *scale = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_scd41_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    if ( (NULL == dsp) || (NULL == parameter))
    {
        return RD_ERROR_NULL;
    }

    if (!check_is_sensor_in_sleep_mode())
    {
        return RD_ERROR_INVALID_STATE;
    }

    // Validate configuration
    if ( ( (RD_SENSOR_CFG_DEFAULT  != *parameter)
            && (RD_SENSOR_CFG_MIN   != *parameter)
            && (RD_SENSOR_CFG_MAX   != *parameter)) ||
            (RD_SENSOR_DSP_LAST  != *dsp))
    {
        return RD_ERROR_NOT_SUPPORTED;
    }

    return RD_SUCCESS;
}

rd_status_t ri_scd41_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    if ( (NULL == dsp) || (NULL == parameter))
    {
        return RD_ERROR_NULL;
    }

    // Only default is available
    *dsp       = RD_SENSOR_CFG_DEFAULT;
    *parameter = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

static rd_status_t ri_scd41_read_measurements (void)
{
    uint16_t co2 = 0;
    int32_t temperature = 0;
    int32_t humidity = 0;
    rd_status_t err_code = SCD4X_TO_RUUVI_ERROR (scd4x_read_measurement (
                               &co2,
                               &temperature,
                               &humidity));

    if (RD_SUCCESS == err_code)
    {
        m_co2 = co2;
        m_ambient_temperature = temperature;
        m_ambient_humidity = humidity;
        char log_buf[SCD41_LOG_BUF_SIZE_MEASUREMENTS];
        (void) snprintf (log_buf, sizeof (log_buf),
                         "scd4x: CO2=%d, H=%d, T=%d\n",
                         (scd5x_printf_int_t) co2,
                         (scd5x_printf_int_t) humidity,
                         (scd5x_printf_int_t) temperature);
        ri_log (RI_LOG_LEVEL_INFO, log_buf);
    }

    return err_code;
}

static rd_status_t ri_scd41_mode_set_sleep (uint8_t * const mode)
{
    m_autorefresh = false;
    *mode = RD_SENSOR_CFG_SLEEP;
    rd_status_t err_code = SCD4X_TO_RUUVI_ERROR (scd4x_stop_periodic_measurement());

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    ri_delay_ms (SCD41_DELAY_AFTER_STOP_PERIODIC_MEASUREMENTS_MS);
    err_code = SCD4X_TO_RUUVI_ERROR (scd4x_power_down());

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    return RD_SUCCESS;
}

static rd_status_t ri_scd41_mode_set_single (uint8_t * const mode)
{
    // Do nothing if sensor is in continuous mode
    uint8_t current_mode = 0;
    ri_scd41_mode_get (&current_mode);

    if (RD_SENSOR_CFG_CONTINUOUS == current_mode)
    {
        *mode = RD_SENSOR_CFG_CONTINUOUS;
        return RD_ERROR_INVALID_STATE;
    }

    // Enter sleep after measurement
    m_autorefresh = false;
    *mode = RD_SENSOR_CFG_SLEEP;
    /*
     * Note that the first reading obtained using measure_single_shot (chapter 3.10.1)
     * after waking up the sensor should be discarded.
     */
    rd_status_t err_code = SCD4X_TO_RUUVI_ERROR (scd4x_wake_up());

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    ri_delay_ms (SCD41_DELAY_AFTER_WAKE_UP_MS);
    err_code = SCD4X_TO_RUUVI_ERROR (scd4x_measure_single_shot());

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    ri_delay_ms (SCD41_DELAY_MEASUREMENT_READY_MS);
    bool data_ready = false;

    for (int i = 0; i < 5; ++i)
    {
        err_code = SCD4X_TO_RUUVI_ERROR (scd4x_get_data_ready_flag (&data_ready));

        if (RD_SUCCESS != err_code)
        {
            return err_code;
        }

        if (data_ready)
        {
            break;
        }

        ri_delay_ms (SCD41_DELAY_BETWEEN_CHECKING_READY_FLAG_MS);
    }

    if (!data_ready)
    {
        return RD_ERROR_INTERNAL;
    }

    uint16_t co2 = 0;
    int32_t temperature = 0;
    int32_t humidity = 0;
    err_code = SCD4X_TO_RUUVI_ERROR (scd4x_read_measurement (&co2, &temperature, &humidity));

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    err_code = SCD4X_TO_RUUVI_ERROR (scd4x_measure_single_shot());

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    ri_delay_ms (SCD41_DELAY_MEASUREMENT_READY_MS);
    data_ready = false;

    for (int i = 0; i < 5; ++i)
    {
        err_code = SCD4X_TO_RUUVI_ERROR (scd4x_get_data_ready_flag (&data_ready));

        if (RD_SUCCESS != err_code)
        {
            return err_code;
        }

        if (data_ready)
        {
            break;
        }

        ri_delay_ms (SCD41_DELAY_BETWEEN_CHECKING_READY_FLAG_MS);
    }

    if (!data_ready)
    {
        return RD_ERROR_INTERNAL;
    }

    m_tsample = rd_sensor_timestamp_get();
    err_code = ri_scd41_read_measurements();

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    err_code = SCD4X_TO_RUUVI_ERROR (scd4x_power_down());

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    return RD_SUCCESS;
}

// Start single on command, mark auto refresh with continuous
rd_status_t ri_scd41_mode_set (uint8_t * mode)
{
    if (NULL == mode)
    {
        return RD_ERROR_NULL;
    }

    // Enter sleep by default and by explicit sleep command
    if ( (RD_SENSOR_CFG_SLEEP == *mode) || (RD_SENSOR_CFG_DEFAULT == *mode))
    {
        return ri_scd41_mode_set_sleep (mode);
    }

    if (RD_SENSOR_CFG_SINGLE == *mode)
    {
        return ri_scd41_mode_set_single (mode);
    }

    if (RD_SENSOR_CFG_CONTINUOUS == *mode)
    {
        m_autorefresh = true;
        rd_status_t err_code = SCD4X_TO_RUUVI_ERROR (scd4x_wake_up());

        if (RD_SUCCESS != err_code)
        {
            return err_code;
        }

        ri_delay_ms (SCD41_DELAY_AFTER_WAKE_UP_MS);
        return SCD4X_TO_RUUVI_ERROR (scd4x_start_periodic_measurement());
    }

    return RD_ERROR_INVALID_PARAM;
}

rd_status_t ri_scd41_mode_get (uint8_t * mode)
{
    if (NULL == mode) { return RD_ERROR_NULL; }

    if (m_autorefresh)
    {
        *mode = RD_SENSOR_CFG_CONTINUOUS;
    }

    if (!m_autorefresh)
    {
        *mode = RD_SENSOR_CFG_SLEEP;
    }

    return RD_SUCCESS;
}

static rd_status_t read_mea_in_autorefresh_mode (void)
{
    bool data_ready = false;
    rd_status_t err_code = SCD4X_TO_RUUVI_ERROR (scd4x_get_data_ready_flag (&data_ready));

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    if (!data_ready)
    {
        return RD_SUCCESS;
    }

    err_code = ri_scd41_read_measurements();

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    m_tsample = rd_sensor_timestamp_get();
    return RD_SUCCESS;
}

static void ri_scd41_data_update (rd_sensor_data_t * const p_data)
{
    rd_sensor_data_t d_environmental = {0};
    rd_sensor_data_fields_t env_fields = {.bitfield = 0};
    scd4x_float_t env_values[SCD4X_NUM_MEASUREMENTS] =
    {
        [SCD4X_MEASUREMENT_IDX_CO2] = (scd4x_float_t) m_co2,
        [SCD4X_MEASUREMENT_IDX_HUMIDITY] = (scd4x_float_t) m_ambient_humidity / SCD41_SCALE_FACTOR_HUMIDITY,
        [SCD4X_MEASUREMENT_IDX_TEMPERATURE] = (scd4x_float_t) m_ambient_temperature / SCD41_SCALE_FACTOR_TEMPERATURE,
    };
    env_fields.datas.co2_ppm = 1;
    env_fields.datas.humidity_rh = 1;
    env_fields.datas.temperature_c = 1;
    d_environmental.data = env_values;
    d_environmental.valid  = env_fields;
    d_environmental.fields = env_fields;
    d_environmental.timestamp_ms = m_tsample;
    rd_sensor_data_populate (p_data,
                             &d_environmental,
                             p_data->fields);
}

rd_status_t ri_scd41_data_get (rd_sensor_data_t * const p_data)
{
    if (NULL == p_data)
    {
        return RD_ERROR_NULL;
    }

    if (m_autorefresh)
    {
        const rd_status_t err_code = read_mea_in_autorefresh_mode();

        if (RD_SUCCESS != err_code)
        {
            return err_code;
        }
    }

    if (RD_UINT64_INVALID != m_tsample)
    {
        ri_scd41_data_update (p_data);
    }

    return RD_SUCCESS;
}

/** @} */

#endif // RI_SCD4X_ENABLED || DOXYGEN
