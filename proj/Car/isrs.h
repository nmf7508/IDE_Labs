/**
 * @file    isrs.h
 * @brief   ISRs for Servo Range Test
 */

#ifndef _ISRS_H_
#define _ISRS_H_

#include <stdbool.h>
#include <stdint.h>

// Global variable to hold the current duty cycle
extern volatile bool g_car_running;
extern volatile bool g_debug_mode;
extern volatile double g_Steer_Correction; // Steering: -1.0 (L) to 1.0 (R)
extern volatile int16_t g_Drive_Speed;     // Drive speed: -50 to 50
extern volatile uint32_t g_integration_time;

// Set MODE to 0 (or any non-3 value) to disable camera interrupts
#define MODE 0

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
void GROUP1_IRQHandler(void); // For S1
void UART1_IRQHandler(void); 
void TIMG0_IRQHandler(void); 
void TIMG6_IRQHandler(void); 
void TIMG12_IRQHandler(void);
void print_help_message(void);

#endif /* _ISRS_H_ */
