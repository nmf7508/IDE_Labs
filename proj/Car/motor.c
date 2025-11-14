#include "motor.h"
#include "lab6/timers.h" // Use your existing timer driver path

#define PWM_PERIOD 400
#define MAX_SPEED 50

static int16_t clamp_speed(int16_t speed) {
    if (speed > MAX_SPEED) return MAX_SPEED;
    if (speed < -MAX_SPEED) return -MAX_SPEED;
    return speed;
}

void Motor_Init(void) {
    // Init all 4 channels with 0% duty cycle
    TIMA0_PWM_init(0, PWM_PERIOD, 0, 0); // Left Rear (AIN1)
    TIMA0_PWM_init(1, PWM_PERIOD, 0, 0); // Left Rear (AIN2)
    TIMA0_PWM_init(2, PWM_PERIOD, 0, 0); // Right Rear (BIN1)
    TIMA0_PWM_init(3, PWM_PERIOD, 0, 0); // Right Rear (BIN2)
}

void Motor_Stop(void) {
    TIMA0_PWM_DutyCycle(0, 0);
    TIMA0_PWM_DutyCycle(1, 0);
    TIMA0_PWM_DutyCycle(2, 0);
    TIMA0_PWM_DutyCycle(3, 0);
}

void Motor_Set_Speed(int16_t speed) {
    speed = clamp_speed(speed);
    double duty = (double)speed / 100.0;

    if (duty > 0) { // Forward
        // Set Left Rear
        TIMA0_PWM_DutyCycle(0, duty);
        TIMA0_PWM_DutyCycle(1, 0);
        // Set Right Rear
        TIMA0_PWM_DutyCycle(2, duty);
        TIMA0_PWM_DutyCycle(3, 0);
    } else { // Reverse or Stop
        // Set Left Rear
        TIMA0_PWM_DutyCycle(0, 0);
        TIMA0_PWM_DutyCycle(1, -duty);
        // Set Right Rear
        TIMA0_PWM_DutyCycle(2, 0);
        TIMA0_PWM_DutyCycle(3, -duty);
    }
}
