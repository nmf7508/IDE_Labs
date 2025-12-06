/**
 * @file    isrs.h
 * @brief   ISRs for Servo Range Test
 */

#ifndef _ISRS_H_
#define _ISRS_H_

#include <stdbool.h>
#include <stdint.h>

extern volatile int g_car_running;
extern volatile int selection;

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
void GROUP1_IRQHandler(void);
void UART1_IRQHandler(void); 
void TIMG0_IRQHandler(void); 
void TIMG6_IRQHandler(void); 
void TIMG12_IRQHandler(void);

#endif /* _ISRS_H_ */
