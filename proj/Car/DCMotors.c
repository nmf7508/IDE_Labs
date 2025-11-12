#include "DCMotors.h"
#include "timers.h" // Use your existing timer driver path

#define PWM_PERIOD 400  // From your previous lab's main.c
#define MAX_SPEED 50    // Project rule [cite: 21]

/**
 * @brief Clamps a speed value to the allowed -50 to +50 range.
 */
static int16_t clamp_speed(int16_t speed) {
    if (speed > MAX_SPEED) return MAX_SPEED;
    if (speed < -MAX_SPEED) return -MAX_SPEED;
    return speed;
}

void Motor_Init(void) {
    // Init all 4 channels with 0% duty cycle
    TIMA0_PWM_init(0, PWM_PERIOD, 0, 0); // Left Motor (AIN1)
    TIMA0_PWM_init(1, PWM_PERIOD, 0, 0); // Left Motor (AIN2)
    TIMA0_PWM_init(2, PWM_PERIOD, 0, 0); // Right Motor (BIN1)
    TIMA0_PWM_init(3, PWM_PERIOD, 0, 0); // Right Motor (BIN2)
}

void Motor_Stop(void) {
    // Required for carpet stopping [cite: 60]
    TIMA0_PWM_DutyCycle(0, 0);
    TIMA0_PWM_DutyCycle(1, 0);
    TIMA0_PWM_DutyCycle(2, 0);
    TIMA0_PWM_DutyCycle(3, 0);
}

void Motor_Set_Speed(int16_t leftSpeed, int16_t rightSpeed) {
    leftSpeed = clamp_speed(leftSpeed);
    rightSpeed = clamp_speed(rightSpeed);

    // Convert int speed (-50 to 50) to double duty cycle (0.0 to 0.5)
    double leftDuty = (double)leftSpeed / 100.0;
    double rightDuty = (double)rightSpeed / 100.0;

    // --- Left Motor ---
    if (leftDuty > 0) { // Forward
        TIMA0_PWM_DutyCycle(0, leftDuty);
        TIMA0_PWM_DutyCycle(1, 0);
    } else { // Reverse or Stop
        TIMA0_PWM_DutyCycle(0, 0);
        TIMA0_PWM_DutyCycle(1, -leftDuty); // -leftDuty becomes positive
    }

    // --- Right Motor ---
    if (rightDuty > 0) { // Forward
        TIMA0_PWM_DutyCycle(2, rightDuty);
        TIMA0_PWM_DutyCycle(3, 0);
    } else { // Reverse or Stop
        TIMA0_PWM_DutyCycle(2, 0);
        TIMA0_PWM_DutyCycle(3, -rightDuty); // -rightDuty becomes positive
    }
}
