/**
 * @file    camera.h
 * @brief   Camera Sensor and PID Control Driver
 * @details Parallax TSL1401-DB Linescan Camera initialization and interaction.
 * Reserves Timer G0 (CLK), Timer G6 (SI), and ADC0 Channel 0.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <stdint.h>
#include <stdbool.h>

/* --- Error Codes --- */
#define LINE_NOT_FOUND      -10000  // Error: Average intensity too low (Carpet)
#define LINE_LOST_HISTORY   -60000  // Error: Average intensity too high (Disconnect)

/* --- Global Data (Shared with ISRs) --- */
extern volatile uint8_t cameraData_complete;
extern volatile int pixelCounter;       // counts CLK edges
extern uint16_t cameraData[128];        // store 128 pixels

/* --- Function Prototypes --- */

/**
 * @brief   Initialize camera associated components (Timers, GPIO, ADC).
 */
void Camera_init(void);

/**
 * @brief   Checks whether camera data is ready to retrieve.
 * @return  1 (True) if data is ready, 0 (False) otherwise.
 */
uint8_t Camera_isDataReady(void);

/**
 * @brief   Retrieves pointer to the raw camera data array.
 * @return  Pointer to the global uint16_t data array.
 */
uint16_t* Camera_getData(void);

/**
 * @brief   Calculates the center of mass (weighted average) of the line.
 * @param   sensorValues Pointer to the camera data array.
 * @return  Normalized error value (-1.0 to 1.0) or an Error Code.
 */
double LineSensor_Calculate_Error(uint16_t *sensorValues);

/**
 * @brief   Computes the PID control output based on the line error.
 * @param   error_norm The normalized error from the line sensor.
 * @return  Control variable (steering adjustment).
 */
double PID_Update(double error_norm);

#endif /* CAMERA_H_ */
