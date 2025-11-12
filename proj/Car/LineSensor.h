#ifndef LINESENSOR_H_
#define LINESENSOR_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Processes 128 pixels to find the line error.
 * @return int32_t: 0=centered, negative=line is left, positive=line is right.
 */
int32_t LineSensor_Calculate_Error(uint16_t* sensorValues);

/**
 * @brief Checks if the camera sees "mostly-dark" carpet.
 * @return true if carpet is detected, false otherwise.
 */
bool LineSensor_Detect_Carpet(uint16_t* sensorValues);

#endif /* LINESENSOR_H_ */
