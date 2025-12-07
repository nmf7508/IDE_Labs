/**
 * @file    servo.h
 * @brief   Servo Motor Driver for MSPM0 Car Project
 * @details Handles PWM generation for the steering servo using Timer A1.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>

/**
 * @brief   Initializes TIMA1 for the steering servo.
 * @details Configures a 50Hz (20ms) PWM signal.
 * Center position is set to 7.5% duty cycle (1.5ms).
 */
void SteeringServo_Init(void);

/**
 * @brief   Sets the steering servo to a specific angle based on a ratio.
 *
 * @param   correction A value from -1.0 (Full Left) to +1.0 (Full Right).
 * 0.0 is Center.
 * @note    The function clamps the input to ensure the servo does not 
 * exceed its physical limits (5% to 10% duty cycle).
 */
void SteeringServo_Set_Turn(double correction);

#endif /* SERVO_H_ */
