/**
 * @file    main.c
 * @brief   Minimal main application to display live camera data.
 * @authors Nick Fair, Nathan Winiarski
 * @date    November 2025
 */

#include <ti/devices/msp/msp.h>
#include <stdio.h>
#include "camera.h"   // For the camera driver
#include "adc12.h"    // For the camera's ADC
#include "oled.h" // For the display
#include "uart.h"
#include "leds.h"

int main(void) {
    // --- 1. INITIALIZATION ---
    __disable_irq();
		UART1_init();
		UART1_put("UART1 Initialized\r\n");
    ADC0_init();         // For the camera
		UART1_put("ADC Initialized\r\n");
    Camera_init();       // Needs MODE 3 in isrs.h
		UART1_put("Camera Initialized\r\n");
    OLED_Init();         // For debugging
		UART1_put("OLED Initialized\r\n");
	
		UART1_put(" ALL Initialized\r\n");

    __enable_irq(); // Enable interrupts (REQUIRED for camera)

    OLED_display_clear();
    
    // --- 2. MAIN CONTROL LOOP ---
    while (1) {
        
        // Wait for camera to have a new line of data
        if (Camera_isDataReady()) {
            
            // 1. Get Sensor Data
            uint16_t* line_data = Camera_getData();
					
						LED1_set(LED1_TOGGLE);

            // 2. Always show camera data on OLED
            OLED_DisplayCameraData(line_data);
						UART1_put("Printing\r\n");
        }
    }
}
