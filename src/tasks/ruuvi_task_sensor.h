#ifndef RUUVI_TASK_SENSOR_H
#define RUUVI_TASK_SENSOR_H
/**
 * @defgroup sensor_tasks Sensor tasks
 */
/*@{*/
/**
 * @file ruuvi_task_sensor.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-20-21
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * Helper functions common to all sensors
 *
 */
#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"
#include "ruuvi_interface_gpio.h"

typedef struct
{
    rd_sensor_t sensor;                       //!< Control structure for sensor.
    rd_sensor_init_fp init;                   //!< Initialization function.
    rd_sensor_configuration_t configuration;  //!< Sensor configuration.
    uint16_t nvm_file;                        //!< NVM file of configuration.
    uint16_t nvm_record;                      //!< NVM record of configuration.
    uint8_t  handle;                          //!< Handle of sensor.
    rd_bus_t bus;                             //!< Bus of sensor.
    ri_gpio_id_t pwr_pin;                     //!< Power control pin.
    ri_gpio_state_t pwr_on;                   //!< Power-on state of ctrl pin.
    ri_gpio_id_t fifo_pin;                    //!< FIFO full interrupt.
    ri_gpio_id_t level_pin;                   //!< Level interrupt.
} rt_sensor_ctx_t;

/** @brief Initialize sensor CTX
 *
 * To initialize a sensor, initialization function, sensor bus and sensor handle must
 * be set. After initialization, sensor control structure is ready to use,
 * initial configuration is set to actual values on sensor.
 *
 * To configure the sensor, set the sensor configuration in struct and call
 * @ref rt_sensor_configure.
 *
 * @param[in] sensor Sensor to initialize.
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
rd_status_t rt_sensor_initialize (rt_sensor_ctx_t * const sensor);

/** @brief Store the sensor state to NVM.
 *
 * @param[in] sensor Sensor to store.
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
rd_status_t rt_sensor_store (rt_sensor_ctx_t * const sensor);

/** @brief Load the sensor state from NVM.
 *
 * @param[in] sensor Sensor to store.
 *
 * @return RD_SUCCESS on success.
 * @return RD_ERROR_NULL if sensor is NULL.
 * @return error code from sensor on other error.
 */
rd_status_t rt_sensor_load (rt_sensor_ctx_t * const sensor);

/**
 * @brief Configure a sensor with given settings.
 *
 * @param[in,out] sensor In: Sensor to configure.
 *                       Out: Sensor->configuration will be set to actual configuration.
 *
 * @retval RD_SUCCESS on success.
 * @retval RD_ERROR_NULL if sensor is NULL.
 * @retval error code from sensor on other error.
 */
rd_status_t rt_sensor_configure (rt_sensor_ctx_t * const sensor);

/**
 * @brief Search for requested sensor backend in given list of sensors.
 *
 * @param[in] sensor_list Array of sensors to search the backend from.
 * @param[in] count Number of sensor backends in the list.
 * @param[in] name NULL-terminated, max 9-byte (including trailing NULL) string
 *                 representation of sensor.
 * @return pointer to requested sensor CTX if found
 * @return NULL if requested sensor was not found
 */
rt_sensor_ctx_t * rt_sensor_find_backend (rt_sensor_ctx_t * const
        sensor_list, const size_t count, const char * const name);

/**
 * @brief Search for a sensor which can provide requested values
 *
 * @param[in] sensor_list Array of sensors to search the backend from.
 * @param[in] count Number of sensor backends in the list.
 * @param[in] values Fields which sensor must provide.
 * @return Pointer to requested sensor CTXif found. If there are many candidates, first is
 *         returned
 * @return NULL if requested sensor was not found.
 */
rt_sensor_ctx_t * rt_sensor_find_provider (rt_sensor_ctx_t * const
        sensor_list, const size_t count, rd_sensor_data_fields_t values);

/*@}*/
#endif
