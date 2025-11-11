/**
 ******************************************************************************
 * @file    adc12.h
 * @brief   ADC module header file for Lab 8 Heart Rate Monitor.
 * @details 
 * This file contains the function prototypes for initializing and
 * interacting with the ADC0 peripheral.
 *
 * @authors 
 * Nick Fair  
 * Nathan Winiarski
 *
 * @date    November 10, 2025
 ******************************************************************************
 */

#ifndef _ADC12_H_
#define _ADC12_H_

#include <stdint.h>

/**
 * @brief Initializes ADC0 for the Heart Rate Monitor.
 * @details Configures ADC0.0 (PA27) for software-triggered, single-channel
 * conversions.
 */
void ADC0_init(void);


/**
 * @brief Performs a single ADC conversion and returns the 12-bit result.
 * @details This is a blocking (polling) function that starts a conversion
 * and waits for it to complete. It is called by the
 * 1000 Hz TIMG6 interrupt.
 * @return ADC0 12-bit processed value (0-4095).
 */
uint32_t ADC0_getVal(void);


#endif