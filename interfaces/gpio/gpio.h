#ifndef RUUVI_GPIO_H
#define RUUVI_GPIO_H

#include "ruuvi_error.h"
#include <stdbool.h>

typedef enum 
{
  RUUVI_GPIO_MODE_HIGH_Z,
  RUUVI_GPIO_MODE_INPUT_NOPULL,
  RUUVI_GPIO_MODE_INPUT_PULLUP,
  RUUVI_GPIO_MODE_INPUT_PULLDOWN,
  RUUVI_GPIO_MODE_OUTPUT_STANDARD,
  RUUVI_GPIO_MODE_OUTPUT_HIGHDRIVE  
}ruuvi_gpio_mode_t;

ruuvi_status_t platform_gpio_init(void);
ruuvi_status_t platform_gpio_configure(uint8_t pin, ruuvi_gpio_mode_t mode);
ruuvi_status_t platform_gpio_set(uint8_t pin);
ruuvi_status_t platform_gpio_clear(uint8_t pin);
ruuvi_status_t platform_gpio_toggle(uint8_t pin);
ruuvi_status_t platform_gpio_read(uint8_t pin, bool* high);

#endif