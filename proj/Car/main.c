/**
 * ******************************************************************************
 * @file     main.c
 * @brief    Minimal main program to ONLY test the camera.
 * @details
 * This program initializes the ADC, UART0, and the Camera.
 * It does not initialize the motors, servo, OLED, or switches.
 *
 * The main loop waits for the camera data-ready flag, streams all
 * 128 pixels over UART0, and toggles LED1.
 * ******************************************************************************
 */

#include <ti/devices/msp/msp.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// --- Core and Camera Includes ---
#include "adc12.h"
#include "camera.h"
#include "timers.h"
#include "sysctl.h"
#include "isrs.h" // Still needed for the cameraData_complete flag

// --- Debugging Includes ---
#include "leds.h"
#include "uart.h"
#include "uart_extras.h" // For UART0_printDec()

// --- Unused Modules (commented out) ---
// #include "i2c.h"
// #include "motor.h"
// #include "oled.h"
// #include "servo.h"
// #include "switches.h"


// ============================================================================
// === 4. MAIN FUNCTION
// ============================================================================

int main(void) {
    
    // Initialize required peripherals for camera test
    ADC0_init();     // Camera needs the ADC
    UART0_init();    // For sending data to PC
    LED1_init();     // For a visual "heartbeat"
    Camera_init();   // Initializes TIMG0 (CLK) and TIMG6 (SI)

    // Set initial states
    LED1_set(LED1_OFF);
    
    // Send a ready message to the serial plotter
    UART0_put((uint8_t*)"Camera test ready...\r\n");

    // Enable all interrupts (CRITICAL for camera ISR)
    __enable_irq();
    
    while (1) {
        // Wait for the camera data ready flag to be set by ISR
        if (Camera_isDataReady()) {
            
            // Toggle LED1 to indicate capture completion (heartbeat)
            LED1_set(LED1_TOGGLE);

            // Retrieve 128-sample buffer
            uint16_t* data = Camera_getData(); 

            // --- Send Start-of-Frame Marker ---
            UART0_put((uint8_t*)"-1\r\n"); 

            // --- Stream pixel data over UART ---
            for (int i = 0; i < 128; i++) {
                UART0_printDec(data[i]);      // Print ADC sample
                UART0_put((uint8_t*)"\r\n");  // Line break
            }

            // --- Send End-of-Frame Marker ---
            UART0_put((uint8_t*)"-2\r\n"); 
        }
    }
}
