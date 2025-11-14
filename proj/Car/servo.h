#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>

/**
 * @brief Initializes TIMA1 for the steering servo.
 * @note Assumes a 50Hz (20ms) period.
 */
void SteeringServo_Init(void);

/**
 * @brief Sets the steering servo to a specific angle.
 *
 * @param correction A value from -1.0 (full left) to +1.0 (full right).
 * 0.0 is center.
 */
void SteeringServo_Set_Turn(double correction);

#endif /* SERVO_H_ */
