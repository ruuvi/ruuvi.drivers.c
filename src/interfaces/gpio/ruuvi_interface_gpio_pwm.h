#ifndef RUUVI_INTERFACE_GPIO_PWM_H
#define RUUVI_INTERFACE_GPIO_PWM_H
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_enabled_modules.h"
#include "ruuvi_interface_gpio.h"
/** @brief Enable implementation selected by application */
#if RI_GPIO_ENABLED
#  define RUUVI_NRF5_SDK15_GPIO_PWM_ENABLED RUUVI_NRF5_SDK15_ENABLED
#endif

#include <stdbool.h>

/**
 * @addtogroup GPIO
 * @brief Pulse width modulation of GPIO.
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

/**
 * @brief Run any necessary initialization for PWM.
 *
 * After calling this function PWM peripheral may consume power even
 * if no PWM functionality is used.
 *
 * @retval RD_SUCCESS Initialization was successful.
 * @retval RD_ERROR_INVALID_STATE If PWM was already initialized.
 * @retval TODO Error code on other error.
 */
rd_status_t ri_gpio_pwm_init (void);

/**
 * @brief Uninitialize PWM.
 *
 * After calling this function PWM peripheral may no longer consume
 * power. GPIO pins may be configured as Hi-Z or they may leave in previous state.
 *
 * @retval RD_SUCCESS Initialization was successful.
 * @retval RD_ERROR_INVALID_STATE If PWM was already initialized.
 */
rd_status_t ri_gpio_pwm_uninit (void);

/**
 * @brief Start PWM on given pin at given frequency and duty cycle.
 *
 * Polarity of PWM is active-high and phase is undefined, implementation is allowed
 * to use centered or edge-triggered PWM.
 *
 * @param[in] pin Pin to start PWM on.
 * @param[in] mode Output mode of GPIO pin, input modes or Hi-Z not supported.
 * @param[in,out] frequency Input: Target frequency, "at least this much".
 *                          Output: Configured frequency, equal or greater than input.
 * @param[in,out] duty_cycle  Input: Target duty cycle of PWM, 0.0 ... 1.0.
 *                            Output: Configured duty cycle, equal or creater than input.
 *
 * @retval RD_SUCCESS PWM was started.
 * @retval RD_ERROR_NULL one or both of pointers was NULL.
 * @retval RD_ERROR_INVALID_STATE PWM was not initialized.
 * @retval RD_ERROR_INVALID_PARAM Pin, mode, frequency or duty cycle were somehow invalid.
 */
rd_status_t ri_gpio_pwm_start (const ri_gpio_id_t pin, const ri_gpio_mode_t mode,
                               float * const frequency, float * const duty_cycle);

/**
 * @brief Stop PWM on given pin.
 *
 * After calling this function PWM is stopped and given pin is
 * configured as Hi-Z regardless of it's previous state, this can
 * be called on non-PWM pin too.
 *
 * @param[in] pin Pin to stop PWM on.
 * @retval RD_SUCCESS If pin was configured as Hi-Z.
 * @retval RD_ERROR_INVALID_STATE currently not stopped.
 */
rd_status_t ri_gpio_pwm_stop (const ri_gpio_id_t pin);

/**
 * @brief Check if PWM is initialized.
 *
 * @retval true PWM is initialized.
 * @retval false PWM is not initialized.
 */
bool ri_gpio_pwm_is_init (void);

/** @} */
#endif // RUUVI_INTERFACE_GPIO_PWM_H
