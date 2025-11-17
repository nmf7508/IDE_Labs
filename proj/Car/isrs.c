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
static int integrationTime = 0;

// --- Global Flags for Car Control ---
volatile bool g_car_running = false;
volatile bool g_debug_mode = false;
volatile double g_Steer_Correction = 0.0; // Steering: -1.0 (L) to 1.0 (R)
volatile int16_t g_Drive_Speed = 30;     // Drive speed: -50 to 50
volatile uint32_t g_integration_time = 75;

static bool read;
 
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
			GPIOB->CPU_INT.ICLR = GPIO_GEN_EVENT1_ICLR_DIO21_CLR;
				
			g_debug_mode = !g_debug_mode; // Toggle the debug flag
		
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
			
    if (pixelCounter == 0) { // First clock edge after SI
        GPIOA->DOUTCLR31_0 |= (1 << 28); // Pull SI low
    }


    //if (read) {
        pixelCounter++;
        if (pixelCounter > 18 && pixelCounter <= (18 + 128)) {
            cameraData[pixelCounter - 19] = (uint16_t)ADC0_getVal();
        }
        if (pixelCounter >= (18 + 128)) {
            cameraData_complete = 1;
            pixelCounter = 0;
					  integrationTime = 0;
            TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED; // Stop CLK
        }
				GPIOA->DOUTCLR31_0 |= (1 << 12);
    //}
    //read = !read;
	}


/* --------------------------------------------------------------------------
 * Timer 6 Interrupt Handler (Camera SI)
 * -------------------------------------------------------------------------- */
void TIMG6_IRQHandler(void) {
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

    if (!cameraData_complete) {
        GPIOA->DOUTSET31_0 = (1 << 28);   // SI high
        pixelCounter = 0;               // Reset counter
        TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED; // Enable CLK
    }
    read = 1;
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
            // --- System Commands ---
            case 'g': // GO (Start line following)
                g_car_running = true;
                Motor_Set_Speed(g_Drive_Speed); // Start motors
                break;
            case 's': // STOP (Kill)
                g_car_running = false;
                Motor_Stop();
                break;
            case 't': // Toggle Debug
                g_debug_mode = !g_debug_mode;
                break;

            // --- Manual Drive Commands ---
            case 'w': // Set drive speed FORWARD
                g_Drive_Speed = 30;
                if(g_car_running) Motor_Set_Speed(g_Drive_Speed);
                break;
            case 'z': // Set drive speed BACKWARD
                g_Drive_Speed = -30;
                if(g_car_running) Motor_Set_Speed(g_Drive_Speed);
                break;
            case 'x': // Set drive speed STOP
                g_Drive_Speed = 0;
                Motor_Stop();
                break;

            // --- Manual Steering Commands ---
            case 'a': // Steer Left
                g_Steer_Correction -= 0.1;
                if (g_Steer_Correction < -1.0) g_Steer_Correction = -1.0;
                SteeringServo_Set_Turn(g_Steer_Correction); // Apply turn
                break;
            case 'd': // Steer Right
                g_Steer_Correction += 0.1;
                if (g_Steer_Correction > 1.0) g_Steer_Correction = 1.0;
                SteeringServo_Set_Turn(g_Steer_Correction); // Apply turn
                break;
            case 'c': // Center Steer
                g_Steer_Correction = 0.0;
                SteeringServo_Set_Turn(g_Steer_Correction); // Apply turn
                break;
        }
    }
}
