#ifndef RUUVI_PIN_INTERRUPT_H
#define RUUVI_PIN_INTERRUPT_H

#include "nrf_drv_gpiote.h"

#include "ruuvi_error.h"
#include "gpio.h"

typedef enum 
{
  RUUVI_GPIO_SLOPE_HITOLO,
  RUUVI_GPIO_SLOPE_LOTOHI,
  RUUVI_GPIO_SLOPE_TOGGLE,
  RUUVI_GPIO_SLOPE_UNKNOWN
}ruuvi_gpio_slope_t;

typedef struct 
{
  ruuvi_gpio_slope_t slope;
  uint8_t pin;
}ruuvi_gpio_evt_t;

typedef void(*pin_interrupt_fp)(const ruuvi_gpio_evt_t);

ruuvi_status_t platform_pin_interrupt_init();
ruuvi_status_t platform_pin_interrupt_enable(uint8_t pin, ruuvi_gpio_slope_t slope, ruuvi_gpio_mode_t mode, pin_interrupt_fp handler);

#endif