/**
 * ******************************************************************************
 * @file     isrs.h
 * @brief    Interrupt Service Routine (ISR) header definitions.
 ******************************************************************************
 */

#ifndef _ISRS_H_
#define _ISRS_H_

#include <stdbool.h> // Make sure this is present

// --- Global Flags for Bluetooth Control ---
extern volatile bool g_car_running;     // true = car moves, false = car stops
extern volatile bool g_debug_mode;      // true = print debug data over UART1
extern volatile double g_Kp;            // For on-the-fly tuning
// ---

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------

/**
 * @brief Handles GPIOA interrupts (S1 - PA18).
 */
void GROUP1_IRQHandler(void);

/**
 * @brief Timer0 interrupt handler.
 * @details Generates camera CLK signal and reads ADC pixel data (MODE 3).
 */
void TIMG0_IRQHandler(void);

/**
 * @brief Timer6 interrupt handler.
 * @details Handles periodic timing for camera SI pulse generation.
 */
void TIMG6_IRQHandler(void);

/**
 * @brief Timer12 interrupt-handler.
 * @details Tracks elapsed time for stopwatch function (MODE 0).
 */
void TIMG12_IRQHandler(void);

/**
 * @brief UART1 interrupt handler.
 * @details Handles commands from the Bluetooth module.
 */
void UART1_IRQHandler(void);

#endif /* _ISRS_H_ */
