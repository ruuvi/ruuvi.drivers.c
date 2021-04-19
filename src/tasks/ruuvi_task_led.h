#ifndef RUUVI_TASK_LED_H
#define RUUVI_TASK_LED_H

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
 * @file ruuvi_task_led.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-01-27
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
 *  ri_yield_indication_set (rt_led_activity_indicate);
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 *  _do_stuff_();
 *  err_code = rt_led_uninit();
 *  RD_ERROR_CHECK(err_code, RD_SUCCESS;
 * @endcode
 */

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
rd_status_t rt_led_init (const ri_gpio_id_t * const leds,
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
rd_status_t rt_led_write (const ri_gpio_id_t led, const bool state);

/**
 * @brief Function to indicate activity in program.
 * Led is turned on while program is active
 * and off while in sleep.
 * Call ri_yield_indication_set to setup this function to be called
 * when entering / leaving sleep for example.
 *
 * @param[in] state True to indicate activity, false to indicate sleep.
 */
void rt_led_activity_indicate (const bool state);

/**
 * @brief Set LED which is used to indicate activity.
 *
 * This function can be called before GPIO or LEDs are initialized.
 * Call with RI_GPIO_ID_UNUSED to disable activity indication.
 *
 * @param[in] led LED to indicate activity with.
 *
 * @retval RD_SUCCESS if valid led was set.
 * @retval RD_ERROR_INVALID_PARAM if there is no pin in LED.
 */
rd_status_t rt_led_activity_led_set (const ri_gpio_id_t led);

/**
 * @brief Get LED which is used to indicate activity.
 *
 * @return Led which is activity indicator, RI_GPIO_ID_UNUSED if none.
 */
uint16_t rt_led_activity_led_get (void);

/**
 * @brief Start blinking led at 50 % duty cycle at given interval.
 *
 * This function requires ri_timer to be initialized.
 * Only one led can blink at once, you must call @ref rt_led_blink_stop
 * before starting to blink another led.
 *
 * @param[in] led LED to blink.
 * @param[in] interval_ms Interval of blinking in milliseconds, min and max values come
 *                        from timer interface.
 *
 * @retval RD_SUCCESS Blinking was started.
 * @retval RD_ERROR_INVALID_STATE If led is already blinking.
 * @retval RD_ERROR_RESOURCES If timer cannot be allocated.
 * @retval RD_ERROR_INVALID_PARAM If there is no pin in LED.
 */
rd_status_t rt_led_blink_start (const ri_gpio_id_t led, const uint16_t interval_ms);

/**
 * @brief Function to blink led once.
 *
 * The function call is ignored if previous timer is already running.
 *
 * This function requires ri_timer to be initialized.
 * Only one led can blink at once. The function turns the led on in the beginning.
 * The interrupt calls @ref rt_led_blink_stop to turn the led off after set time interval.
 *
 * @param[in] led LED to blink.
 * @param[in] interval_ms Interval of blinking in milliseconds, min and max values come
 *                        from timer interface.
 *
 * @retval RD_SUCCESS Blinking was started.
 * @retval RD_ERROR_INVALID_STATE If led is already blinking.
 * @retval RD_ERROR_RESOURCES If timer cannot be allocated.
 * @retval RD_ERROR_INVALID_PARAM If there is no pin in LED.
 */

rd_status_t rt_led_blink_once (const ri_gpio_id_t led, const uint16_t interval_ms);

/**
 * @brief Stop blinking led and leave the pin as high-drive output in inactive state.
 *
 *
 * @param[in] led LED to stop.
 *
 * @retval RD_SUCCESS Blinking was stopped.
 * @retval RD_ERROR_INVALID_STATE If given LED is not blinking.
 */
rd_status_t rt_led_blink_stop (const ri_gpio_id_t led);

/**
 * @brief Check if LED task has been initialized.
 *
 * @retval true LED task is initialized.
 * @retval false LED task is not initialized.
 */
bool rt_led_is_init (void);

#ifdef CEEDLING
int8_t is_led (const ri_gpio_id_t led);
void rt_led_blink_isr (void * const p_context);
void rt_led_blink_once_isr (void * const p_context);
#endif

/*@}*/
#endif
