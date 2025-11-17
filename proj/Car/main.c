/**
 * @file    main.c
 * @brief   Main application file for CMPE460 Car Project
 * @authors Nick Fair, Nathan Winiarski
 * @date    November 2025
 */

#include <ti/devices/msp/msp.h>
#include <stdio.h>
#include "motor.h"
#include "servo.h"
#include "LineSensor.h"
#include "camera.h"   
#include "adc12.h"    
#include "oled.h" 
#include "switches.h" 
#include "isrs.h"          
#include "uart.h"     
#include "leds.h"


extern volatile bool g_car_running;

int main(void) {
    // --- 1. INITIALIZATION ---
    __disable_irq();

    S1_init_interrupt(); // For starting the car
    S2_init_interrupt(); // For toggling debug mode
    LED2_init();         // For RGB status LED
    Motor_Init();        // Initializes TIMA0 for PWM
		SteeringServo_Init(); // Init Servo 
    ADC0_init();         // For the camera
    Camera_init();       // Needs MODE 3 in isrs.h
    OLED_Init();         // For debugging
    UART1_init();        // For Bluetooth
    UART1_init_interrupt(); // For Bluetooth commands

    __enable_irq();

    OLED_display_clear();
    OLED_Print(1, 1, "Demo 1");
    OLED_Print(3, 1, "Press S1 or 'g'");
    LED2_set(LED2_RED); // Set LED to RED (waiting)

    while (!g_car_running) {
        __asm("nop"); // Wait for S1 press or 'g' command
    }

    OLED_display_clear();
    OLED_Print(1, 1, "RUNNING!");

    // --- 2. MAIN CONTROL LOOP ---
    while (1) {
        
        // Wait for camera to have a new line of data
        if (Camera_isDataReady()) {
            
            // 1. Get Sensor Data
            uint16_t* line_data = Camera_getData();

            // 2. Always show camera data on OLED
            OLED_DisplayCameraData(line_data);
				}
			}
		}
