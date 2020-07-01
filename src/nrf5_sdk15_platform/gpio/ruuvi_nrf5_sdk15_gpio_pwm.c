/**
 * @addtogroup GPIO
 * @{
 */
/**
 * @file ruuvi_interface_gpio_pwm.h
 * @author Oleg Protasevich
 * @date 2020-05-27
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Pulse Width Modulation control.
 */
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_gpio_pwm.h"
#if (RUUVI_NRF5_SDK15_GPIO_PWM_ENABLED || DOXYGEN || CEEDLING)
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"

#include <stdbool.h>
#include "nrf_pwm.h"
#include "nrf_drv_pwm.h"

#define RI_GPIO_PWM_DRIVER                  0
#define RI_GPIO_PWM_PLAYBACK_OK             (0)
#define RI_GPIO_PWM_PLAYBACK_COUNT          (1)
#define RI_GPIO_PWM_DRIVER_PIN_POINT        (0)
#define RI_GPIO_PWM_SEQ_DELAY               (0)
#define RI_GPIO_PWM_SEQ_REPEATS             (1)

#define RI_GPIO_PWM_MAX_FREQ                (16000000.00f)
#define RI_GPIO_PWM_MIN_FREQ                (2.00f)

#define RI_GPIO_PWM_MAX_DUTY                (1.00f)
#define RI_GPIO_PWM_MIN_DUTY                (0.00f)

#define RI_GPIO_PWM_CHANNEL_UNUSED          (0)

#define RI_GPIO_PWM_MIN_TOP_VALUE           (1U)
#define RI_GPIO_PWM_MIN_REST_FOR_TOP        (0.999999f)

#define RI_GPIO_PWM_FREQ_COUNT              (8)

#define RI_GPIO_PWM_BASE_FREQ_16MHZ         (RI_GPIO_PWM_MAX_FREQ)
#define RI_GPIO_PWM_BASE_FREQ_8MHZ          (8000000.00f)
#define RI_GPIO_PWM_BASE_FREQ_4MHZ          (4000000.00f)
#define RI_GPIO_PWM_BASE_FREQ_2MHZ          (2000000.00f)
#define RI_GPIO_PWM_BASE_FREQ_1MHZ          (1000000.00f)
#define RI_GPIO_PWM_BASE_FREQ_500KHZ        (500000.00f)
#define RI_GPIO_PWM_BASE_FREQ_250KHZ        (250000.00f)
#define RI_GPIO_PWM_BASE_FREQ_125KHZ        (125000.00f)

/** @brief flag to keep track on if PWM is initialized */
static bool m_gpio_pwm_is_init = false;
static bool m_gpio_pwm_is_start = false;
static nrf_drv_pwm_t m_pwm = NRF_DRV_PWM_INSTANCE (RI_GPIO_PWM_DRIVER);

static float const freq[RI_GPIO_PWM_FREQ_COUNT] =
{
    RI_GPIO_PWM_BASE_FREQ_16MHZ,
    RI_GPIO_PWM_BASE_FREQ_8MHZ,
    RI_GPIO_PWM_BASE_FREQ_4MHZ,
    RI_GPIO_PWM_BASE_FREQ_2MHZ,
    RI_GPIO_PWM_BASE_FREQ_1MHZ,
    RI_GPIO_PWM_BASE_FREQ_500KHZ,
    RI_GPIO_PWM_BASE_FREQ_250KHZ,
    RI_GPIO_PWM_BASE_FREQ_125KHZ
};

typedef enum
{
    RI_GPIO_PWM_BASE_NUM_FREQ_16MHZ   = 0,
    RI_GPIO_PWM_BASE_NUM_FREQ_8MHZ    = 1,
    RI_GPIO_PWM_BASE_NUM_FREQ_4MHZ    = 2,
    RI_GPIO_PWM_BASE_NUM_FREQ_2MHZ    = 3,
    RI_GPIO_PWM_BASE_NUM_FREQ_1MHZ    = 4,
    RI_GPIO_PWM_BASE_NUM_FREQ_500KHZ  = 5,
    RI_GPIO_PWM_BASE_NUM_FREQ_250KHZ  = 6,
    RI_GPIO_PWM_BASE_NUM_FREQ_125KHZ  = 7,
} ri_gpio_base_freq;


rd_status_t ri_gpio_pwm_init (void)
{
    rd_status_t res = RD_ERROR_INVALID_STATE;

    if (!m_gpio_pwm_is_init)
    {
        m_gpio_pwm_is_init = true;
        res = RD_SUCCESS;
    }

    return res;
}

rd_status_t ri_gpio_pwm_uninit (void)
{
    rd_status_t res = RD_ERROR_INVALID_STATE;

    if (false == m_gpio_pwm_is_init)
    {
        res = RD_SUCCESS;
    }
    else
    {
        m_gpio_pwm_is_init = false;
        res = RD_SUCCESS;
    }

    return res;
}

bool  ri_gpio_pwm_is_init (void)
{
    return m_gpio_pwm_is_init;
}

static bool  ri_gpio_pwm_is_start (void)
{
    return m_gpio_pwm_is_start;
}


/**
 * @brief convert @ref ri_gpio_id_t to nRF GPIO.
 */
static inline uint8_t ruuvi_to_nrf_pin_pwm_map (const ri_gpio_id_t pin)
{
    return ( (pin >> 3) & 0xE0) + (pin & 0x1F);
}

static nrf_pwm_clk_t ruuvi_get_base_config (float * const frequency,
        float * const duty_cycle,
        uint16_t * p_top)
{
    nrf_pwm_clk_t clock = NRF_PWM_CLK_16MHz;
    float f_rest_min = RI_GPIO_PWM_MIN_REST_FOR_TOP;
    float m_freq = (*frequency);
    float m_duty = (*duty_cycle);
    float m_top;

    for (uint8_t i = 0; i < RI_GPIO_PWM_FREQ_COUNT ; i++)
    {
        if (freq[i] >= m_freq)
        {
            float f_rest = (freq[i] / m_freq) -
                           (uint32_t) (freq[i] / m_freq);

            if (f_rest_min >= f_rest)
            {
                m_top = (freq[i] / m_freq) * m_duty;

                if (m_top  >= RI_GPIO_PWM_MIN_TOP_VALUE)
                {
                    f_rest_min = f_rest;
                    clock = (nrf_pwm_clk_t) i;
                    (*p_top) = (freq[i] / m_freq);
                }
            }
        }
    }

    return clock;
}

rd_status_t ri_gpio_pwm_start (const ri_gpio_id_t pin, const ri_gpio_mode_t mode,
                               float * const frequency, float * const duty_cycle)
{
    rd_status_t res = RD_ERROR_INVALID_STATE;

    if ( (true == ri_gpio_pwm_is_init()) &&
            (true == ri_gpio_is_init()))
    {
        if (true == ri_gpio_pwm_is_start ())
        {
            if (RD_ERROR_INVALID_STATE == ri_gpio_pwm_stop (pin))
            {
                return RD_ERROR_INVALID_STATE;
            }
        }

        if ( (duty_cycle == NULL) || (frequency == NULL))
        {
            res = RD_ERROR_NULL;
        }
        else
        {
            if ( (RI_GPIO_PWM_MIN_DUTY > (*duty_cycle)) ||
                    (RI_GPIO_PWM_MAX_DUTY < (*duty_cycle)) ||
                    (RI_GPIO_PWM_MAX_FREQ < (*frequency)) ||
                    (RI_GPIO_PWM_MIN_FREQ > (*frequency)) ||
                    ( (RI_GPIO_MODE_OUTPUT_STANDARD != mode) &&
                      (RI_GPIO_MODE_OUTPUT_HIGHDRIVE != mode)))
            {
                res = RD_ERROR_INVALID_PARAM;
            }
            else
            {
                uint16_t top = RI_GPIO_PWM_MIN_TOP_VALUE;
                nrf_pwm_clk_t clock = ruuvi_get_base_config (frequency, duty_cycle, &top);
                uint8_t out_pin = (ruuvi_to_nrf_pin_pwm_map (pin) | NRF_DRV_PWM_PIN_INVERTED);
                nrf_drv_pwm_config_t config = NRF_DRV_PWM_DEFAULT_CONFIG;
                config.output_pins[RI_GPIO_PWM_DRIVER_PIN_POINT] = out_pin;
                config.base_clock = clock;
                config.top_value = top;

                if ( (NRF_SUCCESS == ri_gpio_configure (pin, mode))  &&
                        (NRF_SUCCESS == nrf_drv_pwm_init (&m_pwm, &config, NULL)))
                {
                    uint16_t steps_duty  = (uint16_t) (top * (*duty_cycle));
                    static nrf_pwm_values_individual_t seq_values;
                    seq_values.channel_0 = steps_duty;
                    seq_values.channel_1 = RI_GPIO_PWM_CHANNEL_UNUSED;
                    seq_values.channel_2 = RI_GPIO_PWM_CHANNEL_UNUSED;
                    seq_values.channel_3 = RI_GPIO_PWM_CHANNEL_UNUSED;
                    nrf_pwm_sequence_t const seq =
                    {
                        .values.p_individual = &seq_values,
                        .length          = NRF_PWM_VALUES_LENGTH (seq_values),
                        .repeats         = RI_GPIO_PWM_SEQ_REPEATS,
                        .end_delay       = RI_GPIO_PWM_SEQ_DELAY
                    };

                    if (RI_GPIO_PWM_PLAYBACK_OK == nrf_drv_pwm_simple_playback (&m_pwm, &seq,
                            RI_GPIO_PWM_PLAYBACK_COUNT,
                            NRF_DRV_PWM_FLAG_LOOP))
                    {
                        res = RD_SUCCESS;
                        m_gpio_pwm_is_start = true;
                    }
                }
            }
        }
    }

    return res;
}

rd_status_t ri_gpio_pwm_stop (const ri_gpio_id_t pin)
{
    rd_status_t res = RD_ERROR_INVALID_STATE;
    bool wait = true;

    if (true == nrf_drv_pwm_stop (&m_pwm, wait))
    {
        nrf_drv_pwm_uninit (&m_pwm);
        m_gpio_pwm_is_start = false;
        res = RD_SUCCESS;
    }

    return res;
}
/** @} */
#endif
