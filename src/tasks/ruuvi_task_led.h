#ifndef  TASK_LED_H
#define  TASK_LED_H

/**
  * @defgroup actuator_tasks  Interacting with outside world
  */
/*@{*/
/**
 * @defgroup led_tasks LED tasks
 * @brief LED functions
 *
 */
/*@}*/
/**
 * @addtogroup led_tasks
 */
/*@{*/
/**
 * @file task_led.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-11-18
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * LED control.
 *
 * Typical usage:
 *
 * @code{.c}
 *  rd_status_t err_code = RD_SUCCESS;
 *  err_code = rt_gpio_init();
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  err_code = rt_led_init();
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  err_code = rt_led_write (RUUVI_BOARD_LED_GREEN, RUUVI_BOARD_LEDS_ACTIVE_STATE);
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  err_code = rt_led_activity_led_set (RUUVI_BOARD_LED_GREEN);
 *  ruuvi_interface_yield_indication_set (rt_led_activity_indicate);
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  _do_stuff_();
 *  err_code = rt_led_uninit();
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 * @endcode
 */

#include "ruuvi_boards.h"
#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include <stdbool.h>
#include <stdlib.h>

/** @brief turn led on */
#define RT_LED_ON  (true)
/** @brief turn led off */
#define RT_LED_OFF (false)

/**
 * @brief LED initialization function.
 * - Returns error if leds were already initialized.
 * - Initializes GPIO if GPIO wasn't initialized.
 * - returns error code if GPIO cannot be initialized
 * - Configures GPIOs as high-drive output and sets LEDs as inactive.
 * - Sets activity led to uninitialized
 *
 * @param[in] leds Array of leds used in application.
 * @param[in] active_states Array of gpio states where each led is active
 * @param[in] num_leds number of leds in application.
 *
 * @retval RD_SUCCESS if no errors occured.
 * @retval RD_ERROR_INVALID_STATE if leds were already initialized.
 * @retval error code from stack on other error.
 * @warning Behaviour is undefined if led list and state list are o different size
 *          than num_leds.
 **/
rd_status_t rt_led_init (const uint16_t * const leds,
                         const ri_gpio_state_t * const active_states,
                         const size_t num_leds);

/**
 * @brief LED uninitialization function.
 * - Returns error if leds were already initialized.
 * - Configures GPIOs as high-z.
 * - Sets activity led to uninitialized
 *
 * @retval RD_SUCCESS if no errors occured.
 * @retval error code from stack on other error.
 **/
rd_status_t rt_led_uninit (void);

/**
 * @brief LED write function. Set given LED ON or OFF.
 *
 * @param[in] led  LED to change, use constant from RUUVI_BOARDS
 * @param[in] state  true to turn led on, false to turn led off.
 *
 * @retval RD_SUCCESS if value was written
 * @retval RD_ERROR_INVALID_PARAM  if GPIO pin is not led.
 * @retval RD_ERROR_INVALID_STATE if GPIO task is not initialized.
 **/
rd_status_t rt_led_write (const uint16_t led, const bool state);

/**
 * @brief Function to indicate activity in program.
 * Led is turned on while program is active
 * and off while in sleep.
 * Call ruuvi_interface_yield_indication_set to setup this function to be called
 * when entering / leaving sleep for example.
 *
 * @param[in] state True to indicate activity, false to indicate sleep.
 */
void rt_led_activity_indicate (const bool state);

/**
 * @brief Set LED which is used to indicate activity.
 *
 * This function can be called before GPIO or LEDs are initialized.
 * Call with RUUVI_INTERFACE_GPIO_ID_UNUSED to disable activity indication.
 *
 * @param[in] led LED to indicate activity with.
 *
 * @retval RD_SUCCESS if valid led was set.
 * @retval RD_ERROR_INVALID_PARAM if there is no pin in LED.
 */
rd_status_t rt_led_activity_led_set (const uint16_t led);

/**
 * @brief Get LED which is used to indicate activity.
 *
 * @return Led which is activity indicator, RUUVI_INTERFACE_GPIO_ID_UNUSED if none.
 */
uint16_t rt_led_activity_led_get (void);

#ifdef CEEDLING
bool is_led (const uint16_t led);
#endif

/*@}*/
#endif
