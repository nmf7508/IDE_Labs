/**
 * @file motor.h
 * @brief Header file for DC and Stepper motor control functions.
 *
 * @authors
 * Nick Fair
 * Nathan Winiarski
 *
 * @date 10/16/2025
 */
#ifndef MOTOR_H_
#define MOTOR_H_

#include <ti/devices/msp/msp.h>
#include <stdint.h>

// Part 2: Stepper Motor Control

/**
 * @brief Initializes the 4 GPIO pins used to drive the stepper motor's Darlington array.
 */
void Stepper_Motor_Init(void);

/**
 * @brief Moves the stepper motor one step in the specified direction.
 * @param forward Set to 1 for forward (clockwise), 0 for reverse (counter-clockwise).
 */
void Stepper_Motor_Step(int forward);

#endif /* MOTOR_H_ */
