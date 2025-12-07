/**
 * @file    isrs.c
 * @brief   Interrupt Service Routines (ISRs) for Car Project
 * @details Handles GPIO (Start/Select), Camera Timing (Timer 0/6),
 * and Bluetooth UART interrupts.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
 */

#include <ti/devices/msp/msp.h>
#include <stdio.h>
#include <stdbool.h>

#include "leds.h"
#include "isrs.h"
#include "uart.h"
#include "uart_extras.h"
#include "adc12.h"
#include "camera.h"
#include "timers.h"
#include "motor.h"
#include "servo.h"

/* --- Constants & Macros --- */
#define MENU_WAIT_STATE     1024
#define RUNNING_STATE       2183
#define CAM_WIDTH           128

// Camera GPIO Masks
#define CAM_CLK_PIN         (1 << 12)   // PA12
#define CAM_SI_PIN          (1 << 28)   // PA28

/* --- Global Variables --- */
volatile uint8_t cameraData_complete = 0;
volatile int pixelCounter = 0;
uint16_t cameraData[CAM_WIDTH];
volatile int delayOver = 0;

// State Machine Globals
volatile int g_car_running = MENU_WAIT_STATE;
volatile int selection = 0;

/**
 * @brief   Handles GPIOA interrupts (Button Presses)
 * @details GROUP1 Index 1: Switch 1 (Start)
 * 					GROUP1 Index 2: Switch 2 (Selection)
 */
void GROUP1_IRQHandler(void) {
    switch(CPUSS->INT_GROUP[1].IIDX) {
        case 1: // S1 Pressed (Start)
            GPIOA->CPU_INT.ICLR = GPIO_GEN_EVENT1_ICLR_DIO18_CLR;
            g_car_running = RUNNING_STATE;
            break;

        case 2: // S2 Pressed (Select Speed)
            GPIOB->CPU_INT.ICLR = GPIO_GEN_EVENT1_ICLR_DIO21_CLR;
            
            selection++;
            if (selection > 3) {
                selection = 0;
            }       
            break;

        default:
            break;
    }
}

/**
 * @brief   Timer 0 Interrupt Handler (Camera CLK)
 * @details Toggles the Camera Clock line and reads the ADC value for the current pixel.
 */
void TIMG0_IRQHandler(void) {
    TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
        
    // Toggle GPIO pin for CLK output
    GPIOA->DOUTSET31_0 |= CAM_CLK_PIN;
    GPIOA->DOUTCLR31_0 |= CAM_CLK_PIN;
    
    // Prevent buffer overflow if counter exceeds bounds
    if (pixelCounter < CAM_WIDTH) {
        cameraData[pixelCounter] = (uint16_t)ADC0_getVal();
				pixelCounter++;
    }
    
    // Stop after capturing a full line
    if (pixelCounter == CAM_WIDTH) {
        cameraData_complete = 1;
        pixelCounter = 0;
        TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED; // Stop CLK
    }
}

/**
 * @brief   Timer 6 Interrupt Handler (Camera SI - Start Pulse)
 * @details Generates the SI (Start Integration) pulse and starts the CLK timer.
 */
void TIMG6_IRQHandler(void) {
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

    cameraData_complete = 0;
    pixelCounter = 0;                   // Reset counter

    // Generate SI Pulse sequence
    GPIOA->DOUTSET31_0 = CAM_SI_PIN;    // SI High
    GPIOA->DOUTSET31_0 |= CAM_CLK_PIN;  // CLK High
    GPIOA->DOUTCLR31_0 |= CAM_SI_PIN;   // SI Low
    GPIOA->DOUTCLR31_0 |= CAM_CLK_PIN;  // CLK Low
    
    // Enable Timer 0 to start clocking in pixels
    TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED; 
}

/**
 * @brief   Timer 12 Interrupt Handler
 */
void TIMG12_IRQHandler(void) {
    TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
}

/**
 * @brief   UART1 Interrupt Handler (Bluetooth Commands)
 */
void UART1_IRQHandler(void) {
    if (UART1->CPU_INT.RIS & UART_CPU_INT_RIS_RXINT_SET) {
        UART1->CPU_INT.ICLR = UART_CPU_INT_ICLR_RXINT_CLR;
        
        // char cmd = (char)(UART1->RXDATA & UART_RXDATA_DATA_MASK);
        // Implement command switch statement here if needed
    }
}
