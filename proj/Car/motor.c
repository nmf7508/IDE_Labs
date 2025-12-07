/**
 * @file    motor.c
 * @brief   Motor Driver Implementation
 * @details Implements Differential Drive control using Timer A0 PWM.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
 */

#include "motor.h"
#include "timers.h"
#include <ti/devices/msp/msp.h>
#include <stdlib.h> 

/* --- Constants --- */
#define PWM_PERIOD      400
#define MAX_SPEED_LIMIT 50

/* --- Pin Mux Definitions --- */
// PB19 -> Motor Enable 1 (IOMUX_PINCM45)
#define PIN_ENABLE1_MASK    (1 << 19)
// PA22 -> Motor Enable 2 (IOMUX_PINCM47)
#define PIN_ENABLE2_MASK    (1 << 22)

/**
 * @brief Clamps the speed value to the defined MAX_SPEED_LIMIT.
 */
static int16_t clamp_speed(int16_t speed) {
    if (speed >  MAX_SPEED_LIMIT) return  MAX_SPEED_LIMIT;
    if (speed < -MAX_SPEED_LIMIT) return -MAX_SPEED_LIMIT;
    return speed;
}

void Motor_Init(void) {
    // 1. Initialize PWM Channels (0% duty cycle initially)
    TIMA0_PWM_init(0, PWM_PERIOD, 0, 0); // Left Rear (AIN1)
    TIMA0_PWM_init(1, PWM_PERIOD, 0, 0); // Left Rear (AIN2)
    TIMA0_PWM_init(2, PWM_PERIOD, 0, 0); // Right Rear (BIN1)
    TIMA0_PWM_init(3, PWM_PERIOD, 0, 0); // Right Rear (BIN2)
    
    // 2. Configure PB19 as Output (Motor Enable 1)
    IOMUX->SECCFG.PINCM[IOMUX_PINCM45] |= (0x80 | 0x01); // Connect Peripheral
    IOMUX->SECCFG.PINCM[IOMUX_PINCM45] &= ~IOMUX_PINCM_INENA_ENABLE; // Output
    IOMUX->SECCFG.PINCM[IOMUX_PINCM45] |= IOMUX_PINCM_INV_ENABLE;     // Invert Logic
    GPIOB->DOESET31_0 |= PIN_ENABLE1_MASK;
        
    // 3. Configure PA22 as Output (Motor Enable 2)
    IOMUX->SECCFG.PINCM[IOMUX_PINCM47] |= (0x80 | 0x01); // Connect Peripheral
    IOMUX->SECCFG.PINCM[IOMUX_PINCM47] &= ~IOMUX_PINCM_INENA_ENABLE; // Output
    IOMUX->SECCFG.PINCM[IOMUX_PINCM47] |= IOMUX_PINCM_INV_ENABLE;     // Invert Logic
    GPIOA->DOESET31_0 |= PIN_ENABLE2_MASK;
}

void Motor_Stop(void) {
    TIMA0_PWM_DutyCycle(0, 0);
    TIMA0_PWM_DutyCycle(1, 0);
    TIMA0_PWM_DutyCycle(2, 0);
    TIMA0_PWM_DutyCycle(3, 0);
}

void Motor_Set_Speed(int16_t left_speed, int16_t right_speed) {
    // Clamp inputs
    left_speed  = clamp_speed(left_speed);
    right_speed = clamp_speed(right_speed);

    // Calculate Duty Cycles (0.0 to 0.5 range based on MAX_SPEED_LIMIT)
    double left_duty  = abs(left_speed) / 100.0;
    double right_duty = abs(right_speed) / 100.0;

    // --- Control Left Motor ---
    if (left_speed > 0) {
        // Forward
        TIMA0_PWM_DutyCycle(0, left_duty);
        TIMA0_PWM_DutyCycle(1, 0);
    } else {
        // Reverse (or Stop)
        TIMA0_PWM_DutyCycle(0, 0);
        TIMA0_PWM_DutyCycle(1, left_duty);
    }

    // --- Control Right Motor ---
    if (right_speed > 0) {
        // Forward
        TIMA0_PWM_DutyCycle(2, right_duty);
        TIMA0_PWM_DutyCycle(3, 0);
    } else {
        // Reverse (or Stop)
        TIMA0_PWM_DutyCycle(2, 0);
        TIMA0_PWM_DutyCycle(3, right_duty);
    }
}
