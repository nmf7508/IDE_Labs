/**
 * @file    servo.c
 * @brief   Servo Motor Driver Implementation
 * @details Maps PID control values (-1.0 to 1.0) to PWM Duty Cycles.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
 */

#include "servo.h"
#include "timers.h" 

/* --- Configuration --- */
// Timer Settings for 50Hz (20ms period)
#define SERVO_PERIOD        313
#define SERVO_PRESCALER     255

// Duty Cycle Limits
#define DUTY_MAX_LIMIT      0.10   // 10% Duty Cycle
#define DUTY_CENTER         0.075  // 7.5% Duty Cycle
#define DUTY_MIN_LIMIT      0.05   // 5% Duty Cycle

/**
 * @brief   Clamps a value to a min/max range.
 * @note    Static helper, not exposed in header.
 */
static double clamp(double val, double min, double max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void SteeringServo_Init(void) {
    // Initialize TIMA1 Channel 0 with Center Duty Cycle
    TIMA1_PWM_init(0, SERVO_PERIOD, SERVO_PRESCALER, DUTY_CENTER);
}

void SteeringServo_Set_Turn(double correction) {
    // Calculate target duty cycle based on correction ratio (-1.0 to 1.0)
    // Formula scales the difference between Center and Max Limit
    double duty_cycle = DUTY_CENTER + (correction * (DUTY_MAX_LIMIT - DUTY_CENTER));
    
    // Clamp to ensure we never send an out-of-bounds signal to the servo
    duty_cycle = clamp(duty_cycle, DUTY_MIN_LIMIT, DUTY_MAX_LIMIT);

    // Update PWM
    TIMA1_PWM_DutyCycle(0, duty_cycle);
}
