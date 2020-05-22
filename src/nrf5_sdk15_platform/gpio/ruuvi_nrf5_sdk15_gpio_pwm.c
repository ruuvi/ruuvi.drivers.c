#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_gpio_interrupt.h"
#if RUUVI_NRF5_SDK15_GPIO_INTERRUPT_ENABLED
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_pwm.h"

#include <stdbool.h>
#include "nrf_pwm.h"
#include "nrf_drv_pwm.h"

#define RI_GPIO_PWM_PLAYBACK_OK       0
#define RI_GPIO_PWM_PLAYBACK_COUNT    1
#define RI_GPIO_PWM_DRIVER            0
#define RI_GPIO_PWM_SEQ_DELAY         0
#define RI_GPIO_PWM_SEQ_REPEATS       1

#define RI_GPIO_PWM_MAX_FREQ          16000000.00f
#define RI_GPIO_PWM_MIN_FREQ          2.00f
#define RI_GPIO_PWM_FREQ_COUNT        8

#define RI_GPIO_PWM_BASE_FREQ_16MHZ   16000000.00f
#define RI_GPIO_PWM_BASE_FREQ_8MHZ    8000000.00f
#define RI_GPIO_PWM_BASE_FREQ_4MHZ    4000000.00f
#define RI_GPIO_PWM_BASE_FREQ_2MHZ    2000000.00f
#define RI_GPIO_PWM_BASE_FREQ_1MHZ    1000000.00f
#define RI_GPIO_PWM_BASE_FREQ_500KHZ  500000.00f
#define RI_GPIO_PWM_BASE_FREQ_250KHZ  250000.00f
#define RI_GPIO_PWM_BASE_FREQ_125KHZ  125000.00f

#define RI_GPIO_PWM_MAX_FREQ_FOR_1_DUTY     160000.00f
#define RI_GPIO_PWM_MAX_FREQ_FOR_10_DUTY    1600000.00f
#define RI_GPIO_PWM_MAX_FREQ_FOR_20_DUTY    3200000.00f
#define RI_GPIO_PWM_MAX_FREQ_FOR_50_DUTY    80000000.00f
#define RI_GPIO_PWM_MAX_FREQ_FOR_100_DUTY   16000000.00f

#define RI_GPIO_PWM_STEPS             100

/** @brief flag to keep track on if GPIO is initialized */
static bool m_gpio_pwm_is_init = false;
static nrf_drv_pwm_t m_pwm = NRF_DRV_PWM_INSTANCE(RI_GPIO_PWM_DRIVER);

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
  RI_GPIO_PWM_BASE_NUM_FREQ_MAX     = 8,
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

/**
 * @brief convert @ref ri_gpio_id_t to nRF GPIO.
 */
static inline uint8_t ruuvi_to_nrf_pin_pwm_map (const ri_gpio_id_t pin)
{
    return ( (pin >> 3) & 0xE0) + (pin & 0x1F);
}

/**
 * @brief convert @ref ri_gpio_id_t to nRF GPIO.
 */
static nrf_pwm_clk_t ruuvi_get_base_clock (float * const frequency,
                                           float * const duty_cycle)
{
  nrf_pwm_clk_t clock = NRF_PWM_CLK_16MHz;
  float true_freq = (*frequency)/(*duty_cycle);
  if (RI_GPIO_PWM_MAX_FREQ > true_freq)
  {
    if (RI_GPIO_PWM_MIN_FREQ >= true_freq)
    {
      clock = NRF_PWM_CLK_125kHz;
    }
    else
    {
      float rest[RI_GPIO_PWM_FREQ_COUNT] = 
      {
          ((freq[RI_GPIO_PWM_BASE_NUM_FREQ_16MHZ]/true_freq) - 
            (uint32_t)(freq[RI_GPIO_PWM_BASE_NUM_FREQ_16MHZ]/true_freq)),
          ((freq[RI_GPIO_PWM_BASE_NUM_FREQ_8MHZ]/true_freq) - 
            (uint32_t)(freq[RI_GPIO_PWM_BASE_NUM_FREQ_8MHZ]/true_freq)),
          ((freq[RI_GPIO_PWM_BASE_NUM_FREQ_4MHZ]/true_freq) - 
            (uint32_t)(freq[RI_GPIO_PWM_BASE_NUM_FREQ_4MHZ]/true_freq)),
          ((freq[RI_GPIO_PWM_BASE_NUM_FREQ_2MHZ]/true_freq) - 
            (uint32_t)(freq[RI_GPIO_PWM_BASE_NUM_FREQ_2MHZ]/true_freq)),
          ((freq[RI_GPIO_PWM_BASE_NUM_FREQ_1MHZ]/true_freq) - 
            (uint32_t)(freq[RI_GPIO_PWM_BASE_NUM_FREQ_1MHZ]/true_freq)),
          ((freq[RI_GPIO_PWM_BASE_NUM_FREQ_500KHZ]/true_freq) - 
            (uint32_t)(freq[RI_GPIO_PWM_BASE_NUM_FREQ_500KHZ]/true_freq)),
          ((freq[RI_GPIO_PWM_BASE_NUM_FREQ_250KHZ]/true_freq) - 
            (uint32_t)(freq[RI_GPIO_PWM_BASE_NUM_FREQ_250KHZ]/true_freq)),
          ((freq[RI_GPIO_PWM_BASE_NUM_FREQ_125KHZ]/true_freq) - 
            (uint32_t)(freq[RI_GPIO_PWM_BASE_NUM_FREQ_125KHZ]/true_freq)),
      };
      float  min_rest = rest[RI_GPIO_PWM_BASE_NUM_FREQ_16MHZ];
      for (uint8_t i = 0; i < RI_GPIO_PWM_FREQ_COUNT; i++)
      {
        if (min_rest > rest[i]){
           min_rest = rest[i];
           clock = (nrf_pwm_clk_t)(i +1);
        }
      }
    }
  }
  return clock;
}

static float ruuvi_get_duty_cycle (float * const frequency)                              
{
  float max_duty_cycle = 0.01f;
  if (RI_GPIO_PWM_MAX_FREQ_FOR_1_DUTY > (*frequency))
  {
    max_duty_cycle = 0.01f;
  }
  else
  if (RI_GPIO_PWM_MAX_FREQ_FOR_10_DUTY > (*frequency))
  {
    max_duty_cycle = 0.1f;
  }
  else
  if (RI_GPIO_PWM_MAX_FREQ_FOR_20_DUTY > (*frequency))
  {
    max_duty_cycle = 0.2f;
  }
  else
  if (RI_GPIO_PWM_MAX_FREQ_FOR_50_DUTY > (*frequency))
  {
    max_duty_cycle = 0.5f;
  }
  else
  if (RI_GPIO_PWM_MAX_FREQ_FOR_100_DUTY > (*frequency))
  {
    max_duty_cycle = 1.00f;
  }
  return max_duty_cycle;
}

static uint16_t ruuvi_get_top_value (nrf_pwm_clk_t clock,
                                     float * const frequency,
                                     float * const duty_cycle)
{
  uint16_t tmp_res = 0;
  float tmp = ((*frequency)/(*duty_cycle));
  float tmp_f = (freq[0]);
  tmp_res = (uint16_t)(tmp_f/tmp);
  return tmp_res;
}

rd_status_t ri_gpio_pwm_start (const ri_gpio_id_t pin, const ri_gpio_mode_t mode, 
                               float * const frequency, float * const duty_cycle)
{
    rd_status_t res = RD_ERROR_INVALID_STATE;
    const float min_duty_cycle = ruuvi_get_duty_cycle(frequency);

    nrf_drv_pwm_config_t const config =
    {
        .output_pins =
        {
            ruuvi_to_nrf_pin_pwm_map(pin) | NRF_DRV_PWM_PIN_INVERTED, // channel 0
            NRF_DRV_PWM_PIN_NOT_USED, // channel 1
            NRF_DRV_PWM_PIN_NOT_USED, // channel 2
            NRF_DRV_PWM_PIN_NOT_USED  // channel 3
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = ruuvi_get_base_clock(frequency, &min_duty_cycle),
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = ruuvi_get_top_value(config.base_clock,frequency,&min_duty_cycle),
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    if (NRF_SUCCESS == nrf_drv_pwm_init(&m_pwm, &config, NULL))
    {
      uint8_t step_counter = 1;
      if (min_duty_cycle == 0.01f){
        step_counter = 100;
      }
      else
      if (min_duty_cycle == 0.1f){
        step_counter = 10;
      }
      else
      if (min_duty_cycle == 0.2f){
        step_counter = 5;
      }
      else
      if (min_duty_cycle == 0.5f){
        step_counter = 2;
      }
            else
      if (min_duty_cycle == 1.00f){
        step_counter = 1;
      }

      uint8_t step_duty  = (uint8_t)((*duty_cycle)/min_duty_cycle);
      static nrf_pwm_values_common_t seq_values[RI_GPIO_PWM_STEPS];
      if (step_counter != 1)
      {


        uint8_t  i = 0;
        for (i; i < step_duty; i++)
        {
            seq_values[i] = 0x8000;
        }
        for (i; i < step_counter; i++)
        {
            seq_values[i] = 0;
        }
      }
      else
      {
              seq_values[0] = 0x8000;
              step_counter =1;
      }

      nrf_pwm_sequence_t const seq =
      {
          .values.p_common = seq_values,
          .length          = step_counter,//NRF_PWM_VALUES_LENGTH(seq_values),
          .repeats         = RI_GPIO_PWM_SEQ_REPEATS,
          .end_delay       = RI_GPIO_PWM_SEQ_DELAY
      };
      if (RI_GPIO_PWM_PLAYBACK_OK == nrf_drv_pwm_simple_playback(&m_pwm, &seq, 
                                                        RI_GPIO_PWM_PLAYBACK_COUNT,
                                                        NRF_DRV_PWM_FLAG_LOOP))
      {
          res = RD_SUCCESS;
      }
    }

    return res;
}

rd_status_t ri_gpio_pwm_stop (const ri_gpio_id_t pin)
{
    return RD_ERROR_NOT_IMPLEMENTED;
}

#endif
