/**
 ******************************************************************************
 * @file    motor.c
 * @brief   Stepper motor control implementation for Lab 6.
 * @details This file contains initialization and control routines for the
 *          stepper motor demonstration in Lab 6, Part 2.
 *
 *          The motor is driven by energizing one coil at a time in a 4-phase
 *          pattern to produce roation.
 *
 *          GPIO pins from Port B are configured as outputs and controlled
 *          directly to drive the motor’s coils.
 *
 * @authors
 *   Nick Fair  
 *   Nathan Winiarski
 *
 * @date    October 2025
 ******************************************************************************
 */

#include "motor.h"
#include "lab6/timers.h"  // Provides delay functions for timing control
#include <stdlib.h>

/* --------------------------------------------------------------------------
 *                           Stepper Motor Configuration
 * -------------------------------------------------------------------------- */

/**
 * @brief GPIO pin bitmasks for stepper motor coils on Port B.
 *
 * These correspond to the physical motor driver inputs:
 * - COIL_A_PIN -> 1A
 * - COIL_B_PIN -> 2A
 * - COIL_C_PIN -> 1B
 * - COIL_D_PIN -> 2B
 */
#define COIL_A_PIN  (1 << 6)   /**< Coil A output on PB6 */
#define COIL_B_PIN  (1 << 7)   /**< Coil B output on PB7 */
#define COIL_C_PIN  (1 << 0)   /**< Coil C output on PB0 */
#define COIL_D_PIN  (1 << 16)  /**< Coil D output on PB16 */

/**
 * @brief Keeps track of the current step phase (0–3).
 * Used to determine which coil to energize next.
 */
static int phase = 0;

/* --------------------------------------------------------------------------
 *                         Stepper Motor Initialization
 * -------------------------------------------------------------------------- */

/**
 * @brief Initialize GPIO pins and peripherals for the stepper motor.
 * @details
 *   - Enables Port B power.
 *   - Configures PB0, PB6, PB7, and PB16 as GPIO outputs.
 *   - Ensures all motor coils are de-energized initially.
 *
 * @note The pin mappings and function selections must match
 *       your LaunchPad’s IOMUX configuration.
 */
void Stepper_Motor_Init(void) {
    /* ---------------------- Enable GPIOB Peripheral ---------------------- */
    if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
    }

    /* ----------------------- Configure Output Pins ----------------------- */
    // PB6 -> Coil A
    IOMUX->SECCFG.PINCM[IOMUX_PINCM23] |= (IOMUX_PINCM23_PF_GPIOB_DIO06 | IOMUX_PINCM_PC_CONNECTED);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM23] &= ~IOMUX_PINCM_INENA_ENABLE;
    GPIOB->DOESET31_0 |= (1 << 6);

    // PB7 -> Coil B
    IOMUX->SECCFG.PINCM[IOMUX_PINCM24] |= (IOMUX_PINCM24_PF_GPIOB_DIO07 | IOMUX_PINCM_PC_CONNECTED);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM24] &= ~IOMUX_PINCM_INENA_ENABLE;
    GPIOB->DOESET31_0 |= (1 << 7);

    // PB0 -> Coil C
    IOMUX->SECCFG.PINCM[IOMUX_PINCM12] |= (IOMUX_PINCM12_PF_GPIOB_DIO00 | IOMUX_PINCM_PC_CONNECTED);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM12] &= ~IOMUX_PINCM_INENA_ENABLE;
    GPIOB->DOESET31_0 |= (1 << 0);

    // PB16 -> Coil D
    IOMUX->SECCFG.PINCM[IOMUX_PINCM33] |= (IOMUX_PINCM33_PF_GPIOB_DIO16 | IOMUX_PINCM_PC_CONNECTED);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM33] &= ~IOMUX_PINCM_INENA_ENABLE;
    GPIOB->DOESET31_0 |= (1 << 16);

    /* ---------------------- Set Data Output Enable ----------------------- */
    GPIOB->DOE31_0 |= (COIL_A_PIN | COIL_B_PIN | COIL_C_PIN | COIL_D_PIN);

    /* ---------------------- Turn Off All Coils --------------------------- */
    GPIOB->DOUTCLR31_0 = (COIL_A_PIN | COIL_B_PIN | COIL_C_PIN | COIL_D_PIN);
}

/* --------------------------------------------------------------------------
 *                           Stepper Motor Stepping
 * -------------------------------------------------------------------------- */

/**
 * @brief Perform one step in the wave-drive sequence.
 *
 * @param forward Direction flag:
 *   - `1` -> Rotate forward (clockwise)
 *   - `0` -> Rotate reverse (counter-clockwise)
 *
 * @details
 *   Implements a 4-step drive mode:
 *   ```
 *   Step | Coil Energized
 *   -----|----------------
 *    0   | A
 *    1   | B
 *    2   | C
 *    3   | D
 *   ```
 *   In reverse, the sequence is reversed (D -> C -> B -> A).
 *
 *   Only one coil is energized at a time to reduce power and simplify control.
 *
 * @note Each call to this function should be followed by a small delay
 */
void Stepper_Motor_Step(int forward) {
    /* ------------------- De-energize All Coils First -------------------- */
    GPIOB->DOUTCLR31_0 = (COIL_A_PIN | COIL_B_PIN | COIL_C_PIN | COIL_D_PIN);

    /* ---------------------- Determine Next Coil ------------------------- */
    if (forward) {
        // Forward sequence: A ? B ? C ? D
        if (phase == 0) GPIOB->DOUTSET31_0 = COIL_A_PIN;
        else if (phase == 1) GPIOB->DOUTSET31_0 = COIL_B_PIN;
        else if (phase == 2) GPIOB->DOUTSET31_0 = COIL_C_PIN;
        else GPIOB->DOUTSET31_0 = COIL_D_PIN;
    } else {
        // Reverse sequence: D ? C ? B ? A
        if (phase == 0) GPIOB->DOUTSET31_0 = COIL_D_PIN;
        else if (phase == 1) GPIOB->DOUTSET31_0 = COIL_C_PIN;
        else if (phase == 2) GPIOB->DOUTSET31_0 = COIL_B_PIN;
        else GPIOB->DOUTSET31_0 = COIL_A_PIN;
    }

    /* ------------------------ Advance Phase Index ----------------------- */
    phase = (phase + 1) % 4;  // Wraps around after 3
}
