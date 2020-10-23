#include "ruuvi_driver_enabled_modules.h"
#if (RI_LIS2DH12_ENABLED || DOXYGEN)
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_lis2dh12.h"
#include "ruuvi_interface_spi_lis2dh12.h"
#include "ruuvi_interface_yield.h"
#include "ruuvi_interface_log.h"
#include "lis2dh12_reg.h"


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
#ifndef RUUVI_NRF5_SDK15_LIS2GH12_LOG_LEVEL
#define RUUVI_NRF5_SDK15_LIS2GH12_LOG_LEVEL RI_LOG_LEVEL_DEBUG
#endif
#endif
/**
 * @addtogroup LIS2DH12
 */
/*@{*/
/**
 * @file ruuvi_interface_lis2dh12.c
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-10-11
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Implementation for LIS2DH12 basic usage. The implementation supports
 * different resolutions, samplerates, high-passing, activity interrupt
 * and FIFO.
 *
 * Requires STM lis2dh12 driver, available on GitHub under BSD-3 license.
 * Requires "application_config.h", will only get compiled if LIS2DH12_ACCELERATION is defined.
 * Requires floats enabled in application.
 *
 */

#define NUM_AXIS    (3U) //!< X, Y, Z.
#define NM_BIT_DIVEDER (64U) //!< Normal mode uses 10 bits in 16 bit field, leading to 2^6 factor in results.
#define MOTION_THRESHOLD_MAX (0x7FU) // Highest threshold value allowed.

#define ACC_X                   0           //!< Num of axis(X) in a row
#define ACC_Y                   1           //!< Num of axis(Y) in a row
#define ACC_Z                   2           //!< Num of axis(Z) in a row
#define ACC_XYZ_NUM             3           //!< Num of axises

#define ACC_SAMPLERATE_1HZ      (1U)        //!< Samplerate on 1Hz
#define ACC_SAMPLERATE_10HZ     (10U)       //!< Samplerate on 10Hz
#define ACC_SAMPLERATE_25HZ     (25U)       //!< Samplerate on 25Hz
#define ACC_SAMPLERATE_50HZ     (50U)       //!< Samplerate on 50Hz
#define ACC_SAMPLERATE_100HZ    (100U)      //!< Samplerate on 100Hz
#define ACC_SAMPLERATE_200HZ    (200U)      //!< Samplerate on 200Hz
#define ACC_SAMPLERATE_400HZ    (400U)      //!< Samplerate on 400Hz

#define ACC_RESOLUTION_8BITS    (8U)        //!< Data resolution - 8 bits
#define ACC_RESOLUTION_10BITS   (10U)       //!< Data resolution - 10 bits
#define ACC_RESOLUTION_12BITS   (12U)       //!< Data resolution - 12 bits

#define ACC_SCALE_2G            (2U)        //!< Scale - 2G
#define ACC_SCALE_4G            (4U)        //!< Scale - 4G
#define ACC_SCALE_8G            (8U)        //!< Scale - 8G
#define ACC_SCALE_16G           (16U)       //!< Scale - 16G

#define ACC_DSP_MODE_0          (0U)        //!< DSP mode 0 value
#define ACC_DSP_MODE_1          (1U)        //!< DSP mode 1 value
#define ACC_DSP_MODE_2          (2U)        //!< DSP mode 2 value
#define ACC_DSP_MODE_3          (3U)        //!< DSP mode 3 value

#define ACC_ODR_DELAY           7000        //!< Delay of ODR

#define ACC_G_TO_MG_DIVIDER     (1000.0f)   //!< Diveder from g to mg

#define ACC_SCALE_DIV_2G        (0.016f)    //!< Scale divider on 2G
#define ACC_SCALE_DIV_4G        (0.032f)    //!< Scale divider on 4G
#define ACC_SCALE_DIV_8G        (0.062f)    //!< Scale divider on 8G
#define ACC_SCALE_DIV_16G       (0.186f)    //!< Scale divider on 16G

#define ACC_FIFO_WATERMARK      31          //!< Fifo watermark

/** @brief Representation of 3*2 bytes buffer as 3*int16_t */
typedef union
{
    int16_t i16bit[NUM_AXIS]; //!< Integer values
    uint8_t u8bit[2 * NUM_AXIS];  //!< Buffer
} axis3bit16_t;

/** @brief Representation of 2 bytes buffer as int16_t */
typedef union
{
    int16_t i16bit;   //!< Integer value
    uint8_t u8bit[2]; //!< Buffer
} axis1bit16_t;

/**
 * @brief lis2dh12 sensor settings structure.
 */
#ifndef CEEDLING
static
#endif
ri_lis2dh12_dev dev = {0};

static const char m_acc_name[] = "LIS2DH12";

/** @brief Function for checking that sensor is in sleep mode before configuration */
static rd_status_t lis2dh12_verify_sensor_sleep (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = 0;
    ri_lis2dh12_mode_get (&mode);

    if (RD_SENSOR_CFG_SLEEP != mode)
    {
        err_code = RD_ERROR_INVALID_STATE;
    }

    return err_code;
}

#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
static void lis2dh12_log_debug (const char * fmt, ...)
{
    char buff[1024] = {0};
    snprintf (buff, sizeof (buff), ":%d:%s(): " fmt,
              __LINE__, __func__, ##__VA_ARGS__);
    ri_log (RUUVI_NRF5_SDK15_LIS2GH12_LOG_LEVEL, buff);
}
#endif

static void lis2dh12_get_samples_selftest (axis3bit16_t * p_data_raw_acceleration)
{
    axis3bit16_t data_raw_acceleration = {0};
    int32_t axis_x = 0;
    int32_t axis_y = 0;
    int32_t axis_z = 0;
    // Start delay
    ri_delay_ms (SELF_TEST_DELAY_MS);
    // Discard first sample
    lis2dh12_acceleration_raw_get (&dev.ctx, data_raw_acceleration.u8bit);

    // Obtain 5 no self test samples
    for (uint8_t i = 0; i < SELF_TEST_SAMPLES_NUM; i++)
    {
        lis2dh12_acceleration_raw_get (&dev.ctx, data_raw_acceleration.u8bit);
        axis_x += data_raw_acceleration.i16bit[ACC_X];
        axis_y += data_raw_acceleration.i16bit[ACC_Y];
        axis_z += data_raw_acceleration.i16bit[ACC_Z];
    }

    p_data_raw_acceleration->i16bit[ACC_X] = (int16_t) (axis_x / SELF_TEST_SAMPLES_NUM);
    p_data_raw_acceleration->i16bit[ACC_Y] = (int16_t) (axis_y / SELF_TEST_SAMPLES_NUM);
    p_data_raw_acceleration->i16bit[ACC_Z] = (int16_t) (axis_z / SELF_TEST_SAMPLES_NUM);
}

// Check that self-test values differ enough
static rd_status_t lis2dh12_verify_selftest (const axis3bit16_t * const new,
        const axis3bit16_t * const old)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (LIS2DH12_2g != dev.scale) || (LIS2DH12_NM_10bit != dev.resolution))
    {
        err_code |= RD_ERROR_INVALID_STATE;
    }
    else
    {
        // Calculate positive diffs of each axes and compare to expected change
        for (size_t ii = 0; ii < NUM_AXIS; ii++)
        {
            int16_t diff = new->i16bit[ii] - old->i16bit[ii];
            //Compensate justification, check absolute difference
            diff /= NM_BIT_DIVEDER;

            if (0 > diff) { diff = 0 - diff; }

            if (RI_LIS2DH12_SELFTEST_DIFF_MIN > diff)
            {
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
                lis2dh12_log_debug ("diff < min = %d\r\n", diff);
#endif
                err_code |= RD_ERROR_SELFTEST;
            }

            if (RI_LIS2DH12_SELFTEST_DIFF_MAX < diff)
            {
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
                lis2dh12_log_debug ("diff > max = %d\r\n", diff);
#endif
                err_code |= RD_ERROR_SELFTEST;
            }
        }
    }

    return err_code;
}

static rd_status_t dev_ctx_init (const rd_bus_t bus,
                                 const uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;

    switch (bus)
    {
        case RD_BUS_SPI:
            dev.ctx.write_reg = &ri_spi_lis2dh12_write;
            dev.ctx.read_reg = &ri_spi_lis2dh12_read;
            break;

        case RD_BUS_I2C:
            err_code |= RD_ERROR_NOT_IMPLEMENTED;
            break;

        default:
            err_code |= RD_ERROR_NOT_SUPPORTED;
            break;
    }

    dev.handle = handle;
    dev.ctx.handle = &dev.handle;
    dev.mode = RD_SENSOR_CFG_SLEEP;
    return err_code;
}

static rd_status_t check_whoami (void)
{
    rd_status_t err_code = RD_SUCCESS;
    uint8_t whoamI = 0;
    lis2dh12_device_id_get (&dev.ctx, &whoamI);

    if (whoamI != LIS2DH12_ID)
    {
        err_code |= RD_ERROR_NOT_FOUND;
    }

    return err_code;
}

static rd_status_t clear_sensor_state (void)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code = LIS_SUCCESS;
    rd_float ths = 0;
    // Disable FIFO, activity
    err_code |= ri_lis2dh12_fifo_use (false);
    err_code |= ri_lis2dh12_fifo_interrupt_use (false);
    err_code |= ri_lis2dh12_active_interrupt (false, &ths);
    // Enable temperature sensor
    lis_ret_code = lis2dh12_temperature_meas_set (&dev.ctx, LIS2DH12_TEMP_ENABLE);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }

    // Disable Block Data Update, allow values to update even if old is not read
    lis_ret_code = lis2dh12_block_data_update_set (&dev.ctx, PROPERTY_ENABLE);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }

    // Disable filtering
    lis_ret_code = lis2dh12_high_pass_on_outputs_set (&dev.ctx, PROPERTY_DISABLE);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }

    return err_code;
}

static rd_status_t selftest (void)
{
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
    lis2dh12_log_debug ("begin\r\n");
#endif
    axis3bit16_t data_raw_acceleration_old = {0};
    axis3bit16_t data_raw_acceleration_new = {0};
    int32_t lis_ret_code = LIS_SUCCESS;
    rd_status_t err_code = RD_SUCCESS;
    // Set Output Data Rate for self-test
    dev.samplerate = LIS2DH12_ODR_400Hz;
    lis2dh12_data_rate_set (&dev.ctx, dev.samplerate);
    // Set full scale to 2G for self-test
    dev.scale = LIS2DH12_2g;
    lis2dh12_full_scale_set (&dev.ctx, dev.scale);
    // Set device in 10 bit mode
    dev.resolution = LIS2DH12_NM_10bit;
    lis2dh12_operating_mode_set (&dev.ctx, dev.resolution);
    // Run self-test
    // turn self-test off.
    dev.selftest = LIS2DH12_ST_DISABLE;
    lis_ret_code = lis2dh12_self_test_set (&dev.ctx, dev.selftest);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_SELFTEST;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    lis2dh12_get_samples_selftest (&data_raw_acceleration_old);
    dev.selftest = LIS2DH12_ST_POSITIVE;
    lis2dh12_self_test_set (&dev.ctx, dev.selftest);
    lis2dh12_get_samples_selftest (&data_raw_acceleration_new);
    lis_ret_code = lis2dh12_verify_selftest (&data_raw_acceleration_new,
                   &data_raw_acceleration_old);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_SELFTEST;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
    lis2dh12_log_debug ("selftest = LIS2DH12_ST_POSITIVE.Error = %08x\r\n", err_code);
#endif
    dev.selftest = LIS2DH12_ST_NEGATIVE;
    lis2dh12_self_test_set (&dev.ctx, dev.selftest);
    lis2dh12_get_samples_selftest (&data_raw_acceleration_new);
    lis_ret_code = lis2dh12_verify_selftest (&data_raw_acceleration_new,
                   &data_raw_acceleration_old);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_SELFTEST;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
    lis2dh12_log_debug ("selftest = LIS2DH12_ST_NEGATIVE.Error = %08x\r\n", err_code);
#endif
    // turn self-test off, keep error code in case we "lose" sensor after self-test
    dev.selftest = LIS2DH12_ST_DISABLE;
    lis_ret_code = lis2dh12_self_test_set (&dev.ctx, dev.selftest);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    // Turn accelerometer off
    dev.samplerate = LIS2DH12_POWER_DOWN;
    lis_ret_code = lis2dh12_data_rate_set (&dev.ctx, dev.samplerate);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
    lis2dh12_log_debug ("end. Error = %08x\r\n", err_code);
#endif
    return err_code;
}

rd_status_t ri_lis2dh12_init (rd_sensor_t * p_sensor, rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
    lis2dh12_log_debug ("begin\r\n");
#endif

    if (NULL == p_sensor)
    {
        err_code |= RD_ERROR_NULL;
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
        lis2dh12_log_debug ("p_sensor. Error = %08x\r\n", err_code);
#endif
    }
    else if (NULL != dev.ctx.write_reg)
    {
        err_code |= RD_ERROR_INVALID_STATE;
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
        lis2dh12_log_debug ("write_reg. Error = %08x\r\n", err_code);
#endif
    }
    else
    {
        err_code |= dev_ctx_init (bus, handle);
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
        lis2dh12_log_debug ("dev_ctx_init. Error = %08x\r\n", err_code);
#endif
        rd_sensor_initialize (p_sensor);
        p_sensor->name = m_acc_name;

        if (RD_SUCCESS == err_code)
        {
            err_code |= check_whoami();
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
            lis2dh12_log_debug ("check_whoami. Error = %08x\r\n", err_code);
#endif
        }

        if (RD_SUCCESS == err_code)
        {
            err_code |= clear_sensor_state();
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
            lis2dh12_log_debug ("clear_sensor_state. Error = %08x\r\n", err_code);
#endif
        }

        if (RD_SUCCESS == err_code)
        {
            err_code |= selftest();
#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
            lis2dh12_log_debug ("selftest. Error = %08x\r\n", err_code);
#endif
        }

        if (RD_SUCCESS == err_code)
        {
            p_sensor->init                  = ri_lis2dh12_init;
            p_sensor->uninit                = ri_lis2dh12_uninit;
            p_sensor->samplerate_set        = ri_lis2dh12_samplerate_set;
            p_sensor->samplerate_get        = ri_lis2dh12_samplerate_get;
            p_sensor->resolution_set        = ri_lis2dh12_resolution_set;
            p_sensor->resolution_get        = ri_lis2dh12_resolution_get;
            p_sensor->scale_set             = ri_lis2dh12_scale_set;
            p_sensor->scale_get             = ri_lis2dh12_scale_get;
            p_sensor->dsp_set               = ri_lis2dh12_dsp_set;
            p_sensor->dsp_get               = ri_lis2dh12_dsp_get;
            p_sensor->mode_set              = ri_lis2dh12_mode_set;
            p_sensor->mode_get              = ri_lis2dh12_mode_get;
            p_sensor->data_get              = ri_lis2dh12_data_get;
            p_sensor->configuration_set     = rd_sensor_configuration_set;
            p_sensor->configuration_get     = rd_sensor_configuration_get;
            p_sensor->fifo_enable           = ri_lis2dh12_fifo_use;
            p_sensor->fifo_interrupt_enable = ri_lis2dh12_fifo_interrupt_use;
            p_sensor->fifo_read             = ri_lis2dh12_fifo_read;
            p_sensor->level_interrupt_set   = ri_lis2dh12_active_interrupt;
            p_sensor->provides.datas.acceleration_x_g = 1;
            p_sensor->provides.datas.acceleration_y_g = 1;
            p_sensor->provides.datas.acceleration_z_g = 1;
            p_sensor->provides.datas.temperature_c = 1;
            dev.tsample = RD_UINT64_INVALID;
        }
        else
        {
            rd_sensor_uninitialize (p_sensor);
            memset (&dev, 0, sizeof (dev));
        }
    }

#ifdef RUUVI_NRF5_SDK15_LIS2GH12_DEBUG
    lis2dh12_log_debug ("end. Error = %08x\r\n", err_code);
#endif
    return err_code;
}

/*
 * Lis2dh12 does not have a proper softreset (boot does not reset all registers)
 * Therefore just stop sampling
 */
rd_status_t ri_lis2dh12_uninit (rd_sensor_t * p_sensor,
                                rd_bus_t bus, uint8_t handle)
{
    rd_status_t err_code = RD_SUCCESS;
    RD_UNUSED_PARAMETER (bus);
    RD_UNUSED_PARAMETER (handle);

    if (NULL == p_sensor)
    {
        err_code |= RD_ERROR_NULL;
    }
    else
    {
        rd_sensor_uninitialize (p_sensor);
        dev.samplerate = LIS2DH12_POWER_DOWN;
        //LIS2DH12 function returns SPI write result which is rd_status_t
        int32_t lis_ret_code = lis2dh12_data_rate_set (& (dev.ctx), dev.samplerate);
        memset (&dev, 0, sizeof (dev));

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
    }

    return err_code;
}

/**
 * Set up samplerate. Powers down sensor on SAMPLERATE_STOP, writes value to
 * lis2dh12 only if mode is continous as writing samplerate to sensor starts sampling.
 * MAX is 200 Hz as it can be represented by the configuration format
 * Samplerate is rounded up, i.e. "Please give me at least samplerate F.", 5 is rounded to 10 Hz etc.
 */
rd_status_t ri_lis2dh12_samplerate_set (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == samplerate)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        lis2dh12_verify_sensor_sleep();
        int32_t lis_ret_code;

        if (RD_SENSOR_CFG_NO_CHANGE != *samplerate)
        {
            if (RD_SENSOR_CFG_MIN == *samplerate)      { dev.samplerate = LIS2DH12_ODR_1Hz;   }
            else if (RD_SENSOR_CFG_MAX == *samplerate)      { dev.samplerate = LIS2DH12_ODR_5kHz376_LP_1kHz344_NM_HP; }
            else if (RD_SENSOR_CFG_DEFAULT == *samplerate)  { dev.samplerate = LIS2DH12_ODR_1Hz;   }
            else if (ACC_SAMPLERATE_1HZ   == *samplerate)   { dev.samplerate = LIS2DH12_ODR_1Hz;   }
            else if (ACC_SAMPLERATE_10HZ  >= *samplerate)   { dev.samplerate = LIS2DH12_ODR_10Hz;  }
            else if (ACC_SAMPLERATE_25HZ  >= *samplerate)   { dev.samplerate = LIS2DH12_ODR_25Hz;  }
            else if (ACC_SAMPLERATE_50HZ  >= *samplerate)   { dev.samplerate = LIS2DH12_ODR_50Hz;  }
            else if (ACC_SAMPLERATE_100HZ >= *samplerate)   { dev.samplerate = LIS2DH12_ODR_100Hz; }
            else if (ACC_SAMPLERATE_200HZ >= *samplerate)   { dev.samplerate = LIS2DH12_ODR_200Hz; }
            else if (RD_SENSOR_CFG_CUSTOM_1 == *samplerate) { dev.samplerate = LIS2DH12_ODR_400Hz; }
            else if (RD_SENSOR_CFG_CUSTOM_2 == *samplerate) { dev.samplerate = LIS2DH12_ODR_1kHz620_LP; }
            // This is equal to LIS2DH12_ODR_5kHz376_LP
            else if (RD_SENSOR_CFG_CUSTOM_3 == *samplerate) { dev.samplerate = LIS2DH12_ODR_5kHz376_LP_1kHz344_NM_HP; }
            else { *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED; err_code |= RD_ERROR_NOT_SUPPORTED; }
        }

        if (RD_SUCCESS == err_code)
        {
            lis_ret_code = lis2dh12_data_rate_set (& (dev.ctx), dev.samplerate);

            if (LIS_SUCCESS != lis_ret_code)
            {
                err_code |= RD_ERROR_INTERNAL;
            }
            else
            {
                err_code |= RD_SUCCESS;
            }

            err_code |= ri_lis2dh12_samplerate_get (samplerate);
        }
    }

    return err_code;
}

/*
 *. Read sample rate to pointer
 */
rd_status_t ri_lis2dh12_samplerate_get (uint8_t * samplerate)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == samplerate)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        int32_t lis_ret_code;
        lis_ret_code = lis2dh12_data_rate_get (& (dev.ctx), &dev.samplerate);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        switch (dev.samplerate)
        {
            case LIS2DH12_ODR_1Hz:
                *samplerate = ACC_SAMPLERATE_1HZ;
                break;

            case LIS2DH12_ODR_10Hz:
                *samplerate = ACC_SAMPLERATE_10HZ;
                break;

            case LIS2DH12_ODR_25Hz:
                *samplerate = ACC_SAMPLERATE_25HZ;
                break;

            case LIS2DH12_ODR_50Hz:
                *samplerate = ACC_SAMPLERATE_50HZ;
                break;

            case LIS2DH12_ODR_100Hz:
                *samplerate = ACC_SAMPLERATE_100HZ;
                break;

            case LIS2DH12_ODR_200Hz:
                *samplerate = ACC_SAMPLERATE_200HZ;
                break;

            case LIS2DH12_ODR_400Hz:
                *samplerate = RD_SENSOR_CFG_CUSTOM_1;
                break;

            case LIS2DH12_ODR_5kHz376_LP_1kHz344_NM_HP:
                *samplerate = RD_SENSOR_CFG_MAX;
                break;

            case LIS2DH12_ODR_1kHz620_LP:
                *samplerate = RD_SENSOR_CFG_CUSTOM_2;
                break;

            default:
                *samplerate = RD_SENSOR_ERR_NOT_SUPPORTED;
                err_code |=  RD_ERROR_INTERNAL;
        }
    }

    return err_code;
}

/**
 * Setup resolution. Resolution is rounded up, i.e. "please give at least this many bits"
 */
rd_status_t ri_lis2dh12_resolution_set (uint8_t * resolution)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == resolution)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        lis2dh12_verify_sensor_sleep();
        int32_t lis_ret_code;

        if (RD_SENSOR_CFG_NO_CHANGE != *resolution)
        {
            if (RD_SENSOR_CFG_MIN == *resolution)     { dev.resolution = LIS2DH12_LP_8bit;  }
            else if (RD_SENSOR_CFG_MAX == *resolution)     { dev.resolution = LIS2DH12_HR_12bit; }
            else if (RD_SENSOR_CFG_DEFAULT == *resolution) { dev.resolution = LIS2DH12_NM_10bit; }
            else if (ACC_RESOLUTION_8BITS >= *resolution)  { dev.resolution = LIS2DH12_LP_8bit;  }
            else if (ACC_RESOLUTION_10BITS >= *resolution) { dev.resolution = LIS2DH12_NM_10bit; }
            else if (ACC_RESOLUTION_12BITS >= *resolution) { dev.resolution = LIS2DH12_HR_12bit; }
            else { *resolution = RD_SENSOR_ERR_NOT_SUPPORTED; err_code |= RD_ERROR_NOT_SUPPORTED; }
        }

        if (RD_SUCCESS == err_code)
        {
            lis_ret_code = lis2dh12_operating_mode_set (& (dev.ctx), dev.resolution);

            if (LIS_SUCCESS != lis_ret_code)
            {
                err_code |= RD_ERROR_INTERNAL;
            }
            else
            {
                err_code |= RD_SUCCESS;
            }

            err_code |= ri_lis2dh12_resolution_get (resolution);
        }
    }

    return err_code;
}

rd_status_t ri_lis2dh12_resolution_get (uint8_t * resolution)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == resolution)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        int32_t lis_ret_code;
        lis_ret_code = lis2dh12_operating_mode_get (& (dev.ctx), &dev.resolution);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        switch (dev.resolution)
        {
            case LIS2DH12_LP_8bit:
                *resolution = ACC_RESOLUTION_8BITS;
                break;

            case LIS2DH12_NM_10bit:
                *resolution = ACC_RESOLUTION_10BITS;
                break;

            case LIS2DH12_HR_12bit:
                *resolution = ACC_RESOLUTION_12BITS;
                break;

            default:
                *resolution = RD_SENSOR_ERR_INVALID;
                err_code |= RD_ERROR_INTERNAL;
                break;
        }
    }

    return err_code;
}

/**
 * Setup lis2dh12 scale. Scale is rounded up, i.e. "at least this much"
 */
rd_status_t ri_lis2dh12_scale_set (uint8_t * scale)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == scale)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        lis2dh12_verify_sensor_sleep();
        int32_t lis_ret_code;

        if (RD_SENSOR_CFG_NO_CHANGE != *scale)
        {
            if (RD_SENSOR_CFG_MIN == *scale)     { dev.scale = LIS2DH12_2g; }
            else if (RD_SENSOR_CFG_MAX == *scale)     { dev.scale = LIS2DH12_16g; }
            else if (RD_SENSOR_CFG_DEFAULT == *scale) { dev.scale = LIS2DH12_2g;  }
            else if (ACC_SCALE_2G  >= *scale)         { dev.scale = LIS2DH12_2g;  }
            else if (ACC_SCALE_4G  >= *scale)         { dev.scale = LIS2DH12_4g;  }
            else if (ACC_SCALE_8G  >= *scale)         { dev.scale = LIS2DH12_8g;  }
            else if (ACC_SCALE_16G >= *scale)         { dev.scale = LIS2DH12_16g; }
            else
            {
                *scale = RD_SENSOR_ERR_NOT_SUPPORTED;
                err_code |= RD_ERROR_NOT_SUPPORTED;
            }
        }

        if (RD_SUCCESS == err_code)
        {
            lis_ret_code = lis2dh12_full_scale_set (& (dev.ctx), dev.scale);

            if (LIS_SUCCESS != lis_ret_code)
            {
                err_code |= RD_ERROR_INTERNAL;
            }
            else
            {
                err_code |= RD_SUCCESS;
            }

            err_code |= ri_lis2dh12_scale_get (scale);
        }
    }

    return err_code;
}

rd_status_t ri_lis2dh12_scale_get (uint8_t * scale)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == scale)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        int32_t lis_ret_code;
        lis_ret_code = lis2dh12_full_scale_get (& (dev.ctx), &dev.scale);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        switch (dev.scale)
        {
            case LIS2DH12_2g:
                *scale = ACC_SCALE_2G;
                break;

            case LIS2DH12_4g:
                *scale = ACC_SCALE_4G;
                break;

            case LIS2DH12_8g:
                *scale = ACC_SCALE_8G;
                break;

            case  LIS2DH12_16g:
                *scale = ACC_SCALE_16G;
                break;

            default:
                *scale = RD_SENSOR_ERR_NOT_SUPPORTED;
                err_code |= RD_ERROR_INTERNAL;
                break;
        }
    }

    return err_code;
}

static rd_status_t ri_lis2dh12_dsp_high_pass_set (uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code;
    lis2dh12_hpcf_t hpcf;

    // Set default here to avoid duplicate value error in switch-case
    if (RD_SENSOR_CFG_DEFAULT == *parameter) { *parameter = 0; }

    switch (*parameter)
    {
        case RD_SENSOR_CFG_MIN:
        case ACC_DSP_MODE_0:
            hpcf = LIS2DH12_LIGHT;
            *parameter = 0;
            break;

        case ACC_DSP_MODE_1:
            hpcf = LIS2DH12_MEDIUM;
            break;

        case ACC_DSP_MODE_2:
            hpcf = LIS2DH12_STRONG;
            break;

        case RD_SENSOR_CFG_MAX:
        case ACC_DSP_MODE_3:
            hpcf = LIS2DH12_AGGRESSIVE;
            *parameter = ACC_DSP_MODE_3;
            break;

        default :
            *parameter = RD_ERROR_NOT_SUPPORTED;
            err_code = RD_ERROR_NOT_SUPPORTED;
            break;
    }

    if (err_code != RD_ERROR_NOT_SUPPORTED)
    {
        lis_ret_code = lis2dh12_high_pass_bandwidth_set (& (dev.ctx), hpcf);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        lis_ret_code = lis2dh12_high_pass_mode_set (& (dev.ctx), LIS2DH12_NORMAL);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        lis_ret_code = lis2dh12_high_pass_on_outputs_set (& (dev.ctx), PROPERTY_ENABLE);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }
    }

    return err_code;
}

/*
 Based on AN4662(Visit http://www.st.com and search Application note DM00165265).
 CTRL2 DCF [1:0] HP cutoff frequency [Hz]
 00 ODR/50
 01 ODR/100
 10 ODR/9
 11 ODR/400
*/
rd_status_t ri_lis2dh12_dsp_set (uint8_t * dsp, uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == dsp) || (NULL == parameter))
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        lis2dh12_verify_sensor_sleep();
        int32_t lis_ret_code;
        // Read original values in case one is NO_CHANGE and other should be adjusted.
        uint8_t orig_dsp;
        uint8_t orig_param;
        err_code |= ri_lis2dh12_dsp_get (&orig_dsp, &orig_param);

        if (RD_SENSOR_CFG_NO_CHANGE == *dsp)       { *dsp       = orig_dsp; }

        if (RD_SENSOR_CFG_NO_CHANGE == *parameter) { *parameter = orig_param; }

        if (RD_SENSOR_CFG_DEFAULT == *dsp)
        {
            *dsp = RD_SENSOR_DSP_LAST;
        }

        if (RD_SENSOR_DSP_HIGH_PASS == *dsp)
        {
            err_code = ri_lis2dh12_dsp_high_pass_set (parameter);
        }
        // Has no effect, but kept for future-proofness.
        else if (RD_SENSOR_DSP_LAST == *dsp)
        {
            lis_ret_code = lis2dh12_high_pass_on_outputs_set (& (dev.ctx), PROPERTY_DISABLE);

            if (LIS_SUCCESS != lis_ret_code)
            {
                err_code |= RD_ERROR_INTERNAL;
            }
            else
            {
                err_code |= RD_SUCCESS;
            }

            *dsp = RD_SENSOR_DSP_LAST;
        }
        else
        {
            err_code = RD_ERROR_NOT_SUPPORTED;
        }
    }

    return err_code;
}

rd_status_t ri_lis2dh12_dsp_get (uint8_t * dsp, uint8_t * parameter)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code;
    uint8_t mode;
    lis2dh12_hpcf_t hpcf;
    lis_ret_code = lis2dh12_high_pass_bandwidth_get (& (dev.ctx), &hpcf);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    lis_ret_code = lis2dh12_high_pass_on_outputs_get (& (dev.ctx), &mode);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    if (mode != 0) { *dsp = RD_SENSOR_DSP_HIGH_PASS; }
    else { *dsp = RD_SENSOR_DSP_LAST; }

    switch (hpcf)
    {
        case LIS2DH12_LIGHT:
            *parameter = ACC_DSP_MODE_0;
            break;

        case LIS2DH12_MEDIUM:
            *parameter = ACC_DSP_MODE_1;
            break;

        case LIS2DH12_STRONG:
            *parameter = ACC_DSP_MODE_2;
            break;

        case LIS2DH12_AGGRESSIVE:
            *parameter = ACC_DSP_MODE_3;
            break;

        default:
            *parameter  = RD_SENSOR_ERR_NOT_SUPPORTED;
            err_code = RD_ERROR_INTERNAL;
            break;
    }

    return err_code;
}

static rd_status_t ri_lis2dh12_mode_single_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code;
    // Do nothing if sensor is in continuous mode
    uint8_t current_mode;
    ri_lis2dh12_mode_get (&current_mode);

    if (RD_SENSOR_CFG_CONTINUOUS == current_mode)
    {
        *mode = RD_SENSOR_CFG_CONTINUOUS;
        err_code = RD_ERROR_INVALID_STATE;
    }
    else
    {
        // Start sensor at 400 Hz (highest common samplerate)
        // and wait for 7/ODR ms for turn-on (?) NOTE: 7 s / 400 just to be on safe side.
        // Refer to LIS2DH12 datasheet p.16.
        dev.samplerate = LIS2DH12_ODR_400Hz;
        lis_ret_code = lis2dh12_data_rate_set (& (dev.ctx), dev.samplerate);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        ri_delay_ms ( (ACC_ODR_DELAY / ACC_SAMPLERATE_400HZ) + 1);
        dev.tsample = rd_sensor_timestamp_get();
        dev.samplerate = LIS2DH12_POWER_DOWN;
        lis_ret_code = lis2dh12_data_rate_set (& (dev.ctx), dev.samplerate);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        *mode = RD_SENSOR_CFG_SLEEP;
    }

    return err_code;
}

rd_status_t ri_lis2dh12_mode_set (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        int32_t lis_ret_code;

        if (RD_SENSOR_CFG_SINGLE == *mode)
        {
            err_code = ri_lis2dh12_mode_single_set (mode);
        }
        // Do not store power down mode to dev structure, so we can continue at previous data rate
        // when mode is set to continous.
        else if (RD_SENSOR_CFG_SLEEP == *mode)
        {
            dev.mode = *mode;
            lis_ret_code = lis2dh12_data_rate_set (& (dev.ctx), LIS2DH12_POWER_DOWN);

            if (LIS_SUCCESS != lis_ret_code)
            {
                err_code |= RD_ERROR_INTERNAL;
            }
            else
            {
                err_code |= RD_SUCCESS;
            }
        }
        else if (RD_SENSOR_CFG_CONTINUOUS == *mode)
        {
            dev.mode = *mode;
            lis_ret_code = lis2dh12_data_rate_set (& (dev.ctx), dev.samplerate);

            if (LIS_SUCCESS != lis_ret_code)
            {
                err_code |= RD_ERROR_INTERNAL;
            }
            else
            {
                err_code |= RD_SUCCESS;
            }
        }
        else
        {
            err_code |= RD_ERROR_INVALID_PARAM;
        }
    }

    return err_code;
}

rd_status_t ri_lis2dh12_mode_get (uint8_t * mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == mode)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        switch (dev.mode)
        {
            case RD_SENSOR_CFG_SLEEP:
                *mode = RD_SENSOR_CFG_SLEEP;
                break;

            case RD_SENSOR_CFG_CONTINUOUS:
                *mode = RD_SENSOR_CFG_CONTINUOUS;
                break;

            default:
                *mode = RD_SENSOR_ERR_NOT_SUPPORTED;
                err_code = RD_ERROR_INTERNAL;
                break;
        }
    }

    return err_code;
}

/**
 * Convert raw value to temperature in celcius
 *
 * parameter raw: Input. Raw values from LIS2DH12-
 * parameter acceleration: Output. Temperature values in C.
 *
 */
static rd_status_t rawToC (const uint8_t * const raw_temperature,
                           rd_float * temperature)
{
    rd_status_t err_code = RD_SUCCESS;
    int16_t value = (raw_temperature[1] * 256) + raw_temperature[0];

    switch (dev.resolution)
    {
        case LIS2DH12_LP_8bit:
            *temperature = lis2dh12_from_lsb_lp_to_celsius (value);
            break;

        case LIS2DH12_NM_10bit:
            *temperature = lis2dh12_from_lsb_nm_to_celsius (value);
            break;

        case LIS2DH12_HR_12bit:
            *temperature = lis2dh12_from_lsb_hr_to_celsius (value);
            break;

        default:
            *temperature = RD_FLOAT_INVALID;
            err_code |= RD_ERROR_INTERNAL;
            break;
    }

    return err_code;
}

/**
 * Convert raw value to acceleration in mg for 2g
 */
static rd_status_t rawToMg2g (int16_t raw_acceleration,
                              rd_float * acceleration)
{
    rd_status_t err_code = RD_SUCCESS;

    switch (dev.resolution)
    {
        case LIS2DH12_LP_8bit:
            (*acceleration) = lis2dh12_from_fs2_lp_to_mg (raw_acceleration);
            break;

        case LIS2DH12_NM_10bit:
            (*acceleration) = lis2dh12_from_fs2_nm_to_mg (raw_acceleration);
            break;

        case LIS2DH12_HR_12bit:
            (*acceleration) = lis2dh12_from_fs2_hr_to_mg (raw_acceleration);
            break;

        default:
            (*acceleration) = RD_FLOAT_INVALID;
            err_code |= RD_ERROR_INTERNAL;
            break;
    }

    return err_code;
}

/**
 * Convert raw value to acceleration in mg for 4g
 */
static rd_status_t rawToMg4g (int16_t raw_acceleration,
                              rd_float * acceleration)
{
    rd_status_t err_code = RD_SUCCESS;

    switch (dev.resolution)
    {
        case LIS2DH12_LP_8bit:
            (*acceleration) = lis2dh12_from_fs4_lp_to_mg (raw_acceleration);
            break;

        case LIS2DH12_NM_10bit:
            (*acceleration) = lis2dh12_from_fs4_nm_to_mg (raw_acceleration);
            break;

        case LIS2DH12_HR_12bit:
            (*acceleration) = lis2dh12_from_fs4_hr_to_mg (raw_acceleration);
            break;

        default:
            (*acceleration) = RD_FLOAT_INVALID;
            err_code |= RD_ERROR_INTERNAL;
            break;
    }

    return err_code;
}

/**
 * Convert raw value to acceleration in mg for 8g
 */
static rd_status_t rawToMg8g (int16_t raw_acceleration,
                              rd_float * acceleration)
{
    rd_status_t err_code = RD_SUCCESS;

    switch (dev.resolution)
    {
        case LIS2DH12_LP_8bit:
            (*acceleration) = lis2dh12_from_fs8_lp_to_mg (raw_acceleration);
            break;

        case LIS2DH12_NM_10bit:
            (*acceleration) = lis2dh12_from_fs8_nm_to_mg (raw_acceleration);
            break;

        case LIS2DH12_HR_12bit:
            (*acceleration) = lis2dh12_from_fs8_hr_to_mg (raw_acceleration);
            break;

        default:
            (*acceleration) = RD_FLOAT_INVALID;
            err_code |= RD_ERROR_INTERNAL;
            break;
    }

    return err_code;
}

/**
 * Convert raw value to acceleration in mg for 8g
 */
static rd_status_t rawToMg16g (int16_t raw_acceleration,
                               rd_float * acceleration)
{
    rd_status_t err_code = RD_SUCCESS;

    switch (dev.resolution)
    {
        case LIS2DH12_LP_8bit:
            (*acceleration) = lis2dh12_from_fs16_lp_to_mg (raw_acceleration);
            break;

        case LIS2DH12_NM_10bit:
            (*acceleration) = lis2dh12_from_fs16_nm_to_mg (raw_acceleration);
            break;

        case LIS2DH12_HR_12bit:
            (*acceleration) = lis2dh12_from_fs16_hr_to_mg (raw_acceleration);
            break;

        default:
            (*acceleration) = RD_FLOAT_INVALID;
            err_code |= RD_ERROR_INTERNAL;
            break;
    }

    return err_code;
}

/**
 * Convert raw value to acceleration in mg
 *
 * parameter raw: Input. Raw values from LIS2DH12
 * parameter acceleration: Output. Acceleration values in mg
 *
 */
static rd_status_t rawToMg (const axis3bit16_t * raw_acceleration,
                            rd_float * acceleration)
{
    rd_status_t err_code = RD_SUCCESS;

    for (size_t ii = 0; ii < ACC_XYZ_NUM; ii++)
    {
        switch (dev.scale)
        {
            case LIS2DH12_2g:
                err_code |= rawToMg2g (raw_acceleration->i16bit[ii], &acceleration[ii]);
                break;

            case LIS2DH12_4g:
                err_code |= rawToMg4g (raw_acceleration->i16bit[ii], &acceleration[ii]);
                break;

            case LIS2DH12_8g:
                err_code |= rawToMg8g (raw_acceleration->i16bit[ii], &acceleration[ii]);
                break;

            case LIS2DH12_16g:
                err_code |= rawToMg16g (raw_acceleration->i16bit[ii], &acceleration[ii]);
                break;

            default:
                acceleration[ii] = RD_FLOAT_INVALID;
                err_code |= RD_ERROR_INTERNAL;
                break;
        }
    }

    return err_code;
}

rd_status_t ri_lis2dh12_data_get (rd_sensor_data_t * const
                                  data)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == data)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        int32_t lis_ret_code;
        axis3bit16_t raw_acceleration;
        uint8_t raw_temperature[2];
        memset (raw_acceleration.u8bit, 0x00, ACC_XYZ_NUM * sizeof (int16_t));
        lis_ret_code = lis2dh12_acceleration_raw_get (& (dev.ctx), raw_acceleration.u8bit);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        lis_ret_code = lis2dh12_temperature_raw_get (& (dev.ctx), raw_temperature);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        // Compensate data with resolution, scale
        rd_float acceleration[ACC_XYZ_NUM];
        rd_float temperature;
        err_code |= rawToMg (&raw_acceleration, acceleration);
        err_code |= rawToC (raw_temperature, &temperature);
        uint8_t mode;
        err_code |= ri_lis2dh12_mode_get (&mode);

        if (RD_SENSOR_CFG_SLEEP == mode)           {  data->timestamp_ms   = dev.tsample; }
        else if (RD_SENSOR_CFG_CONTINUOUS == mode)
        {
            data->timestamp_ms = rd_sensor_timestamp_get();
        }
        else { RD_ERROR_CHECK (RD_ERROR_INTERNAL, ~RD_ERROR_FATAL); }

        // If we have valid data, return it.
        if ( (RD_UINT64_INVALID != data->timestamp_ms)
                && (RD_SUCCESS == err_code))
        {
            rd_sensor_data_t d_acceleration;
            rd_float values[ACC_XYZ_NUM + 1];
            rd_sensor_data_fields_t acc_fields = {.bitfield = 0};
            d_acceleration.data = values;
            acc_fields.datas.acceleration_x_g = 1;
            acc_fields.datas.acceleration_y_g = 1;
            acc_fields.datas.acceleration_z_g = 1;
            acc_fields.datas.temperature_c = 1;
            //Convert mG to G.
            values[ACC_X] = acceleration[ACC_X] / ACC_G_TO_MG_DIVIDER;
            values[ACC_Y] = acceleration[ACC_Y] / ACC_G_TO_MG_DIVIDER;
            values[ACC_Z] = acceleration[ACC_Z] / ACC_G_TO_MG_DIVIDER;
            values[ACC_XYZ_NUM] = temperature;
            d_acceleration.valid  = acc_fields;
            d_acceleration.fields = acc_fields;
            rd_sensor_data_populate (data,
                                     &d_acceleration,
                                     data->fields);
        }
    }

    return err_code;
}

// TODO: State checks
rd_status_t ri_lis2dh12_fifo_use (const bool enable)
{
    lis2dh12_fm_t mode;
    int32_t lis_ret_code;
    rd_status_t err_code = RD_SUCCESS;

    if (enable) { mode = LIS2DH12_DYNAMIC_STREAM_MODE; }
    else { mode = LIS2DH12_BYPASS_MODE; }

    lis_ret_code = lis2dh12_fifo_set (& (dev.ctx), enable);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    lis_ret_code = lis2dh12_fifo_mode_set (& (dev.ctx),  mode);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    return err_code;
}

static rd_status_t ri_lis2dh12_fifo_read_elements (size_t * num_elements,
        rd_sensor_data_t * p_data)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code;
    uint8_t elements = 0;
    lis_ret_code = lis2dh12_fifo_data_level_get (& (dev.ctx), &elements);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    if (!elements)
    {
        *num_elements = 0;
        err_code = RD_SUCCESS;
    }
    else
    {
        // 31 FIFO + latest
        elements++;

        // Do not read more than buffer size
        if (elements > (uint8_t) (*num_elements)) { elements = (uint8_t) (*num_elements); }

        // get current time
        p_data->timestamp_ms = rd_sensor_timestamp_get();
        // Read all elements
        axis3bit16_t raw_acceleration;
        rd_float acceleration[ACC_XYZ_NUM];

        for (size_t ii = 0; ii < elements; ii++)
        {
            lis_ret_code = lis2dh12_acceleration_raw_get (& (dev.ctx), raw_acceleration.u8bit);

            if (LIS_SUCCESS != lis_ret_code)
            {
                err_code |= RD_ERROR_INTERNAL;
            }
            else
            {
                err_code |= RD_SUCCESS;
            }

            // Compensate data with resolution, scale
            err_code |= rawToMg (&raw_acceleration, acceleration);
            rd_sensor_data_t d_acceleration;
            rd_sensor_data_fields_t acc_fields = {.bitfield = 0};
            acc_fields.datas.acceleration_x_g = 1;
            acc_fields.datas.acceleration_y_g = 1;
            acc_fields.datas.acceleration_z_g = 1;
            //Convert mG to G
            acceleration[ACC_X] = acceleration[ACC_X] / ACC_G_TO_MG_DIVIDER;
            acceleration[ACC_Y] = acceleration[ACC_Y] / ACC_G_TO_MG_DIVIDER;
            acceleration[ACC_Z] = acceleration[ACC_Z] / ACC_G_TO_MG_DIVIDER;
            d_acceleration.data = acceleration;
            d_acceleration.valid  = acc_fields;
            d_acceleration.fields = acc_fields;
            rd_sensor_data_populate (& (p_data[ii]),
                                     &d_acceleration,
                                     p_data[ii].fields);
        }

        *num_elements = elements;
    }

    return err_code;
}

//TODO * return: RD_INVALID_STATE if FIFO is not in use
rd_status_t ri_lis2dh12_fifo_read (size_t * num_elements,
                                   rd_sensor_data_t * p_data)
{
    rd_status_t err_code = RD_SUCCESS;

    if ( (NULL == num_elements) || (NULL == p_data))
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        err_code |= ri_lis2dh12_fifo_read_elements (num_elements, p_data);
    }

    return err_code;
}


rd_status_t ri_lis2dh12_fifo_interrupt_use (const bool enable)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code;
    lis2dh12_ctrl_reg3_t ctrl = { 0 };

    if (true == enable)
    {
        // Setting the FTH [4:0] bit in the FIFO_CTRL_REG (2Eh) register to an N value,
        // the number of X, Y and Z data samples that should be read at the rise
        // of the watermark interrupt is up to (N+1).
        lis_ret_code = lis2dh12_fifo_watermark_set (& (dev.ctx), ACC_FIFO_WATERMARK);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        ctrl.i1_wtm = PROPERTY_ENABLE;
    }

    lis_ret_code = lis2dh12_pin_int1_config_set (& (dev.ctx), &ctrl);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    return err_code;
}

static rd_float ri_lis2dh12_get_divisor (uint8_t scale)
{
    rd_float divisor;

    switch (scale)
    {
        case ACC_SCALE_2G:
            divisor = ACC_SCALE_DIV_2G;
            break;

        case ACC_SCALE_4G:
            divisor = ACC_SCALE_DIV_4G;
            break;

        case ACC_SCALE_8G:
            divisor = ACC_SCALE_DIV_8G;
            break;

        case ACC_SCALE_16G:
            divisor = ACC_SCALE_DIV_16G;
            break;

        default:
            divisor = ACC_SCALE_DIV_2G;
            break;
    }

    return divisor;
}

static rd_status_t ri_lis2dh12_reconfig_interrupt (lis2dh12_hp_t high_pass,
        lis2dh12_int1_cfg_t * p_cfg,
        lis2dh12_ctrl_reg6_t * p_ctrl6)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code;
    // Configure highpass on INTERRUPT 1
    lis_ret_code = lis2dh12_high_pass_int_conf_set (& (dev.ctx), high_pass);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    // Configure INTERRUPT 1.
    lis_ret_code = lis2dh12_int1_gen_conf_set (& (dev.ctx), p_cfg);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    // Route INTERRUPT 1 to PIN 2.
    lis_ret_code = lis2dh12_pin_int2_config_set (& (dev.ctx), p_ctrl6);

    if (LIS_SUCCESS != lis_ret_code)
    {
        err_code |= RD_ERROR_INTERNAL;
    }
    else
    {
        err_code |= RD_SUCCESS;
    }

    return err_code;
}

/**
 * Enable activity interrupt on LIS2DH12
 * Triggers as ACTIVE HIGH interrupt while detected movement is above threshold limit_g
 * Axes are high-passed for this interrupt, i.e. gravity won't trigger the interrupt
 * Axes are examined individually, compound acceleration won't trigger the interrupt.
 *
 * parameter enable:  True to enable interrupt, false to disable interrupt
 * parameter limit_g: Desired acceleration to trigger the interrupt.
 *                    Is considered as "at least", the acceleration is rounded up to next value.
 *                    Is written with value that was set to interrupt
 * returns: RD_SUCCESS on success
 * returns: RD_ERROR_INVALID_STATE if acceleration limit is higher than maximum scale
 * returns: error code from stack on error.
 *
 */
rd_status_t ri_lis2dh12_active_interrupt (const bool enable, rd_float * const limit_g)
{
    rd_status_t err_code = RD_SUCCESS;
    int32_t lis_ret_code;
    lis2dh12_hp_t high_pass = LIS2DH12_ON_INT1_GEN;
    lis2dh12_ctrl_reg6_t ctrl6 = { 0 };
    lis2dh12_int1_cfg_t  cfg = { 0 };

    if (NULL == limit_g)
    {
        err_code |= RD_ERROR_NULL;
    }
    else if ( (0 > *limit_g) && enable)
    {
        err_code |= RD_ERROR_INVALID_PARAM;
    }
    else if (enable)
    {
        cfg.xhie     = PROPERTY_ENABLE;
        cfg.yhie     = PROPERTY_ENABLE;
        cfg.zhie     = PROPERTY_ENABLE;
        ctrl6.i2_ia1 = PROPERTY_ENABLE;
        /*
        Do not enable lower threshold on activity detection, as it would
        turn logic into not-active detection.
        */
        // Adjust for scale
        // 1 LSb = 16 mg @ FS = 2 g
        // 1 LSb = 32 mg @ FS = 4 g
        // 1 LSb = 62 mg @ FS = 8 g
        // 1 LSb = 186 mg @ FS = 16 g
        uint8_t  scale;
        uint32_t threshold;
        rd_float divisor;
        lis_ret_code = ri_lis2dh12_scale_get (&scale);

        if (LIS_SUCCESS != lis_ret_code)
        {
            err_code |= RD_ERROR_INTERNAL;
        }
        else
        {
            err_code |= RD_SUCCESS;
        }

        divisor = ri_lis2dh12_get_divisor (scale);
        threshold = (uint32_t) (*limit_g / divisor) + 1;

        if (threshold > MOTION_THRESHOLD_MAX)
        {
            err_code |= RD_ERROR_INVALID_PARAM;
        }
        else
        {
            *limit_g = ( (rd_float) threshold) * divisor;
            // Configure INTERRUPT 1 Threshold
            lis_ret_code = lis2dh12_int1_gen_threshold_set (& (dev.ctx), (uint8_t) threshold);

            if (LIS_SUCCESS != lis_ret_code)
            {
                err_code |= RD_ERROR_INTERNAL;
            }
            else
            {
                err_code |= RD_SUCCESS;
            }
        }
    }
    else
    {
        high_pass = LIS2DH12_DISC_FROM_INT_GENERATOR;
    }

    if (RD_SUCCESS == err_code)
    {
        err_code |= ri_lis2dh12_reconfig_interrupt (high_pass, &cfg, &ctrl6);
    }

    return err_code;
}
/*@}*/
#endif
