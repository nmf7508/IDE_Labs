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
#include "isrs.h"          // For global control flags
#include "uart.h"     // For Bluetooth UART1
#include "leds.h"     // For the RGB LED

// --- TUNING PARAMETERS ---
// Base speed (0-50) for "slow and ugly"
#define BASE_SPEED 30
// --- END TUNING ---

int main(void) {
    // --- 1. INITIALIZATION ---
    __disable_irq();

    S1_init_interrupt(); // For starting the car
    S2_init_interrupt(); // For toggling debug mode
    LED2_init();         // For RGB status LED
    Motor_Init();        // Initializes TIMA0 for PWM
    ADC0_init();         // For the camera
    Camera_init();       // Needs MODE 3 in isrs.h
    OLED_Init();         // For debugging
    UART1_init();        // For Bluetooth
    UART1_init_interrupt(); // For Bluetooth commands

    __enable_irq();

    // --- NEW: PRINT COMMANDS TO UART ---
    UART1_put("--- CMPE460 Car Project ---\r\n");
    UART1_put("Commands:\r\n");
    UART1_put(" 'g' or S1: Start Car\r\n");
    UART1_put(" 's': Stop Car (Kill)\r\n");
    UART1_put(" 't' or S2: Toggle Debug Data\r\n");
    UART1_put(" 'p': Kp Up (Tune)\r\n");
    UART1_put(" 'o': Kp Down (Tune)\r\n");
    UART1_put("---------------------------\r\n");
    UART1_put("Waiting for start command...\r\n");


    OLED_display_clear();
    OLED_Print(1, 1, "Demo 1");
    OLED_Print(3, 1, "Press S1 or 'g'");
    LED2_set(LED2_RED); // Set LED to RED (waiting)

    while (!g_car_running) {
        __asm("nop"); // Wait for S1 press or 'g' command
    }
        
    UART1_put("Start command received, Running\r\n");

    OLED_display_clear();
    OLED_Print(1, 1, "RUNNING!");
    
    char oled_buffer[16]; // Buffer for printing to OLED
    char uart_buffer[48]; // Buffer for Bluetooth debug
    int frame_counter = 0; // To slow down UART prints

    // --- 2. MAIN CONTROL LOOP ---
    while (1) {
        
        // Wait for camera to have a new line of data
        if (Camera_isDataReady()) {
            
            // 1. Get Sensor Data
            uint16_t* line_data = Camera_getData();

            // 2. Always show camera data on OLED
            OLED_DisplayCameraData(line_data);

            // 3. Check if car is allowed to run
            if (g_car_running) {
                // 3a. Check for Demo 1 "Carpet Stopping"
                if (LineSensor_Detect_Carpet(line_data)) {
                    Motor_Stop();
                    OLED_Print(1, 1, "CARPET STOP");
                    LED2_set(LED2_RED); // Stopped on carpet
                }
                // 3b. Check for Demo 1 "Oval Line Following"
                else {
                    
                    // --- LED LOGIC ---
                    if (g_debug_mode) {
                        LED2_set(LED2_BLUE); // Actively driving IN DEBUG
                    } else {
                        LED2_set(LED2_GREEN); // Actively driving NORMAL
                    }
                    
                    // Find where the line is
                    int32_t error = LineSensor_Calculate_Error(line_data);

                    // Calculate correction using the new GLOBAL Kp
                    // g_Kp is modified by the ISR for live tuning
                    int16_t correction = (int16_t)(g_Kp * (double)error);

                    // Apply correction to motors
                    Motor_Set_Speed(BASE_SPEED - correction, BASE_SPEED + correction);
                    
                    // Debug: Print error to OLED
                    sprintf(oled_buffer, "Err: %d", (int)error);
                    OLED_Print(1, 1, oled_buffer);

                    // --- DEBUG PRINT ---
                    // Only print every 20 frames to avoid lagging
                    if (g_debug_mode && (frame_counter % 20 == 0)) {
                        sprintf(uart_buffer, "e:%d c:%d Kp:%.2f\r\n", 
                            (int)error, correction, g_Kp);
                        UART1_put(uart_buffer);
                    }
                }
            } 
            // 4. Car is NOT running (Killed)
            else {
                LED2_set(LED2_RED); // Killed by user
                
                // We're not running, but keep OLED live
                OLED_Print(1, 1, "KILLED (s)");
                
                // Print to Bluetooth occasionally
                if (g_debug_mode && (frame_counter % 100 == 0)) {
                    UART1_put("State: Killed\r\n");
                }
            }
        }
        frame_counter++; // Increment frame counter
    }
}
