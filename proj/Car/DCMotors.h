#ifndef DCMOTORS_H_
#define DCMOTORS_H_

#include <stdint.h>

/**
 * @brief Initializes the 4 TIMA0 PWM channels for the motors.
 */
void Motor_Init(void);

/**
 * @brief Stops both motors [Required for Demo 1 Carpet Stop].
 */
void Motor_Stop(void);

/**
 * @brief Sets speed for both motors (-50 to +50).
 * @note Enforces the 50% max duty cycle rule.
 */
void Motor_Set_Speed(int16_t leftSpeed, int16_t rightSpeed);

#endif /* DCMOTORS_H_ */
