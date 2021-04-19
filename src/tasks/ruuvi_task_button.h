/**
 * @addtogroup button_tasks Button tasks
 * @brief User input via buttons
 *
 */
/*@{*/
/**
 * @file ruuvi_task_button.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2020-01-25
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 *
 */
#ifndef  RUUVI_TASK_BUTTON_H
#define  RUUVI_TASK_BUTTON_H

#include "ruuvi_driver_error.h"
#include "ruuvi_interface_gpio.h"
#include "ruuvi_interface_gpio_interrupt.h"
#include <stdlib.h>

/** @brief Called on button event with the slope of edge.
 *
 * @param[in] event Slope of the button event, RI_GPIO_SLOPE_LOTOHI or
 *                  RI_GPIO_SLOPE_HITOLO.
 */
typedef void (*rt_button_fp_t) (const ri_gpio_evt_t event);

/** @brief Struct for initializing buttons. */
typedef struct
{
    const ri_gpio_id_t * p_button_pins;       //!< Array of button pins.
    const ri_gpio_state_t * p_button_active;  //!< Array of button active states.
    const rt_button_fp_t * p_button_handlers; //!< Array of button handlers.
    const size_t num_buttons;                 //!< Number of buttons to initialize.
} rt_button_init_t;

/**
 * @brief Button initialization function.
 *
 * Requires GPIO and interrupts to be initialized.
 * Configures GPIO as pullup/-down according to button active state.
 *
 * @param[in] rt_init Initialization structure for button task.
 *
 * @retval RD_SUCCESS If buttons were initialized.
 * @retval RD_ERROR_NULL If any array of rt_init is NULL or any element of
                         p_button_pins or p_button_active is NULL.
 * @retval RD_ERROR_INVALID_STATE if GPIO or GPIO interrupts aren't initialized.
 * @retval RD_ERROR_INVALID_PARAM if the GPIOs arent useable, e.g. pin 48 on board with
 *                                32 GPIO pins or invalid active state.
 * @retval RD_ERROR_NOT_ENABLED   if the driver module is not enabled at compile time.
 *
 * @note    This function can be called without uninitialization to reconfigure.
 * @warning Behaviour is undefined if lengths of arrays don't match num_buttons.
 * @warning If error occurs while intializing pins the task calls rt_button_unit
 *          with the original initialization structure.
 **/
rd_status_t rt_button_init (const rt_button_init_t * const rt_init);

/**
 * @brief Button uninitialization function.
 *
 * After calling this function the given button pins are configured as High-Z and
 * their interrupts are disabled.
 *
 * @retval RD_SUCCESS
 * @retval RD_ERROR_NULL If any array of rt_init is NULL or any element of
                         p_button_pins NULL.
 **/
rd_status_t rt_button_uninit (const rt_button_init_t * const rt_init);

/*@}*/
#endif
