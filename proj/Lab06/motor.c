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

// Part 2: Stepper Motor Control

// Stepper Motor GPIO pin definitions
#define COIL_A_PIN  (1 << 6)  // To 1a
#define COIL_B_PIN  (1 << 7)  // To 2a
#define COIL_C_PIN  (1 << 0)  // To 1b
#define COIL_D_PIN  (1 << 16) // To 2b

// Keeps track of the current phase of the stepper motor
static int phase = 0;

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
        if (phase == 0) GPIOB->DOUTSET31_0 = COIL_A_PIN; // Phase A
        else if (phase == 1) GPIOB->DOUTSET31_0 = COIL_B_PIN; // Phase B
        else if (phase == 2) GPIOB->DOUTSET31_0 = COIL_C_PIN; // Phase C
        else GPIOB->DOUTSET31_0 = COIL_D_PIN; // Phase D
    } else { // Reverse
        if (phase == 0) GPIOB->DOUTSET31_0 = COIL_D_PIN; // Phase D
        else if (phase == 1) GPIOB->DOUTSET31_0 = COIL_C_PIN; // Phase C
        else if (phase == 2) GPIOB->DOUTSET31_0 = COIL_B_PIN; // Phase B
        else GPIOB->DOUTSET31_0 = COIL_A_PIN; // Phase A
    }

    // Advance to the next phase for the next step
    phase = (phase + 1) % 4;
}
