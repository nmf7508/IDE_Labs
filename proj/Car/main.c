/**
 * @file    main.c
 * @brief   Main application file for CMPE460 Car Project (Servo Steer)
 * @authors Nick Fair, Nathan Winiarski
 * @date    November 2025
 */

#include <ti/devices/msp/msp.h>
#include <stdio.h>
#include "motor.h"         // New drive motor driver
#include "servo.h"         // New steering servo driver
#include "LineSensor.h"
#include "camera.h"
#include "adc12.h"
#include "oled.h"
#include "switches.h"
#include "isrs.h"
#include "uart.h"
#include "leds.h"
#include "timers.h"

// --- TUNING PARAMETERS ---
// Proportional gain for steering. You will need to tune this!
// It converts the pixel error (e.g., -64 to +64) to a steering value (-1.0 to 1.0)
#define LINE_KP 0.015 
// --- END TUNING ---

// Global variables from isrs.c
extern volatile bool g_car_running;
extern volatile bool g_debug_mode;
extern volatile double g_Steer_Correction;
extern volatile int16_t g_Drive_Speed;
extern volatile uint32_t g_integration_time;
extern volatile bool g_auto_exposure;

// --- Auto-Exposure Tuning ---
#define TARGET_BRIGHTNESS 3300
#define BRIGHTNESS_TOLERANCE 300
#define EXPOSURE_STEP 5
#define AUTO_EXPOSURE_FRAME_DIV 10

int main(void) {
    // --- 1. INITIALIZATION ---
    __disable_irq();

    S1_init_interrupt();
    S2_init_interrupt();
    LED2_init();
    Motor_Init();
    SteeringServo_Init();
    ADC0_init();
    Camera_init();
    OLED_Init();
    UART1_init();
    UART1_init_interrupt();

    __enable_irq();

    // --- PRINT COMMANDS TO UART ---
    UART1_put("--- Car Project (Servo Steer) ---\r\n");
    UART1_put(" 'g' or S1: Start Car\r\n");
    UART1_put(" 's': Stop Car (Kill)\r\n");
    UART1_put(" 'w'/'z'/'x': Drive Ctrl\r\n");
    UART1_put(" 'a'/'d'/'c': Steer Ctrl\r\n");
    UART1_put(" 't' or S2: Toggle Debug Data\r\n");
    UART1_put(" 'i'/'k': Manual Exposure\r\n");
    UART1_put(" 'e': Toggle Auto-Exposure\r\n");
    UART1_put("---------------------------------\r\n");
    UART1_put("Waiting for start command...\r\n");

    OLED_display_clear();
    OLED_Print(1, 1, "Demo 1");
    OLED_Print(3, 1, "Press S1 or 'g'");
    LED2_set(LED2_RED); 

    while (!g_car_running) {
        __asm("nop"); // Wait for S1 press or 'g' command
    }
        
    UART1_put("Start command received, Running\r\n");
    Motor_Set_Speed(g_Drive_Speed); // Start motors

    OLED_display_clear();
    OLED_Print(1, 1, "RUNNING!");
    
    char oled_buffer[16];
    char uart_buffer[48];
    int frame_counter = 0;

    // --- 2. MAIN CONTROL LOOP ---
    while (1) {
        if (Camera_isDataReady()) {
            
            uint16_t* line_data = Camera_getData();
            OLED_DisplayCameraData(line_data);

            // --- Auto-Exposure Logic ---
            if (g_auto_exposure && (frame_counter % AUTO_EXPOSURE_FRAME_DIV == 0)) {
                uint16_t max_val = 0;
                for (int i = 0; i < 128; i++) {
                    if (line_data[i] > max_val) max_val = line_data[i];
                }
                if (max_val > (TARGET_BRIGHTNESS + BRIGHTNESS_TOLERANCE)) {
                    if (g_integration_time > EXPOSURE_STEP) {
                         g_integration_time -= EXPOSURE_STEP;
                         TIMG6->COUNTERREGS.LOAD = g_integration_time;
                    }
                } else if (max_val < (TARGET_BRIGHTNESS - BRIGHTNESS_TOLERANCE)) {
                    g_integration_time += EXPOSURE_STEP;
                    TIMG6->COUNTERREGS.LOAD = g_integration_time;
                }
            }

            // --- Main Car Logic ---
            if (g_car_running) {
                if (LineSensor_Detect_Carpet(line_data)) {
                    Motor_Stop(); // Use new function
                    OLED_Print(1, 1, "CARPET STOP");
                    LED2_set(LED2_RED);
                }
                else {
                    if (g_debug_mode) { LED2_set(LED2_BLUE); }
                    else { LED2_set(LED2_GREEN); }
                    
                    // --- NEW STEERING LOGIC ---
                    int32_t error = LineSensor_Calculate_Error(line_data);
                    
                    // Calculate steering correction (-1.0 to 1.0)
                    double correction = (double)error * LINE_KP;
                    
                    // Apply steering
                    SteeringServo_Set_Turn(correction);
                    
                    // Drive motors are set by 'w'/'z'/'x' commands
                    
                    sprintf(oled_buffer, "Err: %d", (int)error);
                    OLED_Print(1, 1, oled_buffer);

                    if (g_debug_mode && (frame_counter % 20 == 0)) {
                        sprintf(uart_buffer, "e:%d c:%.2f\r\n", 
                            (int)error, correction);
                        UART1_put(uart_buffer);
                    }
                }
            } 
            else { // Car is NOT running (Killed)
                LED2_set(LED2_RED);
                OLED_Print(1, 1, "KILLED (s)");
                
                if (g_debug_mode && (frame_counter % 100 == 0)) {
                    UART1_put("State: Killed\r\n");
                }
            }
        }
        frame_counter++;
    }
}
