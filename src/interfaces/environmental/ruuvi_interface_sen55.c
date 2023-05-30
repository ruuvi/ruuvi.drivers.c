/**
 * @file ruuvi_interface_sen55.c
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @author TheSomeMan
 * @date 2023-05-21
 *
 * Sensirion SEN55 (Environmental Sensor Node for HVAC and Air Quality Applications) driver.
 */

#include "ruuvi_driver_enabled_modules.h"
#if RI_SEN5X_ENABLED || DOXYGEN
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
// Ruuvi headers
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_sen55.h"
#include "ruuvi_interface_environmental.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_rtc.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_interface_log.h"

/**
 * @addtogroup SEN55
 */
/** @{ */

// Sensirion driver.
#include "sen5x_i2c.h"
#include "sensirion_i2c_hal.h"

#define SEN5X_BUF_SIZE_PRODUCT_NAME  (32U)
#define SEN5X_BUF_SIZE_SERIAL_NUMBER (32U)
#define SEN5X_BUF_SIZE_FIRMWARE_INFO (100U)
#define SEN5X_BUF_SIZE_MEASUREMENTS  (120U)

#define SEN5X_DELAY_AFTER_DEVICE_RESET_MS  (50U)

#define SEN55_PROBE_RETRIES_MAX (5U)

#define SEN5X_NUM_MEASUREMENTS            (8U)

#define SEN5X_MEASUREMENT_IDX_HUMIDITY    (0U)
#define SEN5X_MEASUREMENT_IDX_PM1_0       (1U)
#define SEN5X_MEASUREMENT_IDX_PM2_5       (2U)
#define SEN5X_MEASUREMENT_IDX_PM4_0       (3U)
#define SEN5X_MEASUREMENT_IDX_PM10_0      (4U)
#define SEN5X_MEASUREMENT_IDX_TEMPERATURE (5U)
#define SEN5X_MEASUREMENT_IDX_VOC_INDEX   (6U)
#define SEN5X_MEASUREMENT_IDX_NOX_INDEX   (7U)

#define SEN5X_SCALE_FACTOR_HUMIDITY       (100.0f)
#define SEN5X_SCALE_FACTOR_PM             (10.0f)
#define SEN5X_SCALE_FACTOR_TEMPERATURE    (200.0f)
#define SEN5X_SCALE_FACTOR_VOC_INDEX      (10.0f)
#define SEN5X_SCALE_FACTOR_NOX_INDEX      (10.0f)

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
    ri_sen55_mode_get (&mode);

    if (RD_SENSOR_CFG_SLEEP == mode)
    {
        return true;
    }

    return false;
}

static uint64_t m_tsample; //!< Timestamp of sample.
static bool m_autorefresh; //!< Flag to refresh data on data_get.
static int32_t m_mass_concentration_pm1p0; //!< Last measured mass concentration PM1.0.
static int32_t m_mass_concentration_pm2p5; //!< Last measured mass concentration PM2.5.
static int32_t m_mass_concentration_pm4p0; //!< Last measured mass concentration PM4.
static int32_t m_mass_concentration_pm10p0; //!< Last measured mass concentration PM10.
static int32_t m_ambient_humidity; //!< Last measured humidity
static int32_t m_ambient_temperature; //!< Last measured temperature
static int32_t m_voc_index; //!< Last measured VOC
static int32_t m_nox_index; //!< Last measured NOx
static bool m_is_init; //!< Flag, is sensor init.
static const char m_sensor_name[] = "SEN55"; //!< Human-readable name of the sensor.

typedef enum sen55_status_e
{
    STATUS_OK = 0,         //!< SEN5X driver ok
    STATUS_UNKNOWN_DEVICE, //!< Invalid WHOAMI
    STATUS_ERR_BAD_DATA,   //!< Bad data
} sen55_status_e;

/**
 * @brief Convert error from SEN5X driver to appropriate NRF ERROR
 *
 * @param[in] rslt error code from SEN5X driver
 * @return    Ruuvi error code corresponding to SEN5X error code
 */
static rd_status_t SEN5X_TO_RUUVI_ERROR (const sen55_status_e status)
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

static rd_status_t ri_sen55_get_product_name (void)
{
    unsigned char product_name[SEN5X_BUF_SIZE_PRODUCT_NAME];
    rd_status_t err_code = SEN5X_TO_RUUVI_ERROR (sen5x_get_product_name (product_name,
                           sizeof (product_name)));

    if (RD_SUCCESS == err_code)
    {
        ri_log (RI_LOG_LEVEL_INFO, "sen5x: device name: ");
        ri_log (RI_LOG_LEVEL_INFO, (const char *) product_name);
        ri_log (RI_LOG_LEVEL_INFO, "\n");
    }
    else
    {
        ri_log (RI_LOG_LEVEL_ERROR, "sen5x: sen5x_get_product_name: error\n");
    }

    return err_code;
}

static rd_status_t ri_sen55_get_serial_number (void)
{
    unsigned char serial_number[SEN5X_BUF_SIZE_SERIAL_NUMBER];
    rd_status_t err_code = SEN5X_TO_RUUVI_ERROR (sen5x_get_serial_number (serial_number,
                           sizeof (serial_number)));

    if (RD_SUCCESS == err_code)
    {
        ri_log (RI_LOG_LEVEL_INFO, "sen5x: serial number: ");
        ri_log (RI_LOG_LEVEL_INFO, (const char *) serial_number);
        ri_log (RI_LOG_LEVEL_INFO, "\n");
    }
    else
    {
        ri_log (RI_LOG_LEVEL_ERROR, "sen5x: sen5x_get_serial_number: error\n");
    }

    return err_code;
}

static rd_status_t ri_sen55_get_version (void)
{
    uint8_t firmware_major = 0;
    uint8_t firmware_minor = 0;
    bool firmware_debug = 0;
    uint8_t hardware_major = 0;
    uint8_t hardware_minor = 0;
    uint8_t protocol_major = 0;
    uint8_t protocol_minor = 0;
    rd_status_t err_code = SEN5X_TO_RUUVI_ERROR (sen5x_get_version (&firmware_major,
                           &firmware_minor,
                           &firmware_debug,
                           &hardware_major,
                           &hardware_minor,
                           &protocol_major,
                           &protocol_minor));

    if (RD_SUCCESS == err_code)
    {
        char log_buf[SEN5X_BUF_SIZE_FIRMWARE_INFO];
        (void) snprintf (log_buf, sizeof (log_buf),
                         "sen5x: fw: %d.%d, hw: %d.%d, protocol: %d.%d\n",
                         firmware_major, firmware_minor,
                         hardware_major, hardware_minor,
                         protocol_major, protocol_minor);
        ri_log (RI_LOG_LEVEL_INFO, log_buf);
    }
    else
    {
        ri_log (RI_LOG_LEVEL_ERROR, "sen5x: sen5x_get_version: error\n");
    }

    return err_code;
}

static rd_status_t ri_sen55_reset (void)
{
    rd_status_t err_code = RD_SUCCESS;

    for (int32_t retries = 0; retries < SEN55_PROBE_RETRIES_MAX; ++retries)
    {
        err_code = SEN5X_TO_RUUVI_ERROR (sen5x_device_reset());
        ri_delay_ms (SEN5X_DELAY_AFTER_DEVICE_RESET_MS);

        if (RD_SUCCESS == err_code)
        {
            ri_log (RI_LOG_LEVEL_INFO, "sen5x: device_reset: OK\n");
            break;
        }

        ri_log (RI_LOG_LEVEL_ERROR, "sen5x: device_reset: error\n");
    }

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_sen55_get_product_name();
    }

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_sen55_get_serial_number();
    }

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_sen55_get_version();
    }

    return err_code;
}

rd_status_t ri_sen55_init (rd_sensor_t * sensor, rd_bus_t bus, uint8_t handle)
{
    (void) handle;
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == sensor)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if (rd_sensor_is_init (sensor))
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        rd_sensor_initialize (sensor);
        sensor->name = m_sensor_name;

        switch (bus)
        {
            case RD_BUS_I2C:
                err_code |= ri_sen55_reset();
                break;

            default:
                err_code |=  RD_ERROR_INVALID_PARAM;
        }

        if (RD_SUCCESS != err_code)
        {
            err_code = RD_ERROR_NOT_FOUND;
        }
        else
        {
            sensor->init              = ri_sen55_init;
            sensor->uninit            = ri_sen55_uninit;
            sensor->samplerate_set    = ri_sen55_samplerate_set;
            sensor->samplerate_get    = ri_sen55_samplerate_get;
            sensor->resolution_set    = ri_sen55_resolution_set;
            sensor->resolution_get    = ri_sen55_resolution_get;
            sensor->scale_set         = ri_sen55_scale_set;
            sensor->scale_get         = ri_sen55_scale_get;
            sensor->dsp_set           = ri_sen55_dsp_set;
            sensor->dsp_get           = ri_sen55_dsp_get;
            sensor->mode_set          = ri_sen55_mode_set;
            sensor->mode_get          = ri_sen55_mode_get;
            sensor->data_get          = ri_sen55_data_get;
            sensor->configuration_set = rd_sensor_configuration_set;
            sensor->configuration_get = rd_sensor_configuration_get;
            sensor->provides.datas.humidity_rh = 1;
            sensor->provides.datas.pm_1_ugm3 = 1;
            sensor->provides.datas.pm_2_ugm3 = 1;
            sensor->provides.datas.pm_4_ugm3 = 1;
            sensor->provides.datas.pm_10_ugm3 = 1;
            sensor->provides.datas.temperature_c = 1;
            sensor->provides.datas.voc_index = 1;
            sensor->provides.datas.nox_index = 1;
            err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_start_measurement());
            m_tsample = RD_UINT64_INVALID;
            m_is_init = true;
        }
    }

    return err_code;
}

rd_status_t ri_sen55_uninit (rd_sensor_t * sensor,
                             rd_bus_t bus, uint8_t handle)
{
    (void) bus;
    (void) handle;

    if (NULL == sensor) { return RD_ERROR_NULL; }

    rd_status_t err_code = RD_SUCCESS;
    err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_device_reset());
    rd_sensor_uninitialize (sensor);
    m_tsample = RD_UINT64_INVALID;
    m_mass_concentration_pm1p0 = RD_INT32_INVALID;
    m_mass_concentration_pm2p5 = RD_INT32_INVALID;
    m_mass_concentration_pm4p0 = RD_INT32_INVALID;
    m_mass_concentration_pm10p0 = RD_INT32_INVALID;
    m_ambient_humidity = RD_INT32_INVALID;
    m_ambient_temperature = RD_INT32_INVALID;
    m_voc_index = RD_INT32_INVALID;
    m_nox_index = RD_INT32_INVALID;
    m_is_init = false;
    m_autorefresh = false;
    return err_code;
}

rd_status_t ri_sen55_samplerate_set (uint8_t * samplerate)
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

rd_status_t ri_sen55_samplerate_get (uint8_t * samplerate)
{
    if (NULL == samplerate) { return RD_ERROR_NULL; }

    *samplerate = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_sen55_resolution_set (uint8_t * resolution)
{
    if (NULL == resolution) { return RD_ERROR_NULL; }

    if (!check_is_sensor_in_sleep_mode())
    {
        return RD_ERROR_INVALID_STATE;
    }

    uint8_t original = *resolution;
    *resolution = RD_SENSOR_CFG_DEFAULT;

    if (check_is_param_valid (original))
    {
        return RD_SUCCESS;
    }

    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_sen55_resolution_get (uint8_t * resolution)
{
    if (NULL == resolution) { return RD_ERROR_NULL; }

    *resolution = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_sen55_scale_set (uint8_t * scale)
{
    if (NULL == scale) { return RD_ERROR_NULL; }

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

rd_status_t ri_sen55_scale_get (uint8_t * scale)
{
    if (NULL == scale) { return RD_ERROR_NULL; }

    *scale = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_sen55_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter) { return RD_ERROR_NULL; }

    if (!check_is_sensor_in_sleep_mode())
    {
        return RD_ERROR_INVALID_STATE;
    }

    // Validate configuration
    if ( (RD_SENSOR_CFG_DEFAULT  != *parameter
            && RD_SENSOR_CFG_MIN   != *parameter
            && RD_SENSOR_CFG_MAX   != *parameter) ||
            (RD_SENSOR_DSP_LAST  != *dsp))
    {
        return RD_ERROR_NOT_SUPPORTED;
    }

    return RD_SUCCESS;
}

rd_status_t ri_sen55_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter) { return RD_ERROR_NULL; }

    // Only default is available
    *dsp       = RD_SENSOR_CFG_DEFAULT;
    *parameter = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

static rd_status_t ri_sen55_read_measurements (void)
{
    uint16_t mass_concentration_pm1p0 = 0;
    uint16_t mass_concentration_pm2p5 = 0;
    uint16_t mass_concentration_pm4p0 = 0;
    uint16_t mass_concentration_pm10p0 = 0;
    int16_t ambient_humidity = 0;
    int16_t ambient_temperature = 0;
    int16_t voc_index = 0;
    int16_t nox_index = 0;
    rd_status_t err_code = SEN5X_TO_RUUVI_ERROR (sen5x_read_measured_values (
                               &mass_concentration_pm1p0, &mass_concentration_pm2p5,
                               &mass_concentration_pm4p0, &mass_concentration_pm10p0,
                               &ambient_humidity, &ambient_temperature, &voc_index, &nox_index));

    if (RD_SUCCESS == err_code)
    {
        m_mass_concentration_pm1p0 = mass_concentration_pm1p0;
        m_mass_concentration_pm2p5 = mass_concentration_pm2p5;
        m_mass_concentration_pm4p0 = mass_concentration_pm4p0;
        m_mass_concentration_pm10p0 = mass_concentration_pm10p0;
        m_ambient_humidity = ambient_humidity;
        m_ambient_temperature = ambient_temperature;
        m_voc_index = voc_index;
        m_nox_index = nox_index;
        char log_buf[SEN5X_BUF_SIZE_MEASUREMENTS];
        (void) snprintf (log_buf, sizeof (log_buf),
                         "sen5x: PM1.0=%d, PM2.5=%d, PM4.0=%d, PM10.0=%d, H=%d, T=%d, VOC=%d, NOx=%d\n",
                         mass_concentration_pm1p0,
                         mass_concentration_pm2p5,
                         mass_concentration_pm4p0,
                         mass_concentration_pm10p0,
                         ambient_humidity,
                         ambient_temperature,
                         voc_index,
                         nox_index);
        ri_log (RI_LOG_LEVEL_INFO, log_buf);
    }

    return err_code;
}

// Start single on command, mark auto refresh with continuous
rd_status_t ri_sen55_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code |= RD_ERROR_NULL;
    }
    // Enter sleep by default and by explicit sleep command
    else if ( (RD_SENSOR_CFG_SLEEP == *mode) || (RD_SENSOR_CFG_DEFAULT == *mode))
    {
        m_autorefresh = false;
        *mode = RD_SENSOR_CFG_SLEEP;
        err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_stop_measurement());
    }
    else if (RD_SENSOR_CFG_SINGLE == *mode)
    {
        // Do nothing if sensor is in continuous mode
        uint8_t current_mode;
        ri_sen55_mode_get (&current_mode);

        if (RD_SENSOR_CFG_CONTINUOUS == current_mode)
        {
            *mode = RD_SENSOR_CFG_CONTINUOUS;
            return RD_ERROR_INVALID_STATE;
        }

        // Enter sleep after measurement
        m_autorefresh = false;
        *mode = RD_SENSOR_CFG_SLEEP;
        m_tsample = rd_sensor_timestamp_get();
        err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_start_measurement());

        if (RD_SUCCESS == err_code)
        {
            bool data_ready = false;

            while ( (RD_SUCCESS == err_code) && (!data_ready))
            {
                err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_read_data_ready (&data_ready));
            }
        }

        if (RD_SUCCESS == err_code)
        {
            err_code |= ri_sen55_read_measurements();
        }

        err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_stop_measurement());
        return err_code;
    }
    else if (RD_SENSOR_CFG_CONTINUOUS == *mode)
    {
        m_autorefresh = true;
        err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_start_measurement());
    }
    else
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }

    return err_code;
}

rd_status_t ri_sen55_mode_get (uint8_t * mode)
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

static rd_status_t ri_sen55_read_measurements_in_autorefresh_mode (void)
{
    rd_status_t err_code = RD_SUCCESS;
    bool data_ready = false;
    err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_read_data_ready (&data_ready));

    if (RD_SUCCESS == err_code)
    {
        if (data_ready)
        {
            err_code |= ri_sen55_read_measurements();

            if (RD_SUCCESS == err_code)
            {
                m_tsample = rd_sensor_timestamp_get();
            }
        }
    }

    return err_code;
}

static rd_status_t ri_sen55_read_measurements_in_single_shot_mode (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_start_measurement());
    bool data_ready = false;

    while ( (RD_SUCCESS == err_code) && (!data_ready))
    {
        err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_read_data_ready (&data_ready));
    }

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_sen55_read_measurements();
    }

    if (RD_SUCCESS == err_code)
    {
        m_tsample = rd_sensor_timestamp_get();
    }

    err_code |= SEN5X_TO_RUUVI_ERROR (sen5x_stop_measurement());
    return err_code;
}

static void ri_sen55_data_update (rd_sensor_data_t * const p_data)
{
    rd_sensor_data_t d_environmental = {0};
    rd_sensor_data_fields_t env_fields = {.bitfield = 0};
    float env_values[SEN5X_NUM_MEASUREMENTS] =
    {
        [SEN5X_MEASUREMENT_IDX_HUMIDITY] = (float) m_ambient_humidity / SEN5X_SCALE_FACTOR_HUMIDITY,
        [SEN5X_MEASUREMENT_IDX_PM1_0] = (float) m_mass_concentration_pm1p0 / SEN5X_SCALE_FACTOR_PM,
        [SEN5X_MEASUREMENT_IDX_PM2_5] = (float) m_mass_concentration_pm2p5 / SEN5X_SCALE_FACTOR_PM,
        [SEN5X_MEASUREMENT_IDX_PM4_0] = (float) m_mass_concentration_pm4p0 / SEN5X_SCALE_FACTOR_PM,
        [SEN5X_MEASUREMENT_IDX_PM10_0] = (float) m_mass_concentration_pm10p0 / SEN5X_SCALE_FACTOR_PM,
        [SEN5X_MEASUREMENT_IDX_TEMPERATURE] = (float) m_ambient_temperature / SEN5X_SCALE_FACTOR_TEMPERATURE,
        [SEN5X_MEASUREMENT_IDX_VOC_INDEX] = (float) m_voc_index / SEN5X_SCALE_FACTOR_VOC_INDEX,
        [SEN5X_MEASUREMENT_IDX_NOX_INDEX] = (float) m_nox_index / SEN5X_SCALE_FACTOR_NOX_INDEX,
    };
    env_fields.datas.humidity_rh = 1;
    env_fields.datas.pm_1_ugm3 = 1;
    env_fields.datas.pm_2_ugm3 = 1;
    env_fields.datas.pm_4_ugm3 = 1;
    env_fields.datas.pm_10_ugm3 = 1;
    env_fields.datas.temperature_c = 1;
    env_fields.datas.voc_index = 1;
    env_fields.datas.nox_index = 1;
    d_environmental.data = env_values;
    d_environmental.valid  = env_fields;
    d_environmental.fields = env_fields;
    d_environmental.timestamp_ms = m_tsample;
    rd_sensor_data_populate (p_data,
                             &d_environmental,
                             p_data->fields);
}

rd_status_t ri_sen55_data_get (rd_sensor_data_t * const p_data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == p_data)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        if (m_autorefresh)
        {
            err_code |= ri_sen55_read_measurements_in_autorefresh_mode();
        }
        else
        {
            err_code |= ri_sen55_read_measurements_in_single_shot_mode();
        }

        if ( (RD_SUCCESS == err_code) && (RD_UINT64_INVALID != m_tsample))
        {
            ri_sen55_data_update (p_data);
        }
    }

    return err_code;
}

/** @} */

#endif // RI_SEN5X_ENABLED || DOXYGEN
