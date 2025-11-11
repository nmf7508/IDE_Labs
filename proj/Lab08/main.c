/**
 ******************************************************************************
 * @file    main.c
 * @brief   Main entry point for the Lab 8 Heart Rate Monitor.
 * @details
 * This file contains the main function for the project. It is responsible
 * for initializing the necessary peripherals:
 * UART0: For printing the calculated BPM to the serial terminal.
 * ADC0: For sampling the analog heart rate signal.
 * TIMG6: To generate the 1000 Hz interrupt that drives the ADC sampling
 * and all processing logic in the ISR.
 *
 * After initialization, the program enters an infinite loop, as all
 * application logic is handled by the TIMG6 interrupt service routine.
 *
 * @authors 
 * Nick Fair  
 * Nathan Winiarski
 *
 * @date    November 10, 2025
 ******************************************************************************
 */

#include <ti/devices/msp/msp.h>
#include "lab4/uart.h"
#include "lab5/adc12.h"
#include "uart_extras.h"
#include "lab5/timers.h"
#include "lab6/isrs.h"

int main() {
    // Initialize UART for debugging and BPM data transmission
    UART0_init();           
    
    // Initialize ADC for heart rate signal acquisition (PA27)
    ADC0_init();            
    
    // Initialize TIMG6 for 1ms / 1000 Hz periodic interrupts
    // (4000, 0) -> (32MHz/8) / 4000 = 1000 Hz
    TIMG6_init(4000, 0);
    
    // All logic is handled by the TIMG6_IRQHandler.
    // The main loop does nothing.
    while(1){
        // Wait for interrupts
        __WFI();
    }
}
