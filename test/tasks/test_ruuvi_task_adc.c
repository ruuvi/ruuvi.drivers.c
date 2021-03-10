#include "unity.h"

#include "ruuvi_driver_enabled_modules.h"

#include "ruuvi_task_adc.h"
#include "ruuvi_driver_error.h"

#include "mock_ruuvi_interface_adc_mcu.h"
#include "mock_ruuvi_interface_atomic.h"
#include "mock_ruuvi_interface_log.h"
#include "mock_ruuvi_driver_sensor.h"

static volatile ri_atomic_t m_true = true;
static volatile ri_atomic_t m_false = false;

static float m_valid_data[2] = {2.8F, 0.6F};
static rd_sensor_data_t m_adc_data =
{
    .data                       = m_valid_data,
    .fields.datas.voltage_v     = 1,
    .fields.datas.voltage_ratio = 1,
    .valid.datas.voltage_v      = 1,
    .valid.datas.voltage_ratio  = 1
};

void setUp (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_atomic_flag_ExpectAnyArgsAndReturn (true);
    ri_atomic_flag_ReturnThruPtr_flag (&m_true);
    ri_adc_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = rt_adc_init();
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (rt_adc_is_init());
}

void tearDown (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adc_stop_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_adc_uninit_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_atomic_flag_ExpectAnyArgsAndReturn (true);
    ri_atomic_flag_ReturnThruPtr_flag (&m_false);
    err_code = rt_adc_uninit();
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (!rt_adc_is_init());
}

/**
 * @brief Initialize ADC in low-power state.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC is already initialized.
 */
void test_rt_adc_init_ok (void)
{
    tearDown();
    setUp();
}

void test_rt_adc_init_busy (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    ri_atomic_flag_ExpectAnyArgsAndReturn (false);
    err_code = rt_adc_init();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
    TEST_ASSERT (!rt_adc_is_init());
}

/**
 * @brief Uninitialize ADC to release it for other users.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_FATAL if ADC lock can't be released. Reboot.
 */
void test_rt_adc_uninit_ok (void)
{
    tearDown();
}

/**
 * @brief Configure ADC before sampling
 *
 * This function readies the ADC for sampling.
 * Configuring the ADC may take some time (< 1 ms) while actual sample must be as fast.
 * as possible to catch transients.
 *
 * <b>Note:</b> ADC should be configured to sleep or continuous mode. To take a single sample,
 * call @ref rt_adc_sample_se after configuration. Configuring ADC into single sample mode is
 * equivalent to configuring ADC into sleep and then calling @ref rt_adc_sample_se immediately
 *
 * @param[in, out] config Configuration of ADC.
 * @param[in] handle Handle to ADC, i.e. ADC pin.
 * @param[in] mode sampling mode, absolute or ratiometric
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_INVALID_STATE if ADC is not initialized or if it is already configured.
 */
void test_rt_adc_configure_se_ratiometric_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adc_mcu_is_valid_ch_ExpectAndReturn (0, true);
    ri_adc_configure_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_sensor_configuration_t config = {0};
    err_code = rt_adc_configure_se (&config, RI_ADC_AINVDD, RATIOMETRIC);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adc_configure_se_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    ri_adc_mcu_is_valid_ch_ExpectAndReturn (0, true);
    ri_adc_configure_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_sensor_configuration_t config = {0};
    err_code = rt_adc_configure_se (&config, RI_ADC_AINVDD, ABSOLUTE);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adc_configure_se_invalid_handle (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_configuration_t config = {0};
    err_code = rt_adc_configure_se (&config, RI_ADC_GND, ABSOLUTE);
    TEST_ASSERT (RD_ERROR_INVALID_PARAM == err_code);
}

void test_rt_adc_configure_se_twice (void)
{
    test_rt_adc_configure_se_ok();
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_configuration_t config = {0};
    err_code = rt_adc_configure_se (&config, RI_ADC_AINVDD, ABSOLUTE);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_adc_configure_se_not_init (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_configuration_t config = {0};
    err_code = rt_adc_configure_se (&config, RI_ADC_AINVDD, ABSOLUTE);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Take a new sample on ADC configured in single-shot/sleep mode
 *
 * If this function returns RD_SUCCESS new sample can be immediately read
 * with rt_adc_voltage_get or rt_adc_ratio_get
 *
 * @retval RD_SUCCESS Sampling was successful
 * @retval RD_ERROR_INVALID_STATE ADC is not initialized or configured
 */
void test_rt_adc_sample_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    test_rt_adc_configure_se_ok();
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    err_code = rt_adc_sample();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adc_sample_not_configured (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_adc_sample();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_adc_sample_not_initialized (void)
{
    tearDown();
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_adc_sample();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Populate data with latest sample.
 *
 * The data is absolute voltage relative to device ground.
 *
 * @param[in] handle Handle for ADC peripheral, e.g. ADC number
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if ADC is not initialized or configured.
 * @retval error code from stack on error.
 */
void test_rt_adc_voltage_get_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float data[2] = {0};
    rd_sensor_data_t adc_data;
    adc_data.data = data;
    adc_data.fields.datas.voltage_v = 1;
    test_rt_adc_sample_ok();
    ri_adc_get_data_absolute_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_sensor_data_populate_ExpectAnyArgs ();
    rd_sensor_data_populate_ReturnThruPtr_target (&m_adc_data);
    rd_sensor_timestamp_get_IgnoreAndReturn (0);
    err_code = rt_adc_voltage_get (&adc_data);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (true == adc_data.valid.datas.voltage_v);
}

/**
 * @brief Prepare for sampling VDD
 *
 * This function should be called before entering energy intensive activity, such as using radio to transmit data.
 * After calling this function ADC is primed for measuring the voltage droop of battery.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if ADC is already initialized.
 */
void test_rt_adc_vdd_prepare_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_configuration_t config = {0};
    tearDown();
    ri_atomic_flag_ExpectAnyArgsAndReturn (true);
    ri_atomic_flag_ReturnThruPtr_flag (&m_true);
    ri_adc_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_adc_mcu_is_valid_ch_ExpectAndReturn (0, true);
    ri_adc_configure_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = rt_adc_vdd_prepare (&config);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adc_vdd_prepare_already_init (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_configuration_t config = {0};
    tearDown();
    ri_atomic_flag_ExpectAnyArgsAndReturn (true);
    ri_atomic_flag_ReturnThruPtr_flag (&m_false);
    ri_adc_init_ExpectAnyArgsAndReturn (RD_ERROR_INVALID_STATE);
    err_code = rt_adc_vdd_prepare (&config);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_rt_adc_vdd_prepare_configuration_fail (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_configuration_t config = {0};
    tearDown();
    ri_atomic_flag_ExpectAnyArgsAndReturn (true);
    ri_atomic_flag_ReturnThruPtr_flag (&m_true);
    ri_adc_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    err_code = rt_adc_init();
    TEST_ASSERT (RD_SUCCESS == err_code);
    test_rt_adc_configure_se_invalid_handle ();
    tearDown ();
}

/**
 * @brief Sample VDD
 *
 * This function should be called as soon as possible after energy intensive activity.
 * After a successful call value returned by @ref rt_adc_vdd_get is updated and ADC is released.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if rt_adc_vdd_prepare wasn't called.
 */
void test_rt_adc_vdd_sample_ok (void)
{
    test_rt_adc_vdd_prepare_ok();
    rd_status_t err_code = RD_SUCCESS;
    uint8_t mode = RD_SENSOR_CFG_SINGLE;
    ri_adc_get_data_absolute_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_sensor_data_populate_ExpectAnyArgs ();
    rd_sensor_data_populate_ReturnThruPtr_target (&m_adc_data);
    rd_sensor_timestamp_get_IgnoreAndReturn (0);
    rd_sensor_data_parse_ExpectAnyArgsAndReturn (m_valid_data[0]);
    ri_adc_stop_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_adc_uninit_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_atomic_flag_ExpectAnyArgsAndReturn (true);
    ri_atomic_flag_ReturnThruPtr_flag (&m_false);
    err_code = rt_adc_vdd_sample();
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adc_vdd_sample_not_prepared (void)
{
    rd_status_t err_code = RD_SUCCESS;
    err_code = rt_adc_vdd_sample();
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Get VDD
 *
 * This function should be called any time after @ref rt_adc_vdd_prepare.
 * The value returned will remain fixed until next call to @ref rt_adc_vdd_prepare.
 *
 * @param[out] vdd VDD voltage in volts.
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if rt_adc_vdd_sample wasn't called.
 */
void test_rt_adc_vdd_get_ok (void)
{
    test_rt_adc_vdd_prepare_ok();
    test_rt_adc_vdd_sample_ok();
    rd_status_t err_code = RD_SUCCESS;
    float data;
    err_code = rt_adc_vdd_get (&data);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_rt_adc_vdd_get_not_sampled (void)
{
    test_rt_adc_vdd_prepare_ok();
    rd_status_t err_code = RD_SUCCESS;
    float data;
    err_code = rt_adc_vdd_get (&data);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

/**
 * @brief Populate data with latest ratiometric value.
 *
 * The data is ratio between 0.0 (gnd) and 1.0 (VDD). However the implementation is
 * allowed to return negative values and values higher than 1.0 if the real voltage is
 * beyond the supply rails or if differential sample is negative.
 *
 * @retval RD_SUCCESS on success
 * @retval RD_ERROR_INVALID_STATE if ADC is not initialized or configured.
 * @retval error code from stack on error.
 */

void test_rt_adc_ratio_get_ok (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float data[2] = {0};
    rd_sensor_data_t adc_data;
    adc_data.data = data;
    adc_data.fields.datas.voltage_v = 1;
    test_rt_adc_init_ok();
    test_rt_adc_configure_se_ratiometric_ok();
    ri_adc_get_data_ratio_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_sensor_data_populate_ExpectAnyArgs ();
    rd_sensor_data_populate_ReturnThruPtr_target (&m_adc_data);
    rd_sensor_timestamp_get_IgnoreAndReturn (0);
    err_code = rt_adc_ratio_get (&adc_data);
    TEST_ASSERT (RD_SUCCESS == err_code);
    TEST_ASSERT (true == adc_data.valid.datas.voltage_v);
}

void test_rt_adc_ratio_get_fail (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float data[2] = {0};
    rd_sensor_data_t adc_data;
    adc_data.data = data;
    adc_data.fields.datas.voltage_v = 1;
    test_rt_adc_sample_ok();
    err_code = rt_adc_ratio_get (&adc_data);
    TEST_ASSERT (RD_ERROR_INVALID_STATE == err_code);
}

void test_adc_absolute_sample_success (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float data[2] = {0};
    rd_sensor_data_t adc_data;
    adc_data.data = data;
    adc_data.fields.datas.voltage_v = 1;
    float sample;
    rd_sensor_configuration_t configuration =
    {
        .dsp_function = RD_SENSOR_CFG_DEFAULT,
        .dsp_parameter = RD_SENSOR_CFG_DEFAULT,
        .mode = RD_SENSOR_CFG_SINGLE,
        .resolution = RD_SENSOR_CFG_DEFAULT,
        .samplerate = RD_SENSOR_CFG_DEFAULT,
        .scale = RD_SENSOR_CFG_DEFAULT
    };
    ri_atomic_flag_ExpectAnyArgsAndReturn (true);
    ri_adc_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_adc_mcu_is_valid_ch_ExpectAndReturn (0, true);
    ri_adc_configure_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_adc_get_data_absolute_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_sensor_data_populate_ExpectAnyArgs ();
    rd_sensor_data_populate_ReturnThruPtr_target (&m_adc_data);
    rd_sensor_timestamp_get_IgnoreAndReturn (0);
    rd_sensor_data_parse_ExpectAnyArgsAndReturn (m_valid_data[0]);
    err_code = rt_adc_absolute_sample (&configuration, RI_ADC_AIN0, &sample);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_adc_absolute_sample_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_configuration_t configuration = {0};
    err_code = rt_adc_absolute_sample (&configuration, RI_ADC_AIN0, NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_adc_absolute_configuration_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float sample;
    err_code = rt_adc_absolute_sample (NULL, RI_ADC_AIN0, &sample);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_rt_adc_ratiometric_success (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float data[2] = {0};
    rd_sensor_data_t adc_data;
    adc_data.data = data;
    adc_data.fields.datas.voltage_v = 1;
    float sample;
    rd_sensor_configuration_t configuration =
    {
        .dsp_function = RD_SENSOR_CFG_DEFAULT,
        .dsp_parameter = RD_SENSOR_CFG_DEFAULT,
        .mode = RD_SENSOR_CFG_SINGLE,
        .resolution = RD_SENSOR_CFG_DEFAULT,
        .samplerate = RD_SENSOR_CFG_DEFAULT,
        .scale = RD_SENSOR_CFG_DEFAULT
    };
    ri_atomic_flag_ExpectAnyArgsAndReturn (true);
    ri_atomic_flag_ReturnThruPtr_flag (&m_true);
    ri_adc_init_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_adc_mcu_is_valid_ch_ExpectAndReturn (0, true);
    ri_adc_configure_ExpectAnyArgsAndReturn (RD_SUCCESS);
    ri_adc_get_data_ratio_ExpectAnyArgsAndReturn (RD_SUCCESS);
    rd_sensor_data_populate_ExpectAnyArgs ();
    rd_sensor_data_populate_ReturnThruPtr_target (&m_adc_data);
    rd_sensor_timestamp_get_IgnoreAndReturn (0);
    rd_sensor_data_parse_ExpectAnyArgsAndReturn (m_valid_data[0]);
    err_code = rt_adc_ratiometric_sample (&configuration, RI_ADC_AIN0, &sample);
    TEST_ASSERT (RD_SUCCESS == err_code);
}

void test_adc_ratiometric_sample_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    rd_sensor_configuration_t configuration = {0};
    err_code = rt_adc_ratiometric_sample (&configuration, RI_ADC_AIN0, NULL);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}

void test_adc_ratiometric_configuration_null (void)
{
    rd_status_t err_code = RD_SUCCESS;
    float sample;
    err_code = rt_adc_ratiometric_sample (NULL, RI_ADC_AIN0, &sample);
    TEST_ASSERT (RD_ERROR_NULL == err_code);
}