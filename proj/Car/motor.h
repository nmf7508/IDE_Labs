#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>

/**
 * @brief Initializes the 4 TIMA0 PWM channels for the rear drive motors.
 * Assumes:
 * Left Rear:  CH0 (AIN1), CH1 (AIN2)
 * Right Rear: CH2 (BIN1), CH3 (BIN2)
 */
void Motor_Init(void);

/**
 * @brief Stops both rear motors.
 */
void Motor_Stop(void);

/**
 * @brief Sets the speed and direction for both rear motors.
 *
 * @param speed Speed for the motors (-50 to +50).
 * Positive is forward, negative is reverse.
 * @note This function enforces the 50% maximum duty cycle.
 */
void Motor_Set_Speed(int16_t left_speed, int16_t right_speed);

#endif /* MOTOR_H_ */
