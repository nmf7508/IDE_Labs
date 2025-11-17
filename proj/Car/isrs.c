/**
 ******************************************************************************
 * @file     isrs.c
 * @brief    Interrupt Service Routines (ISRs) for Car Project (Servo Steer)
 ******************************************************************************
 */

#include <ti/devices/msp/msp.h>
#include "leds.h"
#include "isrs.h"
#include "uart.h"
#include "uart_extras.h"
#include "adc12.h"
#include "camera.h"
#include "timers.h"
#include <stdio.h>
#include <stdbool.h>
#include "motor.h"
#include "servo.h" 

/* --------------------------------------------------------------------------
 * Global Variables and Flags
 * -------------------------------------------------------------------------- */

volatile uint8_t cameraData_complete = 0;
volatile int pixelCounter = 0;
uint16_t cameraData[128];
volatile int delayOver = 0;

volatile bool g_car_running = false;

 
 /**
 * @brief Handles GPIOA interrupts (S1 - PA18).
 */
void GROUP1_IRQHandler(void) {
	switch(CPUSS->INT_GROUP[1].IIDX) {
		case 1:
				GPIOA->CPU_INT.ICLR = GPIO_GEN_EVENT1_ICLR_DIO18_CLR;
        
				g_car_running = true; // start the car running

			break;
		case 2:
			GPIOB->CPU_INT.ICLR = GPIO_GEN_EVENT1_ICLR_DIO21_CLR;
				
			// any code for S2
		
		break;
		default:
			break;
	}
}

/* --------------------------------------------------------------------------
 * Timer 0 Interrupt Handler (Camera CLK)
 * -------------------------------------------------------------------------- */
void TIMG0_IRQHandler(void) {
    TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
		
		// Toggle GPIO pin for CLK output
    GPIOA->DOUTSET31_0 |= (1 << 12);
	
		GPIOA->DOUTCLR31_0 |= (1 << 12);

		pixelCounter++;
		cameraData[pixelCounter] = (uint16_t)ADC0_getVal();
	
		if (pixelCounter == 128) {
				cameraData_complete = 1;
				pixelCounter = 0;
				TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED; // Stop CLK
		}
		
	}


/* --------------------------------------------------------------------------
 * Timer 6 Interrupt Handler (Camera SI)
 * -------------------------------------------------------------------------- */
void TIMG6_IRQHandler(void) {
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

		cameraData_complete = 0;
		pixelCounter = 0;               	// Reset counter
		GPIOA->DOUTSET31_0 = (1 << 28);   // SI high
		
		GPIOA->DOUTSET31_0 |= (1 << 12); 	// CLK high
		
		GPIOA->DOUTCLR31_0 |= (1 << 28); 	// Pull SI low
		
		GPIOA->DOUTCLR31_0 |= (1 << 12); 	// CLK low
		

		TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED; // Enable CLK
		
}


/* --------------------------------------------------------------------------
 * Timer 12 Interrupt Handler (Unused)
 * -------------------------------------------------------------------------- */
void TIMG12_IRQHandler(void) {
    TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
}

/* --------------------------------------------------------------------------
 * UART1 Interrupt Handler (Bluetooth Commands)
 * -------------------------------------------------------------------------- */
void UART1_IRQHandler(void) {
    if (UART1->CPU_INT.RIS & UART_CPU_INT_RIS_RXINT_SET) {
        UART1->CPU_INT.ICLR = UART_CPU_INT_ICLR_RXINT_CLR;
        char cmd = (char)(UART1->RXDATA & UART_RXDATA_DATA_MASK);

        switch (cmd) {
					
				}
    }
}
