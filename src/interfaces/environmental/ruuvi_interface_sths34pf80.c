/**
 * @file ruuvi_interface_sths34pf80.c
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 * @author Ruuvi Innovations Ltd
 * @date 2025-12-22
 *
 * STHS34PF80 thermal infrared presence/motion sensor driver.
 */

#include "ruuvi_driver_enabled_modules.h"
#if RI_STHS34PF80_ENABLED || DOXYGEN
// Ruuvi headers
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_sths34pf80.h"
#include "ruuvi_interface_environmental.h"
#include "ruuvi_interface_i2c.h"
#include "ruuvi_interface_i2c_sths34pf80.h"
#include "ruuvi_interface_yield.h"

// ST driver
#include "sths34pf80_reg.h"

#include <string.h>

/**
 * @addtogroup STHS34PF80
 */
/** @{ */

#define SHTS_DEBUG_DATA_IN_ACCELERATION (1U) //!< Enable to log raw data in acceleration format for easier debugging.
#define SHTS_DATA_FIELD_COUNT (4U + (4 * SHTS_DEBUG_DATA_IN_ACCELERATION)) //!< Temperature, presence, motion, IR object + debug fields

// Data field array indices
// NOTE: These indices are relative to the enabled fields in the bitfield.
// The rd_sensor_data_populate function counts set bits to determine array placement.
// When debug is enabled, acceleration fields (bits 0,1,2) shift all other indices by +3.
#if SHTS_DEBUG_DATA_IN_ACCELERATION
#define STHS34PF80_DEBUG_TOBJECT         (0)  //!< Debug: IR object raw (as accel_x)
#define STHS34PF80_DEBUG_TMOTION         (1)  //!< Debug: Motion algo (as accel_y)
#define STHS34PF80_DEBUG_TPRESENCE       (2)  //!< Debug: Presence algo/tamb_shock (as accel_z)
#define STHS34PF80_TAMBIENT_C            (3)  //!< Ambient temperature in Celsius
#define STHS34PF80_PRESENCE_FLAG         (4)  //!< Presence detection flag
#define STHS34PF80_MOTION_FLAG           (5)  //!< Motion detection flag
#define STHS34PF80_TOBJECT_RAW           (6)  //!< IR object signal (dimensionless)
#define STHS34PF80_DEBUG_TAMB_SHOCK      (7)  //!< Debug: Ambient shock value
#else
#define STHS34PF80_TAMBIENT_C            (0)  //!< Ambient temperature in Celsius
#define STHS34PF80_PRESENCE_FLAG         (1)  //!< Presence detection flag
#define STHS34PF80_MOTION_FLAG           (2)  //!< Motion detection flag
#define STHS34PF80_TOBJECT_RAW           (3)  //!< IR object signal (dimensionless)
#endif

#define BOOT_DELAY_MS       (10U)  //!< Delay after boot/reset in milliseconds.

/** @brief Macro for checking that sensor is in sleep mode before configuration */
#define VERIFY_SENSOR_SLEEPS() do { \
          uint8_t MACRO_MODE = 0; \
          rd_status_t MACRO_ERR = ri_sths34pf80_mode_get(&MACRO_MODE); \
          if(RD_SUCCESS != MACRO_ERR) { return MACRO_ERR; } \
          if(RD_SENSOR_CFG_SLEEP != MACRO_MODE) { return RD_ERROR_INVALID_STATE; } \
          } while(0)

/** @brief Macro for checking "ignored" parameters NO_CHANGE, MIN, MAX, DEFAULT */
#define RETURN_SUCCESS_ON_VALID(param) do {\
            if(RD_SENSOR_CFG_DEFAULT   == param ||\
               RD_SENSOR_CFG_MIN       == param ||\
               RD_SENSOR_CFG_MAX       == param ||\
               RD_SENSOR_CFG_NO_CHANGE == param   \
             ) return RD_SUCCESS;\
           } while(0)

/**
 * @brief STHS34PF80 sensor context structure.
 */
typedef struct
{
    stmdev_ctx_t ctx;             //!< ST driver control structure.
    uint8_t handle;               //!< Device handle (I2C address).
    sths34pf80_odr_t odr;         //!< Current ODR setting.
    uint8_t mode;                 //!< Operating mode: sleep, single, continuous.
    bool autorefresh;             //!< Flag to refresh data on data_get.
    uint64_t tsample;             //!< Timestamp of last sample.
    int16_t tambient_raw;         //!< Last ambient temperature raw value.
    int16_t tobject_raw;          //!< Last IR object signal raw value.
    uint8_t presence_flag;        //!< Last presence flag from FUNC_STATUS.
    uint8_t motion_flag;          //!< Last motion flag from FUNC_STATUS.
    uint8_t tamb_shock_flag;      //!< Last ambient shock flag from FUNC_STATUS.
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    int16_t tpresence_raw;        //!< Last presence algorithm value (debug).
    int16_t tmotion_raw;          //!< Last motion algorithm value (debug).
#endif
} ri_sths34pf80_ctx_t;

static ri_sths34pf80_ctx_t m_ctx = {0};  //!< Sensor context singleton.
static bool m_is_init = false;           //!< Initialization flag.
static const char m_sensor_name[] = "STHS34PF80"; //!< Human-readable sensor name.

/**
 * @brief Convert ST driver error to Ruuvi error code.
 *
 * The ST driver passes through platform I2C errors, which are Ruuvi error codes.
 * This function handles both ST driver internal errors (negative) and
 * platform errors (positive, matching RD_ERROR_* codes).
 *
 * @param[in] st_err ST driver return value (0 = success, other = error).
 * @return Ruuvi error code.
 */
static rd_status_t st_to_ruuvi_error (const int32_t st_err)
{
    if (0 == st_err)
    {
        return RD_SUCCESS;
    }

    // Negative values are ST driver internal errors
    if (st_err < 0)
    {
        return RD_ERROR_INTERNAL;
    }

    // Positive values are platform errors passed through from I2C adapter.
    // These are already Ruuvi error codes (RD_ERROR_*), so return directly.
    // Common cases:
    // - RD_ERROR_NOT_ACKNOWLEDGED: Device not responding (wrong address or not present)
    // - RD_ERROR_TIMEOUT: I2C bus timeout
    // - RD_ERROR_BUSY: I2C bus busy
    // - RD_ERROR_NULL: NULL pointer passed to I2C function
    return (rd_status_t) st_err;
}

/**
 * @brief Read single sample from sensor.
 *
 * Reads ambient temp, object IR signal, and presence/motion flags.
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_INTERNAL on driver error.
 */
static rd_status_t read_sample (void)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t st_err = 0;
    sths34pf80_func_status_t func_status = {0};
    // Read FUNC_STATUS to get presence/motion flags
    st_err = sths34pf80_func_status_get (&m_ctx.ctx, &func_status);
    err_code |= st_to_ruuvi_error (st_err);
    // Read ambient and object temperatures
    st_err = sths34pf80_tambient_raw_get (&m_ctx.ctx, &m_ctx.tambient_raw);
    err_code |= st_to_ruuvi_error (st_err);
    st_err = sths34pf80_tobject_raw_get (&m_ctx.ctx, &m_ctx.tobject_raw);
    err_code |= st_to_ruuvi_error (st_err);
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    // Read algorithm state values for debugging
    st_err = sths34pf80_tpresence_raw_get (&m_ctx.ctx, &m_ctx.tpresence_raw);
    err_code |= st_to_ruuvi_error (st_err);
    st_err = sths34pf80_tmotion_raw_get (&m_ctx.ctx, &m_ctx.tmotion_raw);
    err_code |= st_to_ruuvi_error (st_err);
#endif
    // Store flags
    m_ctx.presence_flag = func_status.pres_flag;
    m_ctx.motion_flag = func_status.mot_flag;
    m_ctx.tamb_shock_flag = func_status.tamb_shock_flag;
    // Record timestamp
    m_ctx.tsample = rd_sensor_timestamp_get();
    return err_code;
}

/**
 * @brief Convert raw ambient temperature to degrees Celsius.
 *
 * The STHS34PF80 ambient temperature has 100 LSB/°C sensitivity.
 *
 * @param[in] raw Raw ambient temperature value.
 * @return Temperature in degrees Celsius.
 */
static float tambient_to_celsius (const int16_t raw)
{
    // Sensitivity: 100 LSB/°C, offset varies per device but ~25°C at 0
    // Using simplified formula: T = raw / 100.0
    return (float) raw / 100.0f;
}

/**
 * @brief Delay function wrapper for ST driver.
 *
 * @param[in] millisec Milliseconds to delay.
 */
static void sths34pf80_mdelay (uint32_t millisec)
{
    ri_delay_ms (millisec);
}

rd_status_t ri_sths34pf80_init (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t st_err = 0;

    if (NULL == p_sensor)
    {
        return RD_ERROR_NULL;
    }

    if (m_is_init)
    {
        return RD_ERROR_INVALID_STATE;
    }

    // Only I2C is supported
    if (RD_BUS_I2C != bus)
    {
        return RD_ERROR_INVALID_PARAM;
    }

    rd_sensor_initialize (p_sensor);
    p_sensor->name = m_sensor_name;
    // Setup I2C context
    m_ctx.handle = handle;
    m_ctx.ctx.handle = &m_ctx.handle;
    m_ctx.ctx.write_reg = ri_i2c_sths34pf80_write;
    m_ctx.ctx.read_reg = ri_i2c_sths34pf80_read;
    m_ctx.ctx.mdelay = sths34pf80_mdelay;
    // Check WHO_AM_I
    uint8_t whoami = 0;
    st_err = sths34pf80_device_id_get (&m_ctx.ctx, &whoami);
    err_code |= st_to_ruuvi_error (st_err);

    // If read failed, return the communication error
    if (RD_SUCCESS != err_code)
    {
        rd_sensor_uninitialize (p_sensor);
        return err_code;
    }

    // If read succeeded but ID doesn't match, sensor is wrong type
    if (STHS34PF80_ID != whoami)
    {
        rd_sensor_uninitialize (p_sensor);
        return RD_ERROR_NOT_FOUND;
    }

    // Reset device
    st_err = sths34pf80_boot_set (&m_ctx.ctx, 1);
    err_code |= st_to_ruuvi_error (st_err);
    ri_delay_ms (BOOT_DELAY_MS);
    // Configure default settings: ODR off (sleep mode), BDU enabled
    st_err = sths34pf80_odr_set (&m_ctx.ctx, STHS34PF80_ODR_OFF);
    err_code |= st_to_ruuvi_error (st_err);
    st_err = sths34pf80_block_data_update_set (&m_ctx.ctx, 1);
    err_code |= st_to_ruuvi_error (st_err);

    if (RD_SUCCESS != err_code)
    {
        rd_sensor_uninitialize (p_sensor);
        return err_code;
    }

    // Initialize context
    m_ctx.odr = STHS34PF80_ODR_OFF;
    m_ctx.mode = RD_SENSOR_CFG_SLEEP;
    m_ctx.autorefresh = false;
    m_ctx.tsample = RD_UINT64_INVALID;
    m_ctx.tambient_raw = 0;
    m_ctx.tobject_raw = 0;
    m_ctx.presence_flag = 0;
    m_ctx.motion_flag = 0;
    m_ctx.tamb_shock_flag = 0;
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    m_ctx.tpresence_raw = 0;
    m_ctx.tmotion_raw = 0;
#endif
    // Setup sensor struct
    p_sensor->init              = ri_sths34pf80_init;
    p_sensor->uninit            = ri_sths34pf80_uninit;
    p_sensor->samplerate_set    = ri_sths34pf80_samplerate_set;
    p_sensor->samplerate_get    = ri_sths34pf80_samplerate_get;
    p_sensor->resolution_set    = ri_sths34pf80_resolution_set;
    p_sensor->resolution_get    = ri_sths34pf80_resolution_get;
    p_sensor->scale_set         = ri_sths34pf80_scale_set;
    p_sensor->scale_get         = ri_sths34pf80_scale_get;
    p_sensor->dsp_set           = ri_sths34pf80_dsp_set;
    p_sensor->dsp_get           = ri_sths34pf80_dsp_get;
    p_sensor->mode_set          = ri_sths34pf80_mode_set;
    p_sensor->mode_get          = ri_sths34pf80_mode_get;
    p_sensor->data_get          = ri_sths34pf80_data_get;
    p_sensor->configuration_set = rd_sensor_configuration_set;
    p_sensor->configuration_get = rd_sensor_configuration_get;
    // Define provided data fields
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    p_sensor->provides.datas.acceleration_x_g = 1;  // Debug: tobject
    p_sensor->provides.datas.acceleration_y_g = 1;  // Debug: tmotion
    p_sensor->provides.datas.acceleration_z_g = 1;  // Debug: tpresence
    p_sensor->provides.datas.debug_tamb = 1;        // Debug: tamb_shock
#endif
    p_sensor->provides.datas.temperature_c = 1;
    p_sensor->provides.datas.presence = 1;
    p_sensor->provides.datas.motion = 1;
    p_sensor->provides.datas.ir_object = 1;
    m_is_init = true;
    return err_code;
}

rd_status_t ri_sths34pf80_uninit (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == p_sensor)
    {
        return RD_ERROR_NULL;
    }

    // Power down sensor
    int32_t st_err = sths34pf80_odr_set (&m_ctx.ctx, STHS34PF80_ODR_OFF);
    err_code |= st_to_ruuvi_error (st_err);
    // Clear sensor structure
    rd_sensor_uninitialize (p_sensor);
    // Clear context
    memset (&m_ctx, 0, sizeof (m_ctx));
    m_is_init = false;
    return err_code;
}

/**
 * @brief Get maximum ODR supported by current oversampling setting.
 *
 * The STHS34PF80 has ODR limits based on the avg_tmos (oversampling) setting:
 * - AVG_TMOS_2, 8, 32: max 30Hz
 * - AVG_TMOS_128: max 8Hz
 * - AVG_TMOS_256: max 4Hz
 * - AVG_TMOS_512: max 2Hz
 * - AVG_TMOS_1024: max 1Hz
 * - AVG_TMOS_2048: max 0.5Hz
 *
 * @param[out] p_max_odr Pointer to store maximum ODR enum value.
 * @param[out] p_max_rate Pointer to store maximum samplerate in Hz (0 for sub-Hz).
 * @return RD_SUCCESS on success, error code on failure.
 */
static rd_status_t get_max_odr_for_oversampling (sths34pf80_odr_t * const p_max_odr,
        uint8_t * const p_max_rate)
{
    rd_status_t err_code = RD_SUCCESS;
    sths34pf80_avg_tobject_num_t avg_tmos;
    int32_t st_err = sths34pf80_avg_tobject_num_get (&m_ctx.ctx, &avg_tmos);
    err_code = st_to_ruuvi_error (st_err);

    if (RD_SUCCESS != err_code)
    {
        return err_code;
    }

    switch (avg_tmos)
    {
        case STHS34PF80_AVG_TMOS_2:
        case STHS34PF80_AVG_TMOS_8:
        case STHS34PF80_AVG_TMOS_32:
        default:
            *p_max_odr = STHS34PF80_ODR_AT_30Hz;
            *p_max_rate = 30U;
            break;

        case STHS34PF80_AVG_TMOS_128:
            *p_max_odr = STHS34PF80_ODR_AT_8Hz;
            *p_max_rate = 8U;
            break;

        case STHS34PF80_AVG_TMOS_256:
            *p_max_odr = STHS34PF80_ODR_AT_4Hz;
            *p_max_rate = 4U;
            break;

        case STHS34PF80_AVG_TMOS_512:
            *p_max_odr = STHS34PF80_ODR_AT_2Hz;
            *p_max_rate = 2U;
            break;

        case STHS34PF80_AVG_TMOS_1024:
            *p_max_odr = STHS34PF80_ODR_AT_1Hz;
            *p_max_rate = 1U;
            break;

        case STHS34PF80_AVG_TMOS_2048:
            *p_max_odr = STHS34PF80_ODR_AT_0Hz50;
            *p_max_rate = RI_STHS34PF80_SAMPLERATE_0HZ50;
            break;
    }

    return RD_SUCCESS;
}

rd_status_t ri_sths34pf80_samplerate_set (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == samplerate)
    {
        return RD_ERROR_NULL;
    }

    VERIFY_SENSOR_SLEEPS();
    sths34pf80_odr_t odr = STHS34PF80_ODR_AT_1Hz;

    if (RD_SENSOR_CFG_NO_CHANGE == *samplerate)
    {
        return ri_sths34pf80_samplerate_get (samplerate);
    }
    else if (RI_STHS34PF80_SAMPLERATE_0HZ25 == *samplerate ||
             RD_SENSOR_CFG_MIN == *samplerate)
    {
        // 0.25 Hz - lowest power, 4 second period
        odr = STHS34PF80_ODR_AT_0Hz25;
        *samplerate = RI_STHS34PF80_SAMPLERATE_0HZ25;
    }
    else if (RI_STHS34PF80_SAMPLERATE_0HZ50 == *samplerate)
    {
        // 0.5 Hz - 2 second period
        odr = STHS34PF80_ODR_AT_0Hz50;
        *samplerate = RI_STHS34PF80_SAMPLERATE_0HZ50;
    }
    else if (RD_SENSOR_CFG_DEFAULT == *samplerate ||
             1U == *samplerate)
    {
        odr = STHS34PF80_ODR_AT_1Hz;
        *samplerate = 1;
    }
    else if (RD_SENSOR_CFG_MAX == *samplerate)
    {
        // Get maximum ODR based on current oversampling setting
        sths34pf80_odr_t max_odr;
        uint8_t max_rate;
        err_code = get_max_odr_for_oversampling (&max_odr, &max_rate);

        if (RD_SUCCESS != err_code)
        {
            return err_code;
        }

        odr = max_odr;
        *samplerate = max_rate;
    }
    else if (*samplerate <= 1U)
    {
        odr = STHS34PF80_ODR_AT_1Hz;
        *samplerate = 1;
    }
    else if (*samplerate <= 2U)
    {
        odr = STHS34PF80_ODR_AT_2Hz;
        *samplerate = 2;
    }
    else if (*samplerate <= 4U)
    {
        odr = STHS34PF80_ODR_AT_4Hz;
        *samplerate = 4;
    }
    else if (*samplerate <= 8U)
    {
        odr = STHS34PF80_ODR_AT_8Hz;
        *samplerate = 8;
    }
    else if (*samplerate <= 15U)
    {
        odr = STHS34PF80_ODR_AT_15Hz;
        *samplerate = 15;
    }
    else if (*samplerate <= 30U)
    {
        odr = STHS34PF80_ODR_AT_30Hz;
        *samplerate = 30;
    }
    else
    {
        *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED;
        err_code |= RD_ERROR_NOT_SUPPORTED;
    }

    // Check if requested ODR exceeds max allowed by current oversampling setting
    if (RD_SUCCESS == err_code)
    {
        sths34pf80_odr_t max_odr;
        uint8_t max_rate;
        rd_status_t max_err = get_max_odr_for_oversampling (&max_odr, &max_rate);

        if (RD_SUCCESS == max_err)
        {
            if (odr > max_odr)
            {
                // Cannot achieve requested rate with current oversampling
                *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED;
                err_code |= RD_ERROR_NOT_SUPPORTED;
            }
        }
        else
        {
            err_code |= max_err;
        }
    }

    // Note: ODR is only applied when mode is set to continuous
    if (RD_SUCCESS == err_code)
    {
        m_ctx.odr = odr;
    }

    return err_code;
}

rd_status_t ri_sths34pf80_samplerate_get (uint8_t * samplerate)
{
    if (NULL == samplerate)
    {
        return RD_ERROR_NULL;
    }

    switch (m_ctx.odr)
    {
        case STHS34PF80_ODR_OFF:
            // When sensor is off, report 1 Hz as default rate
            *samplerate = 1;
            break;

        case STHS34PF80_ODR_AT_0Hz25:
            *samplerate = RI_STHS34PF80_SAMPLERATE_0HZ25;
            break;

        case STHS34PF80_ODR_AT_0Hz50:
            *samplerate = RI_STHS34PF80_SAMPLERATE_0HZ50;
            break;

        case STHS34PF80_ODR_AT_1Hz:
            *samplerate = 1;
            break;

        case STHS34PF80_ODR_AT_2Hz:
            *samplerate = 2;
            break;

        case STHS34PF80_ODR_AT_4Hz:
            *samplerate = 4;
            break;

        case STHS34PF80_ODR_AT_8Hz:
            *samplerate = 8;
            break;

        case STHS34PF80_ODR_AT_15Hz:
            *samplerate = 15;
            break;

        case STHS34PF80_ODR_AT_30Hz:
            *samplerate = 30;
            break;

        default:
            *samplerate = 1;
            break;
    }

    return RD_SUCCESS;
}

rd_status_t ri_sths34pf80_resolution_set (uint8_t * resolution)
{
    if (NULL == resolution)
    {
        return RD_ERROR_NULL;
    }

    VERIFY_SENSOR_SLEEPS();
    uint8_t original = *resolution;
    *resolution = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID (original);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_sths34pf80_resolution_get (uint8_t * resolution)
{
    if (NULL == resolution)
    {
        return RD_ERROR_NULL;
    }

    *resolution = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_sths34pf80_scale_set (uint8_t * scale)
{
    if (NULL == scale)
    {
        return RD_ERROR_NULL;
    }

    VERIFY_SENSOR_SLEEPS();
    uint8_t original = *scale;
    *scale = RD_SENSOR_CFG_DEFAULT;
    RETURN_SUCCESS_ON_VALID (original);
    return RD_ERROR_NOT_SUPPORTED;
}

rd_status_t ri_sths34pf80_scale_get (uint8_t * scale)
{
    if (NULL == scale)
    {
        return RD_ERROR_NULL;
    }

    *scale = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_sths34pf80_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter)
    {
        return RD_ERROR_NULL;
    }

    VERIFY_SENSOR_SLEEPS();

    // For now, only accept default
    if (RD_SENSOR_CFG_DEFAULT != *parameter &&
            RD_SENSOR_CFG_MIN != *parameter &&
            RD_SENSOR_CFG_MAX != *parameter)
    {
        return RD_ERROR_NOT_SUPPORTED;
    }

    *dsp = RD_SENSOR_CFG_DEFAULT;
    *parameter = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_sths34pf80_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    if (NULL == dsp || NULL == parameter)
    {
        return RD_ERROR_NULL;
    }

    *dsp = RD_SENSOR_CFG_DEFAULT;
    *parameter = RD_SENSOR_CFG_DEFAULT;
    return RD_SUCCESS;
}

rd_status_t ri_sths34pf80_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t st_err = 0;

    if (NULL == mode)
    {
        return RD_ERROR_NULL;
    }

    if (RD_SENSOR_CFG_SLEEP == *mode || RD_SENSOR_CFG_DEFAULT == *mode)
    {
        st_err = sths34pf80_odr_set (&m_ctx.ctx, STHS34PF80_ODR_OFF);
        err_code |= st_to_ruuvi_error (st_err);
        m_ctx.mode = RD_SENSOR_CFG_SLEEP;
        m_ctx.autorefresh = false;
        *mode = RD_SENSOR_CFG_SLEEP;
    }
    else if (RD_SENSOR_CFG_SINGLE == *mode)
    {
        // If in continuous mode, cannot do single
        if (RD_SENSOR_CFG_CONTINUOUS == m_ctx.mode)
        {
            *mode = RD_SENSOR_CFG_CONTINUOUS;
            return RD_ERROR_INVALID_STATE;
        }

        // Trigger one-shot measurement using max sample rate for fastest response
        uint8_t samplerate = RD_SENSOR_CFG_MAX;
        err_code = ri_sths34pf80_samplerate_set (&samplerate);

        if (RD_SUCCESS == err_code)
        {
            // Apply ODR to hardware
            st_err = sths34pf80_odr_set (&m_ctx.ctx, m_ctx.odr);
            err_code |= st_to_ruuvi_error (st_err);
        }

        if (RD_SUCCESS == err_code)
        {
            // Wait for sample based on configured rate (add margin)
            uint32_t delay_ms = (1000U / samplerate) + 10U;
            ri_delay_ms (delay_ms);
            // Read data
            err_code |= read_sample();
        }

        // Return to sleep
        st_err = sths34pf80_odr_set (&m_ctx.ctx, STHS34PF80_ODR_OFF);
        err_code |= st_to_ruuvi_error (st_err);
        m_ctx.mode = RD_SENSOR_CFG_SLEEP;
        *mode = RD_SENSOR_CFG_SLEEP;
    }
    else if (RD_SENSOR_CFG_CONTINUOUS == *mode)
    {
        // Use default samplerate if ODR was never configured
        if (STHS34PF80_ODR_OFF == m_ctx.odr)
        {
            uint8_t samplerate = RD_SENSOR_CFG_DEFAULT;
            err_code = ri_sths34pf80_samplerate_set (&samplerate);
        }

        if (RD_SUCCESS == err_code)
        {
            st_err = sths34pf80_odr_set (&m_ctx.ctx, m_ctx.odr);
            err_code |= st_to_ruuvi_error (st_err);
        }

        if (RD_SUCCESS == err_code)
        {
            m_ctx.mode = RD_SENSOR_CFG_CONTINUOUS;
            m_ctx.autorefresh = true;
            *mode = RD_SENSOR_CFG_CONTINUOUS;
        }
    }
    else
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }

    return err_code;
}

rd_status_t ri_sths34pf80_mode_get (uint8_t * mode)
{
    if (NULL == mode)
    {
        return RD_ERROR_NULL;
    }

    *mode = m_ctx.mode;
    return RD_SUCCESS;
}

rd_status_t ri_sths34pf80_data_get (rd_sensor_data_t * const data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == data)
    {
        return RD_ERROR_NULL;
    }

    // If in continuous mode, check DRDY and read only if new data is available
    if (m_ctx.autorefresh)
    {
        sths34pf80_drdy_status_t drdy_status = {0};
        int32_t st_err = sths34pf80_drdy_status_get (&m_ctx.ctx, &drdy_status);
        err_code |= st_to_ruuvi_error (st_err);

        if ( (RD_SUCCESS == err_code) && (drdy_status.drdy))
        {
            err_code |= read_sample();
        }

        // If no new data, we return the cached values with their original timestamp
    }

    // Populate data structure
    // Note: Only fields that are requested AND provided will be populated
    rd_sensor_data_t d_environmental = {0};
    rd_sensor_data_fields_t env_fields = {0};
    // Populate data array using indexed constants
    float values[SHTS_DATA_FIELD_COUNT] = {0};
    values[STHS34PF80_TAMBIENT_C] = tambient_to_celsius (m_ctx.tambient_raw);
    values[STHS34PF80_PRESENCE_FLAG] = (float) m_ctx.presence_flag;
    values[STHS34PF80_MOTION_FLAG] = (float) m_ctx.motion_flag;
    values[STHS34PF80_TOBJECT_RAW] = (float) m_ctx.tobject_raw;
#if SHTS_DEBUG_DATA_IN_ACCELERATION
    // Debug: Algorithm state values in acceleration format for easier plotting
    values[STHS34PF80_DEBUG_TOBJECT] = (float) m_ctx.tobject_raw / 1000.0f;
    values[STHS34PF80_DEBUG_TMOTION] = (float) m_ctx.tmotion_raw / 1000.0f;
    values[STHS34PF80_DEBUG_TPRESENCE] = (float) m_ctx.tpresence_raw / 1000.0f;
    values[STHS34PF80_DEBUG_TAMB_SHOCK] = (float) m_ctx.tamb_shock_flag;
#endif

    // Only mark fields valid if a sample has actually been taken
    if (RD_UINT64_INVALID != m_ctx.tsample)
    {
#if SHTS_DEBUG_DATA_IN_ACCELERATION
        env_fields.datas.acceleration_x_g = 1;  // Debug: tobject
        env_fields.datas.acceleration_y_g = 1;  // Debug: tmotion
        env_fields.datas.acceleration_z_g = 1;  // Debug: tpresence
        env_fields.datas.debug_tamb = 1;        // Debug: tamb_shock
#endif
        env_fields.datas.temperature_c = 1;
        env_fields.datas.presence = 1;
        env_fields.datas.motion = 1;
        env_fields.datas.ir_object = 1;
    }

    d_environmental.data = values;
    d_environmental.valid = env_fields;
    d_environmental.fields = env_fields;
    d_environmental.timestamp_ms = m_ctx.tsample;
    rd_sensor_data_populate (data, &d_environmental, data->fields);
    data->timestamp_ms = m_ctx.tsample;
    return err_code;
}

/** @} */
#endif // RI_STHS34PF80_ENABLED
