#ifndef RUUVI_GPIO_H
#define RUUVI_GPIO_H

#include "ruuvi_error.h"
#include <stdbool.h>

typedef enum 
{
  HIGH_Z,
  INPUT_NOPULL,
  INPUT_PULLUP,
  INPUT_PULLDOWN,
  OUTPUT_STANDARD,
  OUTPUT_HIGHDRIVE  
}ruuvi_gpio_mode_t;

ruuvi_status_t platform_gpio_init(void);
ruuvi_status_t platform_gpio_configure(uint8_t pin, ruuvi_gpio_mode_t mode);
ruuvi_status_t platform_gpio_set(uint8_t pin);
ruuvi_status_t platform_gpio_clear(uint8_t pin);
ruuvi_status_t platform_gpio_toggle(uint8_t pin);
ruuvi_status_t platform_gpio_read(uint8_t pin, bool* high);

#endif