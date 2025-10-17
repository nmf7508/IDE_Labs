/**
 * @file servo.h
 * @brief Header file for Servo Motor control using TIMA1 PWM.
 *
 * @authors
 * Nick Fair
 * Nathan Winiarski
 *
 * @date 10/17/2025
 */
#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>

/**
 * @brief Initializes TIMA1 Channel 0 for 50 Hz PWM servo control.
 * Sets the servo to the center position (1.5 ms pulse) upon initialization.
 */
void Servo_Init(void);

/**
 * @brief Sets the servo position based on an angle.
 * @param angle The desired angle, from -90 (full counter-clockwise) to 90 (full clockwise).
 * A value of 0 will center the servo.
 */
void Servo_Set_Position(int8_t angle);

/**
 * @brief Sets the servo position based on a specific pulse width.
 * This is useful for fine-tuning and debugging.
 * @param pulse_ms The desired pulse width in milliseconds (typically 1.0 to 2.0).
 */
void Servo_Set_Pulse_ms(double pulse_ms);

#endif /* SERVO_H_ */
