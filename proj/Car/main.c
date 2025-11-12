/**
 * @file    main.c
 * @brief   Main application file for CMPE460 Car Project
 * @authors Nick Fair, Nathan Winiarski
 * @date    November 2025
 */

#include <ti/devices/msp/msp.h>
#include <stdio.h>
#include "DCMotors.h"
#include "LineSensor.h"
#include "camera.h"   // Update this path if needed
#include "adc12.h"    // Update this path if needed
#include "oled.h" // Update this path if needed
#include "switches.h" // Update this path if needed

// --- TUNING PARAMETERS ---
// Base speed (0-50) for "slow and ugly"
#define BASE_SPEED 30

// Proportional gain (KP). This is the "P" in PID.
// For Demo 1, we only need P-control.
#define KP 0.6
// --- END TUNING ---

// Global flag set by S1 interrupt in isrs.c
extern volatile bool g_car_running;

int main(void) {
    // --- 1. INITIALIZATION ---
    __disable_irq();

    S1_init_interrupt(); // For starting the car
    Motor_Init();        // Initializes TIMA0 for PWM
    ADC0_init();         // For the camera
    Camera_init();       // Needs MODE 3 in isrs.h
    OLED_Init();         // For debugging

    __enable_irq();

    OLED_display_clear();
    OLED_Print(1, 1, "Demo 1");
    OLED_Print(3, 1, "Press S1...");

    while (!g_car_running) {
        __asm("nop"); // Wait for S1 press
    }

    OLED_display_clear();
    OLED_Print(1, 1, "RUNNING!");
    
    char oled_buffer[16]; // Buffer for printing to OLED

    // --- 2. MAIN CONTROL LOOP ---
    while (1) {
        
        // Wait for camera to have a new line of data
        if (Camera_isDataReady()) {
            
            // 1. Get Sensor Data
            uint16_t* line_data = Camera_getData();

            // 2. Debug: Show camera data on OLED
            OLED_DisplayCameraData(line_data);

            // 3. Check for Demo 1 "Carpet Stopping"
            if (LineSensor_Detect_Carpet(line_data)) {
                Motor_Stop();
                OLED_Print(1, 1, "CARPET STOP");
            }
            // 4. Check for Demo 1 "Oval Line Following"
            else {
                // Find where the line is
                int32_t error = LineSensor_Calculate_Error(line_data);

                // Calculate correction using P-controller
                int16_t correction = (int16_t)(KP * (double)error);

                // Apply correction to motors
                Motor_Set_Speed(BASE_SPEED - correction, BASE_SPEED + correction);
                
                // Debug: Print error to OLED
                sprintf(oled_buffer, "Err: %ld", error);
                OLED_Print(1, 1, oled_buffer);
            }
        }
    }
}
