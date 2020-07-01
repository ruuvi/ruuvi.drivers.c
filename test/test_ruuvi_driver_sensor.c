#include "unity.h"

#include "ruuvi_driver_sensor.h"

#include <string.h>

#define NUM_FIELDS (7U) //!< Total of different fields
#define ACCX_INDEX (0U) //!< Index of acceleration X
#define ACCY_INDEX (1U) //!< Index of acceleration Y 
#define ACCZ_INDEX (2U) //!< Index of acceleration Z
#define HUMI_INDEX (3U) //!< Index of humidity
#define LUMI_INDEX (4U) //!< Index of luminosity
#define PRES_INDEC (5U) //!< Index of pressure
#define TEMP_INDEX (6U) //!< Index of temperature


const float bme280_data[]   = {34.53F, 99687.01F, 20.45F};
const float shtcx_data[]    = {35.55F, 21.12F};
const float dps310_data[]   = {99720.15F, 19.96F};
const float lis2dh12_data[] = {0.014F, -0.14F, 1.064F, 24.65F};
const float photodiode_data[] = {2104.0F};

const rd_sensor_data_fields_t field_accx =
{
    .datas.acceleration_x_g = 1
};

const rd_sensor_data_fields_t field_accy =
{
    .datas.acceleration_y_g = 1
};

const rd_sensor_data_fields_t field_accz =
{
    .datas.acceleration_z_g = 1
};

const rd_sensor_data_fields_t field_humi =
{
    .datas.humidity_rh = 1
};

const rd_sensor_data_fields_t field_photo =
{
    .datas.luminosity = 1
};

const rd_sensor_data_fields_t field_pres =
{
    .datas.pressure_pa = 1
};

const rd_sensor_data_fields_t field_temp =
{
    .datas.temperature_c = 1
};

const rd_sensor_data_fields_t bme280_provided =
{
    .datas.humidity_rh = 1,
    .datas.pressure_pa = 1,
    .datas.temperature_c = 1
};

const rd_sensor_data_fields_t shtcx_provided =
{
    .datas.humidity_rh = 1,
    .datas.temperature_c = 1
};

const rd_sensor_data_fields_t dps310_provided =
{
    .datas.pressure_pa = 1,
    .datas.temperature_c = 1
};

const rd_sensor_data_fields_t lis2dh12_provided =
{
    .datas.acceleration_x_g = 1,
    .datas.acceleration_y_g = 1,
    .datas.acceleration_z_g = 1,
    .datas.temperature_c = 1
};

const rd_sensor_data_fields_t photo_provided =
{
    .datas.luminosity = 1
};

const rd_sensor_data_fields_t bme_photo_provided =
{
    .datas.humidity_rh = 1,
    .datas.pressure_pa = 1,
    .datas.temperature_c = 1,
    .datas.luminosity = 1
};

const rd_sensor_data_fields_t all_provided =
{
    .datas.acceleration_x_g = 1,
    .datas.acceleration_y_g = 1,
    .datas.acceleration_z_g = 1,
    .datas.humidity_rh = 1,
    .datas.luminosity = 1,
    .datas.pressure_pa = 1,
    .datas.temperature_c = 1
};

void setUp (void)
{
}

void tearDown (void)
{
}

// Return humidity, temperature, pressure.
static void mock_bme (rd_sensor_data_t * const target)
{
    rd_sensor_data_set (target,
                        field_humi,
                        bme280_data[0]);
    rd_sensor_data_set (target,
                        field_pres,
                        bme280_data[1]);
    rd_sensor_data_set (target,
                        field_temp,
                        bme280_data[2]);
}

// Return humidity, temperature.
static void mock_shtcx (rd_sensor_data_t * const target)
{
    rd_sensor_data_set (target,
                        field_humi,
                        shtcx_data[0]);
    rd_sensor_data_set (target,
                        field_temp,
                        shtcx_data[1]);
}

// return temperature, pressure.
void mock_dps310 (rd_sensor_data_t * const target)
{
    rd_sensor_data_set (target,
                        field_pres,
                        dps310_data[0]);
    rd_sensor_data_set (target,
                        field_temp,
                        dps310_data[1]);
}

// return acceleration, temperature.
void mock_lis2dh12 (rd_sensor_data_t * const target)
{
    rd_sensor_data_set (target,
                        field_accx,
                        lis2dh12_data[0]);
    rd_sensor_data_set (target,
                        field_accy,
                        lis2dh12_data[1]);
    rd_sensor_data_set (target,
                        field_accz,
                        lis2dh12_data[2]);
    rd_sensor_data_set (target,
                        field_temp,
                        lis2dh12_data[3]);
}

// return luminosity
void mock_photo (rd_sensor_data_t * const target)
{
    rd_sensor_data_set (target,
                        field_photo,
                        photodiode_data[0]);
}

/**
 * @brief Populate given target data with data provided by sensor as requested.
 *
 * This function looks up the appropriate assigments on each data field in given target
 * and populates it with provided data if caller requested the field to be populated.
 * Populated fields are marked as valid.
 *
 * Example: Board can have these sensors in this order of priority:
 *  - TMP117 (temperature)
 *  - SHTC3 (temperature, humidity)
 *  - DPS310 (temperature, pressure)
 *  - LIS2DH12 (acceleration, temperature)
 *
 * If a target with fields for temperature, humidity, pressure and acceleration is
 * created and populated from data of the sensors end result will be:
 *
 * -> Temperature from TMP117
 * -> Humidity from SHTC3
 * -> Pressure from DPS310
 * -> Acceleration from LIS2DH12
 *
 * If same firmware is run on a board with only LIS2DH12 populated, end result will be
 *
 * -> Temperature, acceleration from LIS2DH12
 * -> RD_FLOAT_INVALID on humidity and pressure.
 *
 *
 *
 * @param[out] target Data to be populated. Fields must be initially populated with
 *                    RD_FLOAT_INVALID.
 * @param[in]  provided Data provided by sensor.
 * @param[in]  requested Fields to be filled if possible.
 */
void test_ruuvi_driver_sensor_populate_one (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);
    TEST_ASSERT (!memcmp (&bme280_provided, &data.valid, sizeof (bme280_provided)));
    TEST_ASSERT (values[HUMI_INDEX] == bme_data.data[0]);
    TEST_ASSERT (values[PRES_INDEC] == bme_data.data[1]);
    TEST_ASSERT (values[TEMP_INDEX] == bme_data.data[2]);
    TEST_ASSERT (data.timestamp_ms == bme_data.timestamp_ms);
}

void test_ruuvi_driver_sensor_populate_null (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    rd_sensor_data_fields_t no_fields = {0};
    mock_bme (&bme_data);
    // All of these should return without effect.
    rd_sensor_data_populate (&data,
                             NULL,
                             all_provided);
    rd_sensor_data_populate (NULL,
                             &bme_data,
                             all_provided);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             no_fields);
    TEST_ASSERT (data.timestamp_ms != bme_data.timestamp_ms);
}


void test_ruuvi_driver_sensor_populate_two_no_overlap (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    float photo_values[1] = {0};
    rd_sensor_data_t photo_data = {0};
    photo_data.fields = photo_provided;
    photo_data.data = photo_values;
    photo_data.timestamp_ms = 2;
    mock_photo (&photo_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);
    rd_sensor_data_populate (&data,
                             &photo_data,
                             all_provided);
    TEST_ASSERT (!memcmp (&bme_photo_provided, &data.valid, sizeof (bme_photo_provided)));
    TEST_ASSERT (values[HUMI_INDEX] == bme_data.data[0]);
    TEST_ASSERT (values[PRES_INDEC] == bme_data.data[1]);
    TEST_ASSERT (values[TEMP_INDEX] == bme_data.data[2]);
    TEST_ASSERT (values[LUMI_INDEX] == photo_data.data[0]);
    TEST_ASSERT (data.timestamp_ms == bme_data.timestamp_ms);
}


void test_ruuvi_driver_sensor_populate_one_no_match (void)
{
    rd_sensor_data_fields_t no_fields = {0};
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             photo_provided);
    TEST_ASSERT (!memcmp (&no_fields, &data.valid, sizeof (data.valid)));
}

void test_ruuvi_driver_sensor_populate_one_some_match (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             lis2dh12_provided);
    TEST_ASSERT (!memcmp (&field_temp, &data.valid, sizeof (data.valid)));
}

void test_ruuvi_driver_sensor_populate_three_overlap (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float shtcx_values[2] = {0};
    rd_sensor_data_t shtcx_data = {0};
    shtcx_data.fields = shtcx_provided;
    shtcx_data.data = shtcx_values;
    shtcx_data.timestamp_ms = 1;
    mock_shtcx (&shtcx_data);
    float acc_values[4] = {0};
    rd_sensor_data_t acc_data = {0};
    acc_data.fields = lis2dh12_provided;
    acc_data.data = acc_values;
    acc_data.timestamp_ms = 2;
    mock_lis2dh12 (&acc_data);
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 3;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &shtcx_data,
                             all_provided);
    rd_sensor_data_populate (&data,
                             &acc_data,
                             all_provided);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);
    TEST_ASSERT (values[HUMI_INDEX] == shtcx_data.data[0]);
    TEST_ASSERT (values[PRES_INDEC] == bme_data.data[1]);
    TEST_ASSERT (values[TEMP_INDEX] == shtcx_data.data[1]);
    TEST_ASSERT (values[ACCX_INDEX] == acc_data.data[0]);
    TEST_ASSERT (values[ACCY_INDEX] == acc_data.data[1]);
    TEST_ASSERT (values[ACCZ_INDEX] == acc_data.data[2]);
    TEST_ASSERT (data.timestamp_ms == shtcx_data.timestamp_ms);
}

/**
 * @brief Parse data from provided struct.
 *
 * @param[in]  provided Data to be parsed.
 * @param[in]  requested One data field to be parsed.
 * @return     sensor value if found, RD_FLOAT_INVALID if the provided data didn't
 *             have a valid value.
 */
void test_ruuvi_driver_sensor_parse_match (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);
    const float pressure = rd_sensor_data_parse (&data, field_pres);
    TEST_ASSERT (pressure == bme280_data[1]);
}

void test_ruuvi_driver_sensor_parse_no_match (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);
    const float pressure = rd_sensor_data_parse (&data, field_accz);
    TEST_ASSERT (isnan (pressure));
}

void test_ruuvi_driver_sensor_parse_null (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);
    const float pressure = rd_sensor_data_parse (NULL, field_accz);
    TEST_ASSERT (isnan (pressure));
}

void test_ruuvi_driver_sensor_parse_excess (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);
    const float pressure = rd_sensor_data_parse (&data, bme280_provided);
    TEST_ASSERT (isnan (pressure));
}

void test_ruuvi_driver_sensor_parse_null_excess (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme (&bme_data);
    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);
    const float pressure = rd_sensor_data_parse (NULL, bme280_provided);
    TEST_ASSERT (isnan (pressure));
}


/**
 * @brief Count number of floats required for this data structure.
 *
 * @param[in]  target Structure to count number of fields from.
 * @return     Number of floats required to store the sensor data.
 */
void test_ruuvi_driver_sensor_count_one_field (void)
{
    rd_sensor_data_t data = {0};
    data.fields = photo_provided;
    const uint8_t fc = rd_sensor_data_fieldcount (&data);
    TEST_ASSERT (1 == fc);
}

void test_ruuvi_driver_sensor_count_no_field (void)
{
    rd_sensor_data_t data = {0};
    const uint8_t fc = rd_sensor_data_fieldcount (&data);
    TEST_ASSERT (0 == fc);
}

void test_ruuvi_driver_sensor_count_many_field (void)
{
    rd_sensor_data_t data = {0};
    data.fields = all_provided;
    const uint8_t fc = rd_sensor_data_fieldcount (&data);
    TEST_ASSERT (NUM_FIELDS == fc);
}

/**
 * @brief Set a desired value to target data.
 *
 * This function looks up the appropriate assigments on each data field in given target
 * and populates it with provided data. Does nothing if there is no appropriate slot
 * in target data.
 *
 * This is a shorthand for @ref rd_sensor_data_populate for only one data field, without
 * setting timestamp.
 *
 * @param[out] target
 * @param[in]  field  Quantity to set, exactly one must be set to true.
 * @param[in]  value  Value of quantity,
 */
void test_ruuvi_driver_sensor_data_set_one_field (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    data.timestamp_ms = 0;
    rd_sensor_data_set (&data,
                        field_photo,
                        photodiode_data[0]);
    TEST_ASSERT (values[LUMI_INDEX] == photodiode_data[0]);
    TEST_ASSERT (0 == data.timestamp_ms);
}

void test_ruuvi_driver_sensor_data_set_many_field_error (void)
{
    float values[NUM_FIELDS] = {0};
    float zeroes[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    rd_sensor_data_set (&data,
                        bme_photo_provided,
                        photodiode_data[0]);
    TEST_ASSERT (!memcmp (values, zeroes, sizeof (values)));
}

void test_ruuvi_driver_sensor_data_set_no_field_error (void)
{
    float values[NUM_FIELDS] = {0};
    float zeroes[NUM_FIELDS] = {0};
    rd_sensor_data_fields_t no_fields = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    rd_sensor_data_set (&data,
                        no_fields,
                        photodiode_data[0]);
    TEST_ASSERT (!memcmp (values, zeroes, sizeof (values)));
}

void test_ruuvi_driver_sensor_data_set_null (void)
{
    float values[NUM_FIELDS] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;
    rd_sensor_data_set (NULL,
                        field_photo,
                        photodiode_data[0]);
    // Just verify NULL doesn't crash, no ASSERT required.
}

void test_ruuvi_driver_sensor_data_set_no_match (void)
{
    float values[3] = {0};
    float zeroes[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = values;
    rd_sensor_data_set (&bme_data,
                        field_photo,
                        photodiode_data[0]);
    TEST_ASSERT (!memcmp (values, zeroes, sizeof (values)));
}

void test_validate_default_input_get_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = validate_default_input_get (NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_validate_default_input_get_ok (void)
{
    uint8_t input = 54;
    rd_status_t err_code = validate_default_input_get (&input);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == input);
}

void test_validate_default_input_set_default (void)
{
    uint8_t input = RD_SENSOR_CFG_DEFAULT;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    rd_status_t err_code = validate_default_input_set (&input, mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == input);
}

void test_validate_default_input_set_min (void)
{
    uint8_t input = RD_SENSOR_CFG_MIN;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    rd_status_t err_code = validate_default_input_set (&input, mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == input);
}

void test_validate_default_input_set_max (void)
{
    uint8_t input = RD_SENSOR_CFG_MAX;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    rd_status_t err_code = validate_default_input_set (&input, mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == input);
}

void test_validate_default_input_set_nochange (void)
{
    uint8_t input = RD_SENSOR_CFG_NO_CHANGE;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    rd_status_t err_code = validate_default_input_set (&input, mode);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == input);
}

void test_validate_default_input_set_wrongmode (void)
{
    uint8_t input = RD_SENSOR_CFG_NO_CHANGE;
    uint8_t mode = RD_SENSOR_CFG_CONTINUOUS;
    rd_status_t err_code = validate_default_input_set (&input, mode);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_NO_CHANGE == input);
}

void test_validate_default_input_set_null (void)
{
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    rd_status_t err_code = validate_default_input_set (NULL, mode);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_validate_default_input_set_notsupported (void)
{
    uint8_t input = 50;
    uint8_t mode = RD_SENSOR_CFG_SLEEP;
    rd_status_t err_code = validate_default_input_set (&input, mode);
    TEST_ASSERT (RD_ERROR_NOT_SUPPORTED == err_code);
    TEST_ASSERT (RD_SENSOR_CFG_DEFAULT == input);
}