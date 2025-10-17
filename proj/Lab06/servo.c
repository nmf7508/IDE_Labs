/**
 * @file servo.c
 * @brief Implementation of servo motor control functions.
 *
 * @authors
 * Nick Fair
 * Nathan Winiarski
 *
 * @date 10/17/2025
 */
#include "servo.h"
#include "lab6/timers.h" // Your existing timers header

// --- Servo Timing Constants ---
// The timer's input clock is 32MHz / 8 (from CLKDIV) = 4 MHz.
// To get a 50 Hz period (20 ms), we need: 0.020s * 4,000,000 Hz = 80,000 ticks.
#define SERVO_PERIOD        80000
#define SERVO_PRESCALER     0

#define MIN_PULSE_MS        1.0   // Corresponds to -90 degrees
#define MAX_PULSE_MS        2.0   // Corresponds to +90 degrees
#define CENTER_PULSE_MS     1.5   // Corresponds to 0 degrees

void Servo_Init(void) {
    // Calculate the initial duty cycle for the center position (1.5 ms)
    // Your PWM function uses (1 - %ON_TIME) for the duty cycle argument.
    // %ON_TIME = 1.5ms / 20ms = 0.075
    // Initial Duty Argument = 1.0 - 0.075 = 0.925
    double initial_duty = 1.0 - (CENTER_PULSE_MS / 20.0);

    // Initialize TIMA1 Channel 0 for the servo
    TIMA1_PWM_init(0, SERVO_PERIOD, SERVO_PRESCALER, initial_duty);
}

void Servo_Set_Pulse_ms(double pulse_ms) {
    // Clamp the pulse width to the valid range [1.0, 2.0]
    if (pulse_ms < MIN_PULSE_MS) {
        pulse_ms = MIN_PULSE_MS;
    }
    if (pulse_ms > MAX_PULSE_MS) {
        pulse_ms = MAX_PULSE_MS;
    }

    // Calculate the duty cycle argument needed by your TIMA1_PWM_DutyCycle function
    // The total period is 20ms (50 Hz)
    double duty_arg = 1.0 - (pulse_ms / 20.0);

    TIMA1_PWM_DutyCycle(0, duty_arg);
}

void Servo_Set_Position(int8_t angle) {
    // Clamp the angle to the valid range [-90, 90]
    if (angle < -90) {
        angle = -90;
    }
    if (angle > 90) {
        angle = 90;
    }

    // Linearly map the angle from [-90, 90] to the pulse width [1.0ms, 2.0ms]
    double pulse_ms = CENTER_PULSE_MS + ((double)angle / 90.0) * (MAX_PULSE_MS - CENTER_PULSE_MS) / 2.0;

    Servo_Set_Pulse_ms(pulse_ms);
}
