/**
 ******************************************************************************
 * @file     isrs.c
 * @brief    Interrupt Service Routines (ISRs) for the MSPM0 microcontroller.
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
#include "DCMotors.h"

/* --------------------------------------------------------------------------
 * Global Variables and Flags
 * -------------------------------------------------------------------------- */

volatile uint8_t cameraData_complete = 0; /**< Set to 1 when a full camera line is captured. */
volatile int pixelCounter = 0;            /**< Counts CLK pulses for pixel sampling (includes dummy cycles). */
uint16_t cameraData[128];                 /**< Storage buffer for one 128-pixel line. */
volatile int delayOver = 0;               /**< Delay completion flag, used for timer-driven events. */

// --- Global Flags for Bluetooth Control ---
volatile bool g_car_running = false;
volatile bool g_debug_mode = false;     // Shared with main.c, toggled by S2 or 't'
volatile double g_Kp = 0.6;             // Initial Kp value, shared with main.c

static bool read;                         /**< Toggles between read and idle states for camera CLK synchronization. */


/* --------------------------------------------------------------------------
 * GPIOA Interrupt Handler (S1 Button)
 * -------------------------------------------------------------------------- */

/**
 * @brief Handles GPIOA interrupts (S1 - PA18).
 */
void GROUP1_IRQHandler(void) {
	switch(CPUSS->INT_GROUP[1].IIDX) {
		case 1:
				GPIOA->CPU_INT.ICLR = GPIO_GEN_EVENT1_ICLR_DIO18_CLR;
        
        g_car_running = true; // Set the flag for main()
			break;
		case 2:
			
		break;
		default:
			break;
	}
}

/* --------------------------------------------------------------------------
 * Timer 0 Interrupt Handler (Camera CLK)
 * -------------------------------------------------------------------------- */
void TIMG0_IRQHandler(void) {
    // Clear interrupt flag
    TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
    // Toggle GPIO pin for CLK output or debug observation
    GPIOA->DOUTTGL31_0 |= (1 << 12);

    // First clock edge after SI pulse — ensure SI is pulled low
    if (pixelCounter == 1) {
        GPIOA->DOUTCLR31_0 |= (1 << 28);
    }

    if (read) {
        pixelCounter++;

        // Skip first 18 dummy cycles
        if (pixelCounter > 18 && pixelCounter <= (18 + 128)) {
            int idx = pixelCounter - 19;
            cameraData[idx] = (uint16_t)ADC0_getVal();
        }

        // All 128 pixels captured
        if (pixelCounter >= (18 + 128)) {
            cameraData_complete = 1;  // Mark frame complete
            pixelCounter = 0;
            TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED; // Stop CLK
        }
    }

    // Alternate read state between rising/falling clock edges
    read = !read;
}


/* --------------------------------------------------------------------------
 * Timer 6 Interrupt Handler (Camera SI)
 * -------------------------------------------------------------------------- */
void TIMG6_IRQHandler(void) {
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
	
    // Generate SI pulse and start camera frame capture
    if (!cameraData_complete) {
        GPIOA->DOUTSET31_0 = (1 << 28);   // SI high
        pixelCounter = 0;               // Reset counter
        TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED; // Enable CLK
    }
    read = 1; // Set to ensure first CLK edge captures data
}


/* --------------------------------------------------------------------------
 * Timer 12 Interrupt Handler
 * -------------------------------------------------------------------------- */
void TIMG12_IRQHandler(void) {
    TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

}

/* --------------------------------------------------------------------------
 * UART1 Interrupt Handler (Bluetooth Commands)
 * -------------------------------------------------------------------------- */
void UART1_IRQHandler(void) {
    // Check if the interrupt is a Receive Interrupt
    if (UART1->CPU_INT.RIS & UART_CPU_INT_RIS_RXINT_SET) {
        // Clear the receive interrupt flag
        UART1->CPU_INT.ICLR = UART_CPU_INT_ICLR_RXINT_CLR; // <-- Correct flag

        // Read the character from the data register
        char cmd = (char)(UART1->RXDATA & UART_RXDATA_DATA_MASK);

        switch (cmd) {
            case 's': // STOP (Kill Command)
                g_car_running = false;
                Motor_Stop(); // Call Motor_Stop() immediately
                break;
                
            case 'g': // GO (Resume)
                g_car_running = true;
                break;
                
            case 't': // TOGGLE Debug
                g_debug_mode = !g_debug_mode;
                break;
                
            case 'p': // Kp UP
                g_Kp += 0.05;
                break;
                
            case 'o': // Kp DOWN (can't use 'p-')
                g_Kp -= 0.05;
                if (g_Kp < 0) g_Kp = 0; // Don't allow negative Kp
                break;
        }
    }
}
