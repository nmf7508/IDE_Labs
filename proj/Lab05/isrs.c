/**
 ******************************************************************************
 * @file    isrs.c
 * @brief   Interrupt Service Routines (ISRs) for the MSPM0 microcontroller.
 * @details 
 *   This file implements interrupt handlers for hardware-triggered events
 *   including GPIO interrupts, timer overflows, and ADC conversions.
 *
 *   The behavior of each ISR is determined by the `MODE` macro, which enables
 *   flexible use of the same interrupt architecture across multiple labs:
 *
 *   | MODE | Functionality Description |
 *   |------|----------------------------|
 *   | 0 | Stopwatch/timing demonstration using timers and UART |
 *   | 1 | Periodic ADC sampling with UART output |
 *   | 2 | Temperature sensor sampling and unit conversion |
 *   | 3 | Line-scan camera capture sequence with synchronized CLK and SI |
 *
 *   Each ISR ensures interrupt flags are cleared, system timing remains
 *   synchronized, and peripheral-specific behavior is executed based on the
 *   current lab mode.
 *
 * @authors
 *   Nick Fair  
 *   Nathan Winiarski
 *
 * @date    October 7, 2025
 ******************************************************************************
 */

#include <ti/devices/msp/msp.h>
#include "lab1/leds.h"
#include "isrs.h"
#include "lab4/uart.h"
#include "uart_extras.h"
#include "lab5/adc12.h"
#include "lab5/camera.h"
#include "lab6/timers.h"
#include <stdio.h>

/* --------------------------------------------------------------------------
 *                          Global Variables and Flags
 * -------------------------------------------------------------------------- */

static int low_counter = 0;
static int threshold = 2000;
static uint32_t heart_rate = 0;

/* --------------------------------------------------------------------------
 *                        Timer 6 Interrupt Handler
 * -------------------------------------------------------------------------- */
void TIMG6_IRQHandler(void) {
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

		int val = (int)ADC0_getVal();
		//UART0_printDec(val);
		//UART0_put((uint8_t*)"\r\n");
	
		if (val <= threshold){
				low_counter++;
		}		
		else{
			GPIOA->DOUTTGL31_0 |= (1 << 28);
			heart_rate = (uint32_t)(58000/low_counter);
			if((int)heart_rate != -1){
				UART0_put((uint8_t*)"Heart Rate is: ");
				UART0_printDec((int)heart_rate);
				UART0_put((uint8_t*)"\r\n");
				low_counter = 0;
			}
		}
}
