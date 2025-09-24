/**
 * ******************************************************************************
 * @file    : camera.h
 * @brief   : Camera module header file
 * @details : Parallax TSL1401-DB Linescan Camera initialization and interaction
 * @note    : Reserves the use of Timers G0 (CLK) and G6 (SI)
 *            and ADC0 channel 0 
 *
 * @author 
 * @date 
 * ******************************************************************************
*/

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <stdint.h>

/**
 * @brief Initialize camera associated components
*/
void Camera_init(void);


/**
 * @brief Checks whether camer data is ready to retrieve
 * @note Make sure to check all data is available
 * @return True(1)/False(0) if camera data is ready
*/
uint8_t Camera_isDataReady(void);


/**
 * @brief Retrieves pointer to camera data array
 * @return Pointer to global data array stored locally in this file
*/
uint16_t* Camera_getData(void);


#endif // _CAMERA_H_
