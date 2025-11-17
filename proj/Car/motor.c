#include "motor.h"
#include "lab6/timers.h" // Use your existing timer driver path
#include <ti/devices/msp/msp.h>

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
	
		//Set peripheral connected and configure PF for GPIO
		IOMUX->SECCFG.PINCM[IOMUX_PINCM45] |= (0x80 | 0x01);
		//Clear input enable bit, and invert output logic
		IOMUX->SECCFG.PINCM[IOMUX_PINCM45] &= ~IOMUX_PINCM_INENA_ENABLE;
		IOMUX->SECCFG.PINCM[IOMUX_PINCM45] |= IOMUX_PINCM_INV_ENABLE;
		GPIOB->DOESET31_0 |= (1 << 19);
			
		//Set peripheral connected and configure PF for GPIO
		IOMUX->SECCFG.PINCM[IOMUX_PINCM47] |= (0x80 | 0x01);
		//Clear input enable bit, and invert output logic
		IOMUX->SECCFG.PINCM[IOMUX_PINCM47] &= ~IOMUX_PINCM_INENA_ENABLE;
		IOMUX->SECCFG.PINCM[IOMUX_PINCM47] |= IOMUX_PINCM_INV_ENABLE;
		GPIOA->DOESET31_0 |= (1 << 22);
}

void Motor_Stop(void) {
    TIMA0_PWM_DutyCycle(0, 0);
    TIMA0_PWM_DutyCycle(1, 0);
    TIMA0_PWM_DutyCycle(2, 0);
    TIMA0_PWM_DutyCycle(3, 0);
}

void Motor_Set_Speed(int16_t left_speed, int16_t right_speed) {
    left_speed = clamp_speed(left_speed);
    double left_duty = (double)left_speed / 100.0;
	
		right_speed = clamp_speed(right_speed);
    double right_duty = (double)right_speed / 100.0;

    if (left_duty > 0) { // Forward
        // Set Left Rear
        TIMA0_PWM_DutyCycle(0, left_duty);
        TIMA0_PWM_DutyCycle(1, 0);
        // Set Right Rear
        TIMA0_PWM_DutyCycle(2, right_duty);
        TIMA0_PWM_DutyCycle(3, 0);
    } else { // Reverse or Stop
        // Set Left Rear
        TIMA0_PWM_DutyCycle(0, 0);
        TIMA0_PWM_DutyCycle(1, -left_duty);
        // Set Right Rear
        TIMA0_PWM_DutyCycle(2, 0);
        TIMA0_PWM_DutyCycle(3, -right_duty);
    }
}
