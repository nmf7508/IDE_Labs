/**
 ******************************************************************************
 * @file    isrs.h
 * @brief   Interrupt Service Routine (ISR) header definitions.
 * @details
 * This header file declares the interrupt handler prototype for the
 * Lab 8 Heart Rate Monitor. It corresponds to the TIMG6 timer interrupt,
 * which is configured to fire at 1000 Hz to sample the ADC.
 *
 * @authors
 * Nick Fair  
 * Nathan Winiarski
 *
 * @date    November 10, 2025
 ******************************************************************************
 */

#ifndef _ISRS_H_
#define _ISRS_H_

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------

/**
 * @brief Timer6 interrupt handler (1000 Hz).
 * @details
 * Handles the 1ms periodic interrupt for the Heart Rate Monitor.
 * This ISR performs all core logic for the lab:
 * Reads the raw ADC value.
 * Applies a software low-pass filter to the raw data.
 * Updates a dynamic 'peak' and 'trough' envelope.
 * Recalculates the dynamic threshold (midpoint).
 * Detects a beat on a rising-edge threshold cross.
 * Calculates and prints the BPM to the UART.
 */
void TIMG6_IRQHandler(void);


#endif