/**
 ******************************************************************************
 * @file    isrs.c
 * @brief   Interrupt Service Routines (ISRs) for the MSPM0 microcontroller.
 * @details 
 * This file implements the interrupt handler for the Lab 8 Heart Rate Monitor.
 * It uses the TIMG6 timer to trigger an ADC sample at 1000 Hz.
 *
 * The core logic in `TIMG6_IRQHandler` performs:
 * 1.  Software Filtering: A simple IIR low-pass filter smooths the raw ADC data.
 * 2.  Dynamic Thresholding: A 'decaying' peak and trough envelope tracks the
 * signals amplitude, constantly recalculating a dynamic midpoint.
 * 3.  Beat Detection: A beat is registered on a rising-edge cross of the
 * dynamic threshold.
 * 4.  BPM Calculation: The time between beats is used to calculate a raw BPM.
 *
 * @authors
 * Nick Fair  
 * Nathan Winiarski
 *
 * @date    November 10, 2025
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
 * Global Variables and Flags
 * -------------------------------------------------------------------------- */

static int peak = 0;                 // Running high-point, initialized low
static int trough = 4095;            // Running low-point, initialized high
static int dynamic_threshold = 2048; // The adaptive threshold
static int filtered_val = 2048;      // Software low-pass filtered ADC value

static int beat_sample_counter = 0;    // Time since last beat (in ms)
static bool was_below_threshold = true; // State-tracking for beat detection

/* --------------------------------------------------------------------------
 * Timer 6 Interrupt Handler
 * -------------------------------------------------------------------------- */
void TIMG6_IRQHandler(void) {
    // Clear the interrupt flag
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

    // Get the ADC value
    int raw_val = (int)ADC0_getVal();

    // Apply Software Low-Pass Filter
    filtered_val = ( (filtered_val * 3) + raw_val ) / 4;
    
    // Update Peak and Trough with Decay
    // If the new value is a peak, update the peak.
    if (filtered_val > peak) {
        peak = filtered_val;
    } 
    // Otherwise, make the peak decay slowly
    // This allows the threshold to drop if the signal amplitude decreases.
    else {
        peak--;
    }

    // If the new value is a trough, update the trough.
    if (filtered_val < trough) {
        trough = filtered_val;
    }
    // Otherwise, make the trough rise slowly
    // This allows the threshold to rise if the signal amplitude increases.
    else {
        trough++;
    }

    // Prevent peak and trough from crossing over
    if (peak < trough) {
        trough = peak;
    }

    // Recalculate the Threshold 
    dynamic_threshold = (peak + trough) / 2;

    // Beat Detection
    beat_sample_counter++; // Always count time
    
    bool is_currently_below = (filtered_val < dynamic_threshold);

    if (was_below_threshold && !is_currently_below) {
    
    // Calculate the raw BPM
    uint32_t heart_rate = (uint32_t)(60000 / beat_sample_counter);
        
		// Toggle debug LED on PA28
		GPIOA->DOUTTGL31_0 |= (1 << 28); 
		
		UART0_put((uint8_t*)"Heart Rate is: ");
		// Print the heart rate
		UART0_printDec((int)heart_rate);
		UART0_put((uint8_t*)"\r\n");
		
		// Reset the counter to start measuring time for the next beat
		beat_sample_counter = 0;
		}

    // 7. Save the current state for the next interrupt
    was_below_threshold = is_currently_below;
}
