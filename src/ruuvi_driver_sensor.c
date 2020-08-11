#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include <stddef.h>
#include <string.h>

#define BITS_PER_BYTE (8U) //!< Number of bits in a byte.

static const char m_init_name[] = "NOTINIT";

rd_status_t rd_sensor_configuration_set (const rd_sensor_t *
        sensor, rd_sensor_configuration_t * config)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == sensor || NULL == config) { return RD_ERROR_NULL; }

    if (NULL == sensor->samplerate_set) { return RD_ERROR_INVALID_STATE; }

    uint8_t sleep = RD_SENSOR_CFG_SLEEP;
    err_code |= sensor->mode_set (&sleep);
    err_code |= sensor->samplerate_set (& (config->samplerate));
    err_code |= sensor->resolution_set (& (config->resolution));
    err_code |= sensor->scale_set (& (config->scale));
    err_code |= sensor->dsp_set (& (config->dsp_function), & (config->dsp_parameter));
    err_code |= sensor->mode_set (& (config->mode));
    return err_code;
}

rd_status_t rd_sensor_configuration_get (const rd_sensor_t *
        sensor, rd_sensor_configuration_t * config)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == sensor || NULL == config) { return RD_ERROR_NULL; }

    if (NULL == sensor->samplerate_set) { return RD_ERROR_INVALID_STATE; }

    err_code |= sensor->samplerate_get (& (config->samplerate));
    err_code |= sensor->resolution_get (& (config->resolution));
    err_code |= sensor->scale_get (& (config->scale));
    err_code |= sensor->dsp_get (& (config->dsp_function), & (config->dsp_parameter));
    err_code |= sensor->mode_get (& (config->mode));
    return err_code;
}

static rd_sensor_timestamp_fp millis = NULL;

rd_status_t rd_sensor_timestamp_function_set (
    rd_sensor_timestamp_fp timestamp_fp)
{
    millis = timestamp_fp;
    return RD_SUCCESS;
}

// Calls the timestamp function and returns it's value. returns 0 if timestamp function is NULL
uint64_t rd_sensor_timestamp_get (void)
{
    if (NULL == millis)
    {
        return 0;
    }

    return millis();
}

static rd_status_t rd_fifo_enable_ni (const bool enable)
{
    return RD_ERROR_NOT_INITIALIZED;
}

static rd_status_t rd_fifo_interrupt_enable_ni (const bool enable)
{
    return RD_ERROR_NOT_INITIALIZED;
}

static rd_status_t rd_fifo_read_ni (size_t * num_elements, rd_sensor_data_t * data)
{
    return RD_ERROR_NOT_INITIALIZED;
}

static rd_status_t rd_data_get_ni (rd_sensor_data_t * const data)
{
    return RD_ERROR_NOT_INITIALIZED;
}

static rd_status_t rd_init_ni (rd_sensor_t * const
                               p_sensor, const rd_bus_t bus, const uint8_t handle)
{
    return RD_ERROR_NOT_INITIALIZED;
}

static rd_status_t rd_setup_ni (uint8_t * const value)
{
    return RD_ERROR_NOT_INITIALIZED;
}

static rd_status_t rd_level_interrupt_use_ni (const bool enable,
        float * limit_g)
{
    return RD_ERROR_NOT_INITIALIZED;
}

static rd_status_t rd_dsp_ni (uint8_t * const dsp, uint8_t * const parameter)
{
    return RD_ERROR_NOT_INITIALIZED;
}

static rd_status_t rd_sensor_configuration_ni (const rd_sensor_t *
        sensor, rd_sensor_configuration_t * config)
{
    return RD_ERROR_NOT_INITIALIZED;
}

void rd_sensor_initialize (rd_sensor_t * const p_sensor)
{
    if (NULL != p_sensor->name)
    {
        p_sensor->name = m_init_name;
    }

    p_sensor->configuration_get     = rd_sensor_configuration_ni;
    p_sensor->configuration_set     = rd_sensor_configuration_ni;
    p_sensor->data_get              = rd_data_get_ni;
    p_sensor->dsp_get               = rd_dsp_ni;
    p_sensor->dsp_set               = rd_dsp_ni;
    p_sensor->fifo_enable           = rd_fifo_enable_ni;
    p_sensor->fifo_interrupt_enable = rd_fifo_interrupt_enable_ni;
    p_sensor->fifo_read             = rd_fifo_read_ni;
    p_sensor->init                  = rd_init_ni;
    p_sensor->uninit                = rd_init_ni;
    p_sensor->level_interrupt_set   = rd_level_interrupt_use_ni;
    p_sensor->mode_get              = rd_setup_ni;
    p_sensor->mode_set              = rd_setup_ni;
    p_sensor->resolution_get        = rd_setup_ni;
    p_sensor->resolution_set        = rd_setup_ni;
    p_sensor->samplerate_get        = rd_setup_ni;
    p_sensor->samplerate_set        = rd_setup_ni;
    p_sensor->scale_get             = rd_setup_ni;
    p_sensor->scale_set             = rd_setup_ni;
    memset (& (p_sensor->provides), 0, sizeof (p_sensor->provides));
}

void rd_sensor_uninitialize (rd_sensor_t * const p_sensor)
{
    // Reset sensor to initial values.
    rd_sensor_initialize (p_sensor);
}

static inline uint8_t get_index_of_field (const rd_sensor_data_t * const target,
        const rd_sensor_data_fields_t field)
{
    // Null bits higher than target
    uint8_t leading_zeros = (uint8_t) __builtin_clz (field.bitfield);
    uint32_t mask = UINT32_MAX;
    uint8_t bitfield_size = sizeof (field.bitfield) * BITS_PER_BYTE;
    uint8_t target_bit = (bitfield_size - leading_zeros);

    if (target_bit < bitfield_size)
    {
        mask = (1U << target_bit) - 1U;
    }

    // Count set bits in nulled bitfield to find index.
    uint8_t index = (uint8_t) (__builtin_popcount (target->fields.bitfield & mask) - 1);

    // return 0 if we don't have a valid result.
    if (index > bitfield_size)
    {
        index = 0;
    }

    return index;
}

float rd_sensor_data_parse (const rd_sensor_data_t * const provided,
                            const rd_sensor_data_fields_t requested)
{
    float rvalue = RD_FLOAT_INVALID;

    // If one value was requested and is available return value.
    if ( (NULL != provided)
            && (0 != (provided->valid.bitfield & requested.bitfield))
            && (1 == __builtin_popcount (requested.bitfield)))
    {
        rvalue = provided->data[get_index_of_field (provided, requested)];
    }

    return rvalue;
}

void rd_sensor_data_set (rd_sensor_data_t * const target,
                         const rd_sensor_data_fields_t field,
                         const float value)
{
    // If there isn't valid requested data, return
    if (NULL == target)
    {
        // No action needed
    }
    else if (! (target->fields.bitfield & field.bitfield))
    {
        // No action needed
    }
    // If trying to set more than one field, return.
    else if (1 != __builtin_popcount (field.bitfield))
    {
        // No action needed
    }
    else
    {
        // Set value to appropriate index
        target->data[get_index_of_field (target, field)] = value;
        // Mark data as valid
        target->valid.bitfield |= field.bitfield;
    }
}

bool rd_sensor_has_valid_data (const rd_sensor_data_t * const target,
                               const uint8_t index)
{
    uint8_t fieldcount = 0;
    rd_sensor_data_fields_t check = {0};
    uint8_t target_index = 0U;
    uint32_t mask = 0U;
    bool valid = false;

    if (NULL != target)
    {
        fieldcount = rd_sensor_data_fieldcount (target);
        check = target->fields;
    }

    // Verify bounds
    if (fieldcount > index)
    {
        // Find target field
        for (uint8_t ii = 0U; ii < index; ii++)
        {
            // Null trailing fields
            target_index = (uint8_t) __builtin_ctz (check.bitfield);
            mask = 1U << target_index;
            check.bitfield &= ~mask;
        }

        target_index = (uint8_t) __builtin_ctz (check.bitfield);
        mask = 1U << target_index;
        // Check if field at given index is marked valid, convert to bool.
        valid = !! (target->valid.bitfield & mask);
    }

    return valid;
}

rd_sensor_data_bitfield_t rd_sensor_field_type (const rd_sensor_data_t * const target,
        const uint8_t index)
{
    uint8_t fieldcount = 0;
    rd_sensor_data_fields_t check = {0};
    uint8_t target_index = 0U;
    uint32_t mask = 0U;

    if (NULL != target)
    {
        fieldcount = rd_sensor_data_fieldcount (target);
    }

    // Verify bounds
    if (fieldcount > index)
    {
        check = target->fields;

        // Find target field
        for (uint8_t ii = 0U; ii < index; ii++)
        {
            // Null trailing fields
            target_index = (uint8_t) __builtin_ctz (check.bitfield);
            mask = 1U << target_index;
            check.bitfield &= ~mask;
        }

        target_index = (uint8_t) __builtin_ctz (check.bitfield);
        mask = 1U << target_index;
        check.bitfield &= mask;
    }

    // return given bitfield
    return check.datas;
}

void rd_sensor_data_populate (rd_sensor_data_t * const target,
                              const rd_sensor_data_t * const provided,
                              const rd_sensor_data_fields_t requested)
{
    if ( (NULL != target) && (NULL != provided))
    {
        // Compare provided data to requested data not yet valid.
        rd_sensor_data_fields_t available =
        {
            .bitfield = (provided->valid).bitfield
            & (requested.bitfield & ~ (target->valid.bitfield))
        };

        // Update timestamp if there is not already a valid timestamp
        if ( (0 != available.bitfield)
                && ( (0 == target->timestamp_ms)
                     || (RD_SENSOR_INVALID_TIMSTAMP == target->timestamp_ms)))
        {
            target->timestamp_ms = provided->timestamp_ms;
        }

        // We have the available, requested fields. Fill the target struct with those
        while (available.bitfield)
        {
            // read rightmost field
            const uint8_t index = (uint8_t) __builtin_ctz (available.bitfield);

            if (index < sizeof (requested.bitfield) * BITS_PER_BYTE)
            {
                rd_sensor_data_fields_t next =
                {
                    .bitfield = (1U << index)
                };
                float value = rd_sensor_data_parse (provided, next);
                rd_sensor_data_set (target, next, value);
            }

            // set rightmost bit of available to 0
            available.bitfield &= (available.bitfield - 1U);
        }
    }
}

inline uint8_t rd_sensor_data_fieldcount (const rd_sensor_data_t * const target)
{
    return __builtin_popcount (target->fields.bitfield);
}

rd_status_t validate_default_input_set (uint8_t * const input, const uint8_t mode)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == input)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        if (RD_SENSOR_CFG_SLEEP != mode)
        {
            err_code = RD_ERROR_INVALID_STATE;
        }
        else
        {
            if (RD_SENSOR_CFG_DEFAULT == (*input))
            {
                (*input) = RD_SENSOR_CFG_DEFAULT;
            }
            else if (RD_SENSOR_CFG_NO_CHANGE == (*input))
            {
                (*input) = RD_SENSOR_CFG_DEFAULT;
            }
            else if (RD_SENSOR_CFG_MIN == (*input))
            {
                (*input) = RD_SENSOR_CFG_DEFAULT;
            }
            else if (RD_SENSOR_CFG_MAX == (*input))
            {
                (*input) = RD_SENSOR_CFG_DEFAULT;
            }
            else
            {
                (*input) = RD_SENSOR_CFG_DEFAULT;
                err_code = RD_ERROR_NOT_SUPPORTED;
            }
        }
    }

    return err_code;
}

rd_status_t validate_default_input_get (uint8_t * const input)
{
    rd_status_t err_code = RD_SUCCESS;

    if (NULL == input)
    {
        err_code = RD_ERROR_NULL;
    }
    else
    {
        (*input) = RD_SENSOR_CFG_DEFAULT;
    }

    return err_code;
}

bool rd_sensor_is_init (const rd_sensor_t * const sensor)
{
    bool init = false;

    if ( (NULL != sensor) && (NULL != sensor->uninit))
    {
        init = (sensor->uninit != &rd_init_ni);
    }

    return init;
}