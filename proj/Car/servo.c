#include "servo.h"
#include "timers.h" // Use your existing timer driver path

// --- Servo Configuration ---
// 32MHz / 8 (CLKDIV) / 256 (prescaler) = 15625 Hz
// 15625 Hz / 313 (period) = 50 Hz (20ms)
#define SERVO_PERIOD 313
#define SERVO_PRESCALER 255

// Servo pulse width (duty cycle)
#define SERVO_LEFT 0.097   // 10% duty cycle
#define SERVO_CENTER 0.073  // 7.5% duty cycle
#define SERVO_RIGHT 0.049   // 5% duty cycle

void SteeringServo_Init(void) {
    TIMA1_PWM_init(0, SERVO_PERIOD, SERVO_PRESCALER, SERVO_CENTER);
}

/**
 * @brief Clamps a value to a min/max range.
 */
static double clamp(double val, double min, double max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void SteeringServo_Set_Turn(double correction) {
    // Clamp correction value from -1.0 to +1.0
    correction = clamp(correction, -1.0, 1.0);

    // Map the correction value to the servo's duty cycle range
    double duty_cycle = SERVO_CENTER + (correction * (SERVO_LEFT - SERVO_CENTER));
    
    // Clamp to ensure we don't exceed servo limits
    duty_cycle = clamp(duty_cycle, SERVO_LEFT, SERVO_RIGHT);

    TIMA1_PWM_DutyCycle(0, duty_cycle);
}
