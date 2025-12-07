/**
 * @file    isrs.h
 * @brief   Interrupt Service Routines (ISRs) for Car Project
 * @details Function prototypes and external global variables for
 * interrupt handling.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
 */

#ifndef ISRS_H_
#define ISRS_H_

#include <stdbool.h>
#include <stdint.h>

/* --- Global Variables --- */
extern volatile int g_car_running;
extern volatile int selection;

/* --- ISR Function Prototypes --- */
void GROUP1_IRQHandler(void);
void UART1_IRQHandler(void); 
void TIMG0_IRQHandler(void); 
void TIMG6_IRQHandler(void); 
void TIMG12_IRQHandler(void);

#endif /* ISRS_H_ */
