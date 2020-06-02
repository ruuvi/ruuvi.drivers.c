#include "unity.h"

#include "ruuvi_driver_sensor.h"

#include <string.h>

const float bme280_data[]   = {34.53F, 99687.01F, 20.45F};
const float shtcx_data[]    = {35.55F, 21.12F};
const float dps310_data[]   = {99720.15F, 19.96F};
const float lis2dh12_data[] = {0.014F, -0.14F, 1.064F, 24.65F};

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

const rd_sensor_data_fields_t all_provided = 
{
  .datas.acceleration_x_g = 1,
  .datas.acceleration_y_g = 1,
  .datas.acceleration_z_g = 1,
  .datas.humidity_rh = 1,
  .datas.pressure_pa = 1,
  .datas.temperature_c = 1
};

void setUp(void)
{
}

void tearDown(void)
{
}

// Return humidity, temperature, pressure.
static void mock_bme(rd_sensor_data_t * const target)
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
static void mock_shtcx(rd_sensor_data_t * const target)
{
    rd_sensor_data_set (target,
                        field_humi,
                        shtcx_data[0]);
    rd_sensor_data_set (target,
                        field_temp,
                        shtcx_data[1]);
}

// return temperature, pressure.
void mock_dps310(rd_sensor_data_t * const target)
{
    rd_sensor_data_set (target,
                        field_pres,
                        dps310_data[0]);
    rd_sensor_data_set (target,
                        field_temp,
                        dps310_data[1]);
}

// return acceleration, temperature. 
void mock_lis2dh12(rd_sensor_data_t * const target)
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
void test_ruuvi_driver_sensor_populate_one(void)
{
    float values[6] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;

    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;
    mock_bme(&bme_data);

    rd_sensor_data_populate (&data,
                             &bme_data,
                             all_provided);

    TEST_ASSERT(!memcmp(&bme280_provided, &data.valid, sizeof(bme280_provided)));
    TEST_ASSERT(values[3] == bme_data.data[0]);
    TEST_ASSERT(values[4] == bme_data.data[1]);
    TEST_ASSERT(values[5] == bme_data.data[2]);
    TEST_ASSERT(data.timestamp_ms == bme_data.timestamp_ms);

}

void test_ruuvi_driver_sensor_populate_null(void)
{
    float values[6] = {0};
    rd_sensor_data_t data = {0};
    data.data = values;
    data.fields = all_provided;

    float bme_values[3] = {0};
    rd_sensor_data_t bme_data = {0};
    bme_data.fields = bme280_provided;
    bme_data.data = bme_values;
    bme_data.timestamp_ms = 1;

    rd_sensor_data_fields_t no_fields = {0};
    mock_bme(&bme_data);
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
    TEST_ASSERT(data.timestamp_ms != bme_data.timestamp_ms)
}

#if 0
void test_ruuvi_driver_sensor_populate_two_no_overlap(void)
{
void rd_sensor_data_populate (rd_sensor_data_t * const target,
                              const rd_sensor_data_t * const provided,
                              const rd_sensor_data_fields_t requested);
}

void test_ruuvi_driver_sensor_populate_one_no_match(void)
{
void rd_sensor_data_populate (rd_sensor_data_t * const target,
                              const rd_sensor_data_t * const provided,
                              const rd_sensor_data_fields_t requested);
}

void test_ruuvi_driver_sensor_populate_three_overlap(void)
{
void rd_sensor_data_populate (rd_sensor_data_t * const target,
                              const rd_sensor_data_t * const provided,
                              const rd_sensor_data_fields_t requested);
}

/**
 * @brief Parse data from provided struct.
 *
 * @param[in]  provided Data to be parsed.
 * @param[in]  requested One data field to be parsed. 
 * @return     sensor value if found, RD_FLOAT_INVALID if the provided data didn't 
 *             have a valid value.
 */
float rd_sensor_data_parse (const rd_sensor_data_t * const provided,
                            const rd_sensor_data_fields_t requested);

/**
 * @brief Count number of floats required for this data structure.
 *
 * @param[in]  target Structure to count number of fields from.
 * @return     Number of floats required to store the sensor data.
 */
uint8_t rd_sensor_data_fieldcount (const rd_sensor_data_t * const target);

/**
 * @brief Set a desired value to target data.
 *
 * This function looks up the appropriate assigments on each data field in given target
 * and populates it with provided data. Does nothing if there is no appropriate slot
 * in target data.
 *
 * This is a shorthand for @ref rd_sensor_data_populate for only one data field. 
 *
 * @param[out] target 
 * @param[in]  field  Quantity to set, exactly one must be set to true.
 * @param[in]  value  Value of quantity,
 */
void rd_sensor_data_set (rd_sensor_data_t * const target,
                         const rd_sensor_data_fields_t field,
                         const float value);
#endif