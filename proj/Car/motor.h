/**
 * @file    motor.h
 * @brief   Motor Driver for MSPM0 Car Project
 * @details Handles PWM generation for DC motors and GPIO configuration 
 * for the Motor Driver enable pins.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>

/**
 * @brief   Initializes the 4 TIMA0 PWM channels and GPIO Enable pins.
 * @details Configures:
 * - Left Rear:  CH0 (AIN1), CH1 (AIN2)
 * - Right Rear: CH2 (BIN1), CH3 (BIN2)
 * - GPIO Enables: PB19 and PA22
 */
void Motor_Init(void);

/**
 * @brief   Stops both rear motors immediately (0% Duty Cycle).
 */
void Motor_Stop(void);

/**
 * @brief   Sets the speed and direction for both rear motors.
 * * @param   left_speed  Speed percentage for left motor (-50 to +50).
 * @param   right_speed Speed percentage for right motor (-50 to +50).
 * @note    Positive = Forward, Negative = Reverse.
 * Values are clamped to +/- MAX_SPEED (50).
 */
void Motor_Set_Speed(int16_t left_speed, int16_t right_speed);

#endif /* MOTOR_H_ */
