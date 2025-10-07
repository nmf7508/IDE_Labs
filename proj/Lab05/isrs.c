/**
 * ******************************************************************************
 * @file    : isrs.c
 * @brief   : Interrupt Service Routines (ISRs) for the MSP Microcontroller
 * @details : 
 *   This file defines interrupt handlers that respond to hardware-triggered 
 *   events such as GPIO inputs, timer overflows, and ADC conversions. 
 *   The specific functionality of each handler depends on the current operation 
 *   mode defined by the `MODE` macro:
 *   
 *   - MODE 0: Stopwatch/timing demonstration using timers and UART output.
 *   - MODE 1: Periodic ADC sampling with UART transmission.
 *   - MODE 2: Temperature sensor data acquisition and temperature conversion.
 *   - MODE 3: Camera line capture sequence using ADC sampling synchronized 
 *                with timer-driven CLK and SI signals.
 *   
 *   Each interrupt routine manages clearing interrupt flags, handling GPIO toggles, 
 *   timing events, and data collection logic based on the active mode.
 * 
 * @authors 
 *   Nick Fair  
 *   Nathan Winiarski
 * 
 * @date   10/07/2025
 * ******************************************************************************
*/

#include <ti/devices/msp/msp.h>
#include "lab1/leds.h"
#include "isrs.h"
#include "lab4/uart.h"
#include "uart_extras.h"
#include "lab5/adc12.h"
#include "lab5/camera.h"
#include <stdio.h>

#if MODE == 0
static int timerOn = 0;        // Flag for stopwatch state (on/off)
static long int timeElapsed = 0; // Tracks elapsed time in ms
#endif

volatile uint8_t cameraData_complete = 0; // Set when full camera frame line captured
volatile int pixelCounter = 0;            // Counts CLK pulses for pixel capture (including dummy cycles)
uint16_t cameraData[128];                 // Buffer to store 128-pixel camera line
#if MODE == 3
static bool read;                         // Toggles between read and idle phases for CLK synchronization
#endif


/**
 * @brief Handles interrupts for the CPUSS GROUP1 interrupt group.
 * @details 
 *   This ISR is responsible for responding to external interrupt group events.
 *   Depending on which interrupt (INT0 or INT1) was triggered, different hardware 
 *   timers and LEDs are toggled. In MODE 0, it also handles stopwatch functionality 
 *   by starting/stopping timing and sending elapsed time via UART.
 */
void GROUP1_IRQHandler(void) {
	switch(CPUSS->INT_GROUP[1].IIDX) {
		case 1: // External interrupt 0
			// Clear interrupt flag
			CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT0;
#if MODE == 0 || MODE == 1 || MODE == 2
			// Toggle Timer 6 enable bit and LED1
			TIMG6->COUNTERREGS.CTRCTL ^= GPTIMER_CTRCTL_EN_ENABLED;
			LED1_set(LED1_TOGGLE);
#endif
			break;

		case 2: // External interrupt 1
			CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT1;
#if MODE == 0
			// Toggle Timer 12 and handle stopwatch timing
			TIMG12->COUNTERREGS.CTRCTL ^= GPTIMER_CTRCTL_EN_ENABLED;

			if (timerOn) { 
				// Stop the timer and display elapsed time
				LED2_set(0);
				timerOn = 0;
				char str[20];
				
				sprintf(str, "%ld", timeElapsed);
				UART0_put((uint8_t*) str);
				UART0_put((uint8_t*)" ms\r\n");
				timeElapsed = 0;
			}
			else {
				// Start the timer
				timerOn = 1;
			}
#endif
			break;

		default:
			break;
	}
}


/**
 * @brief Timer0 interrupt handler (used primarily in MODE 3 for camera clock generation).
 * @details 
 *   This ISR manages the pixel clock (CLK) for the camera module. It samples 
 *   pixel data from the ADC in synchronization with the CLK signal and stores 
 *   128 pixel values per capture. It also handles the required dummy cycles 
 *   before valid pixel data is available from the camera.
 */
void TIMG0_IRQHandler(void) {
	// Clear timer interrupt flag
	TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

#if MODE == 3
	// Toggle GPIO (CLK line simulation for debugging or external trigger)
	GPIOA->DOUTTGL31_0 |= (1 << 12);

	// At first pulse, ensure SI (Start Integration) line is low
	if (pixelCounter == 1) {
		GPIOA->DOUTCLR31_0 |= (1 << 28);
	}

	if (read) {
		pixelCounter++;

		// Skip the first 18 dummy cycles (no valid data)
		if (pixelCounter > 18 && pixelCounter <= (18 + 128)) {
			int idx = pixelCounter - 19;
			cameraData[idx] = (uint16_t)ADC0_getVal();
		}

		// Once all 128 pixels are captured
		if (pixelCounter >= (18 + 128)) {
			cameraData_complete = 1;  // Mark frame line as complete
			pixelCounter = 0;
			// Stop CLK until next SI signal is issued
			TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED;
		}
	}
	read = !read; // Toggle read state each clock edge
#endif
}


/**
 * @brief Timer6 interrupt handler.
 * @details 
 *   Behavior depends on MODE:
 *   - MODE 0: Toggles LED1 to indicate periodic timer events.
 *   - MODE 1: Samples ADC and sends raw ADC values via UART.
 *   - MODE 2: Reads ADC temperature sensor and converts voltage to °C and °F.
 *   - MODE 3: Generates camera "Start Integration" (SI) pulses and enables CLK timer.
 */
void TIMG6_IRQHandler(void) {
	TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

#if MODE == 0
	LED1_set(LED1_TOGGLE);

#elif MODE == 1
	// Sample ADC and print integer value
	int val = (int) ADC0_getVal();
	UART0_put((uint8_t *)"Sample: ");
	UART0_printDec(val);
	UART0_put((uint8_t *)"\r\n");

#elif MODE == 2
	// Convert ADC voltage reading to temperature
	int val = (int) ADC0_getVal();
	double tempC = ((((double) val * 3.3) / 4095.0) - 0.5) * 100.0;
	UART0_put((uint8_t *) "Temp in C: ");
	UART0_printFloat(tempC);
	UART0_put((uint8_t *) ", in F: ");
	UART0_printFloat(tempC * 9.0 / 5.0 + 32.0);
	UART0_put((uint8_t *) "\r\n");

#elif MODE == 3
	// Camera line capture control
	if (!cameraData_complete) {
		// Pulse SI (Start Integration) line to begin capture
		GPIOA->DOUTSET31_0 = (1 << 28);

		// Reset pixel counter and start CLK timer
		pixelCounter = 0;
		TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
	}
	read = 1; // Ensure next CLK edge triggers read
#endif
}


/**
 * @brief Timer12 interrupt handler.
 * @details 
 *   Used in MODE 0 to track elapsed time in milliseconds.
 *   Toggles LEDs sequentially every 500 ms as a visual time marker.
 */
void TIMG12_IRQHandler(void) {
	TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

#if MODE == 0
	// Every 500 ms, increment LED pattern
	if (timeElapsed % 500 == 0 && timeElapsed / 500 < 7) {
		LED2_set((LED2State)(timeElapsed / 500 + 1));
	}
	timeElapsed++; // Increment elapsed time counter
#endif
}
