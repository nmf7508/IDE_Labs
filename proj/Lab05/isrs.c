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
 * Global Variables and Flags
 * -------------------------------------------------------------------------- */

static int peak = 0;                  // Running high-point, initialized low
static int trough = 4095;             // Running low-point, initialized high
static int dynamic_threshold = 2048;  // The adaptive threshold
static int filtered_val = 2048;       // Software low-pass filtered ADC value

static int beat_sample_counter = 0;   // Time since last beat (in ms)
static bool was_below_threshold = true; // State-tracking for beat detection

/* --------------------------------------------------------------------------
 * Timer 6 Interrupt Handler (1000 Hz)
 * Tuned for OPB745
 * -------------------------------------------------------------------------- */
void TIMG6_IRQHandler(void) {
    // 1. Clear the interrupt flag
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

    // 2. Get the raw ADC value
    int raw_val = (int)ADC0_getVal();

    // 3. --- Apply Software Low-Pass Filter ---
    // This smooths out the sensor noise *before* we process it.
    // We give 75% weight to the old filter value and 25% to the new sample.
    filtered_val = ( (filtered_val * 3) + raw_val ) / 4;
    
    // 4. --- Update Peak and Trough with Decay ---
    // This logic allows the threshold to adapt much faster than a 2-sec window.

    // If the new value is a peak, update the peak.
    if (filtered_val > peak) {
        peak = filtered_val;
    } 
    // Otherwise, make the peak "decay" slowly (e.g., by 1)
    // This allows the threshold to drop if the signal amplitude decreases.
    else {
        peak--;
    }

    // If the new value is a trough, update the trough.
    if (filtered_val < trough) {
        trough = filtered_val;
    }
    // Otherwise, make the trough "rise" slowly (e.g., by 1)
    // This allows the threshold to rise if the signal amplitude increases.
    else {
        trough++;
    }

    // Prevent peak and trough from crossing over
    if (peak < trough) {
        trough = peak;
    }

    // 5. --- Recalculate the Dynamic Threshold ---
    // Update the threshold on *every sample* based on the decaying peak/trough.
    dynamic_threshold = (peak + trough) / 2;

    // 6. --- Beat Detection (Rising-Edge Crossing) ---
    beat_sample_counter++; // Always count time
    
    bool is_currently_below = (filtered_val < dynamic_threshold);

    if (was_below_threshold && !is_currently_below) {
    
    // Calculate the raw BPM
    uint32_t heart_rate = (uint32_t)(60000 / beat_sample_counter);

    // --- APPLY CALIBRATION ---
    // Use floating-point math for calibration
		float calibrated_bpm_float = (0.9375f * (float)heart_rate) + 5.625f;
    
    // Convert back to an integer for printing
    uint32_t calibrated_bpm = (uint32_t)calibrated_bpm_float;


    // Sanity check for a realistic heart rate
    if (calibrated_bpm > 40 && calibrated_bpm < 200) {
        
        GPIOA->DOUTTGL31_0 |= (1 << 28); 
        
        UART0_put((uint8_t*)"Heart Rate is: ");
        // Print the *calibrated* value
        UART0_printDec((int)calibrated_bpm);
        UART0_put((uint8_t*)"\r\n");
        
        beat_sample_counter = 0;
    }
}

    // 7. Save the current state for the next interrupt
    was_below_threshold = is_currently_below;
}
