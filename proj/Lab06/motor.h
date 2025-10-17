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

// ==============================================================================
// ?? Part 1: DC Motor (H-Bridge) Control
// ==============================================================================

void delay_ms(uint32_t ms);
/**
 * @brief Initializes GPIOs and Timers for PWM control of both DC motors.
 * This sets up the H-bridge enable pins and initializes TIMA0 for PWM.
 */
void DC_Motor_Init(void);

/**
 * @brief Sets the speed and direction of a single DC motor.
 * @param channel 0 for the left motor, 1 for the right motor.
 * @param speed Speed percentage from -100 (full reverse) to 100 (full forward).
 * A value of 0 stops the motor.
 */
void DC_Motor_Set_Speed(uint8_t channel, int8_t speed);

/**
 * @brief Stops both DC motors.
 */
void DC_Motor_Stop(void);


// ==============================================================================
// ?? Part 2: Stepper Motor Control
// ==============================================================================

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
