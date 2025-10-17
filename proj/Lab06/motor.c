/**
 * @file motor.c
 * @brief Implementation of motor control functions.
 *
 * @authors
 * Nick Fair
 * Nathan Winiarski
 *
 * @date 10/16/2025
 */
#include "motor.h"
#include "lab6/timers.h" // Assuming your timer functions are here
#include <stdlib.h>


// A simple blocking delay function (can be shared by motor functions)
void delay_ms(uint32_t ms) {
    // This is a rough estimate; adjust the inner loop for your clock speed
    volatile uint32_t i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 4000; j++) {
            __NOP();
        }
    }
}

// ==============================================================================
// Part 1: DC Motor (H-Bridge) Control
// ==============================================================================

// PWM Period for 10kHz with 32MHz Clock (32M / 8 / 400 = 10k)
#define DC_MOTOR_PERIOD 400
#define DC_MOTOR_PRESCALER 0 // Assuming CLKDIV is set to 8 in TIMA0_PWM_init

void DC_Motor_Init(void) {
    // Initialize the PWM channels for both motors (Left: Ch0, Ch1; Right: Ch2, Ch3)
    TIMA0_PWM_init(0, DC_MOTOR_PERIOD, DC_MOTOR_PRESCALER, 0.0); // Left Fwd (PB8)
    TIMA0_PWM_init(1, DC_MOTOR_PERIOD, DC_MOTOR_PRESCALER, 0.0); // Left Rev (PB12)
    TIMA0_PWM_init(2, DC_MOTOR_PERIOD, DC_MOTOR_PRESCALER, 0.0); // Right Fwd (PB17)
    TIMA0_PWM_init(3, DC_MOTOR_PERIOD, DC_MOTOR_PRESCALER, 0.0); // Right Rev (PB13)

    // --- Initialize H-Bridge Enable Pins ---
    // Enable Power for GPIOA and GPIOB if not already done in PWM init
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
    }
    if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
    }
}

void DC_Motor_Set_Speed(uint8_t channel, int8_t speed) {
    uint8_t fwd_pin, rev_pin;

    if (channel == 0) { // Left Motor
        fwd_pin = 0;
        rev_pin = 1;
    } else { // Right Motor
        fwd_pin = 2;
        rev_pin = 3;
    }

    // Clamp speed to the range [-100, 100]
    if (speed > 100) speed = 100;
    if (speed < -100) speed = -100;

    double percentDutyCycle = abs(speed) / 100.0;

    if (speed > 0) { // Forward
        TIMA0_PWM_DutyCycle(fwd_pin, percentDutyCycle);
        TIMA0_PWM_DutyCycle(rev_pin, 0.0);
    } else if (speed < 0) { // Reverse
        TIMA0_PWM_DutyCycle(fwd_pin, 0.0);
        TIMA0_PWM_DutyCycle(rev_pin, percentDutyCycle);
    } else { // Stop
        TIMA0_PWM_DutyCycle(fwd_pin, 0.0);
        TIMA0_PWM_DutyCycle(rev_pin, 0.0);
    }
}

void DC_Motor_Stop(void) {
    DC_Motor_Set_Speed(0, 0); // Stop left motor
    DC_Motor_Set_Speed(1, 0); // Stop right motor
}

// ==============================================================================
// Part 2: Stepper Motor Control
// ==============================================================================

// Stepper Motor GPIO pin definitions
#define COIL_A_PORT GPIOB
#define COIL_A_PIN  (1 << 6)  // To 1a
#define COIL_B_PORT GPIOB
#define COIL_B_PIN  (1 << 7)  // To 2a
#define COIL_C_PORT GPIOB
#define COIL_C_PIN  (1 << 0)  // To 1b
#define COIL_D_PORT GPIOB
#define COIL_D_PIN  (1 << 16) // To 2b

// Keeps track of the current phase of the stepper motor
static int g_phase = 0;

void Stepper_Motor_Init(void) {
    // Power on GPIOB if not already on
    if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
    }
    // Configure all stepper pins as GPIO outputs
		IOMUX->SECCFG.PINCM[IOMUX_PINCM23] |= (IOMUX_PINCM23_PF_GPIOB_DIO06 | IOMUX_PINCM_PC_CONNECTED); //PB6
		IOMUX->SECCFG.PINCM[IOMUX_PINCM23] &= ~IOMUX_PINCM_INENA_ENABLE;
		GPIOB->DOESET31_0 |= (1 << 6);
		IOMUX->SECCFG.PINCM[IOMUX_PINCM24] |= (IOMUX_PINCM24_PF_GPIOB_DIO07 | IOMUX_PINCM_PC_CONNECTED); //PB7
		IOMUX->SECCFG.PINCM[IOMUX_PINCM24] &= ~IOMUX_PINCM_INENA_ENABLE;
		GPIOB->DOESET31_0 |= (1 << 7);
		IOMUX->SECCFG.PINCM[IOMUX_PINCM12] |= (IOMUX_PINCM12_PF_GPIOB_DIO00 | IOMUX_PINCM_PC_CONNECTED); //PB0
		IOMUX->SECCFG.PINCM[IOMUX_PINCM12] &= ~IOMUX_PINCM_INENA_ENABLE;
		GPIOB->DOESET31_0 |= (1 << 0);
		IOMUX->SECCFG.PINCM[IOMUX_PINCM33] |= (IOMUX_PINCM33_PF_GPIOB_DIO16 | IOMUX_PINCM_PC_CONNECTED); //PB16
		IOMUX->SECCFG.PINCM[IOMUX_PINCM33] &= ~IOMUX_PINCM_INENA_ENABLE;
		GPIOB->DOESET31_0 |= (1 << 16);

    GPIOB->DOE31_0 |= (COIL_A_PIN | COIL_B_PIN | COIL_C_PIN | COIL_D_PIN);

    // Ensure all coils are off initially
    GPIOB->DOUTCLR31_0 = (COIL_A_PIN | COIL_B_PIN | COIL_C_PIN | COIL_D_PIN);
}

// Implements "Wave Drive" stepping mode
void Stepper_Motor_Step(int forward) {
    // Turn off all coils before energizing the next one
    GPIOB->DOUTCLR31_0 = (COIL_A_PIN | COIL_B_PIN | COIL_C_PIN | COIL_D_PIN);

    if (forward) {
        if (g_phase == 0) GPIOB->DOUTSET31_0 = COIL_A_PIN; // Phase A
        else if (g_phase == 1) GPIOB->DOUTSET31_0 = COIL_B_PIN; // Phase B
        else if (g_phase == 2) GPIOB->DOUTSET31_0 = COIL_C_PIN; // Phase C
        else GPIOB->DOUTSET31_0 = COIL_D_PIN; // Phase D
    } else { // Reverse
        if (g_phase == 0) GPIOB->DOUTSET31_0 = COIL_D_PIN; // Phase D
        else if (g_phase == 1) GPIOB->DOUTSET31_0 = COIL_C_PIN; // Phase C
        else if (g_phase == 2) GPIOB->DOUTSET31_0 = COIL_B_PIN; // Phase B
        else GPIOB->DOUTSET31_0 = COIL_A_PIN; // Phase A
    }

    // Advance to the next phase for the next step
    g_phase = (g_phase + 1) % 4;
}
