#ifndef RUUVI_INTERFACE_TMP117_H
#define RUUVI_INTERFACE_TMP117_H
/**
 * @addtogroup Environmental
 */
/*@{*/
/**
 * @defgroup TMP117 TMP117 Interface
 * @brief Implement @ref ruuvi_driver_sensor_t functions on TMP117
 *
 * The implementation supports taking single-samples and a continuous mode
 */
/*@}*/
/**
 * @addtogroup TMP117
 */
/*@{*/
/**
 * @file ruuvi_interface_tmp117.h
 * @author Otso Jousimaa <otso@ojousima.net>
 * @date 2019-11-13
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 *
 * TMP117 temperature sensor driver.
 *
 */

#include "ruuvi_driver_error.h"
#include "ruuvi_driver_sensor.h"

/*
ADDRESS TYPE RESET ACRONYM       REGISTER NAME
00h     R    8000h Temp_Result   Temperature result register
01h     R/W  0220h Configuration Configuration register
02h     R/W  6000h THigh_Limit   Temperature high limit register
03h     R/W  8000h TLow_Limit    Temperature low limit register
04h     R/W  0000h EEPROM_UL     EEPROM unlock register
05h     R/W  xxxxh EEPROM1       EEPROM1 register
06h     R/W  xxxxh EEPROM2       EEPROM2 register
07h     R/W  0000h Temp_Offset   Temperature offset register
08h     R/W  xxxxh EEPROM3       EEPROM3 register
0Fh     R    0117h Device_ID     Device ID register
*/

#define TMP117_REG_TEMP_RESULT   0x00
#define TMP117_REG_CONFIGURATION 0x01
#define TMP117_REG_THIGH_LIMIT   0x02
#define TMP117_REG_TLOW_LIMIT    0x03
#define TMP117_REG_EEPROM_UL     0x04
#define TMP117_REG_EEPROM1       0x05
#define TMP117_REG_EEPROM2       0x06
#define TMP117_REG_TEMP_OFFSET   0x07
#define TMP117_REG_EEPROM3       0x08
#define TMP117_REG_DEVICE_ID     0x0F

#define TMP117_MASK_RESET        0x0002
#define TMP117_MASK_ID           0x01FF
#define TMP117_MASK_OS           0x0030
#define TMP117_MASK_MODE         0x0C00
#define TMP117_MASK_CC           0x0380

#define TMP117_VALUE_ID          0x0117

#define TMP117_POS_OS            5
#define TMP117_VALUE_OS_1        (0x00 << TMP117_POS_OS)
#define TMP117_VALUE_OS_8        (0x01 << TMP117_POS_OS)
#define TMP117_VALUE_OS_32       (0x02 << TMP117_POS_OS)
#define TMP117_VALUE_OS_64       (0x03 << TMP117_POS_OS)

#define TMP117_POS_CC            7
#define TMP117_VALUE_CC_16_MS    (0x00 << TMP117_POS_CC)
#define TMP117_VALUE_CC_125_MS   (0x01 << TMP117_POS_CC)
#define TMP117_VALUE_CC_250_MS   (0x02 << TMP117_POS_CC)
#define TMP117_VALUE_CC_500_MS   (0x03 << TMP117_POS_CC)
#define TMP117_VALUE_CC_1000_MS  (0x04 << TMP117_POS_CC)
#define TMP117_VALUE_CC_4000_MS  (0x05 << TMP117_POS_CC)
#define TMP117_VALUE_CC_8000_MS  (0x06 << TMP117_POS_CC)
#define TMP117_VALUE_CC_16000_MS (0x07 << TMP117_POS_CC)

#define TMP117_POS_MODE          10
#define TMP117_VALUE_MODE_SLEEP  (0x01 << TMP117_POS_MODE)
#define TMP117_VALUE_MODE_SINGLE (0x03 << TMP117_POS_MODE)
#define TMP117_VALUE_MODE_CONT   (0x00 << TMP117_POS_MODE)

#define TMP117_VALUE_TEMP_NA     0x8000

/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_tmp117_init (rd_sensor_t *
                            environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_init_fp */
rd_status_t ri_tmp117_uninit (rd_sensor_t *
                              environmental_sensor, rd_bus_t bus, uint8_t handle);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_tmp117_samplerate_set (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_tmp117_samplerate_get (uint8_t * samplerate);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_tmp117_resolution_set (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_tmp117_resolution_get (uint8_t * resolution);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_tmp117_scale_set (uint8_t * scale);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_tmp117_scale_get (uint8_t * scale);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_tmp117_dsp_set (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_dsp_fp */
rd_status_t ri_tmp117_dsp_get (uint8_t * dsp, uint8_t * parameter);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_tmp117_mode_set (uint8_t *);
/** @brief @ref rd_sensor_setup_fp */
rd_status_t ri_tmp117_mode_get (uint8_t *);
/** @brief @ref rd_sensor_data_fp */
rd_status_t ri_tmp117_data_get (rd_sensor_data_t * const
                                data);
/*@}*/
#endif