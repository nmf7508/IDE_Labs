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
//#include "LineSensor.h"
#include "camera.h"   
#include "adc12.h"    
#include "oled.h" 
#include "switches.h" 
#include "isrs.h"          
#include "uart.h"     
#include "leds.h"
#include "uart_extras.h"


extern volatile bool g_car_running;

int main(void) {
    // --- 1. INITIALIZATION ---
    __disable_irq();

		UART0_init();
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
		Motor_Set_Speed(25, 25);
    OLED_display_clear();
    OLED_Print(1, 1, "RUNNING!");
		
    // --- 2. MAIN CONTROL LOOP ---
    while (1) {
        
        // Wait for camera to have a new line of data
        if (Camera_isDataReady()) {
            
            // 1. Get Sensor Data
            uint16_t* line_data = Camera_getData();

            // 2. Always show camera data on OLED
						//uint32_t average = 0;
						uint16_t line_data_smooth[128];
						int16_t line_data_diff[128];
						OLED_DisplayCameraData(line_data);
						//for (int i = 0; i < 128; i++) {
						//	average += line_data[i];
						//}
						//average = average/128;
						//if (average < 3000) {
						//	Motor_Stop();
						//}
						//else if (average < 40001) {
						for (int i = 1; i < 128 - 1; i++) {
								line_data_smooth[i] = (line_data[i-1] + line_data[i] + line_data[i+1]) / 3;
						}
						line_data_smooth[0] = line_data_smooth[1];
						line_data_smooth[128-1] = line_data_smooth[128-2];

						for (int i = 1; i < 128 - 1; i++) {
								line_data_diff[i] =
										(int16_t)line_data_smooth[i+1] - (int16_t)line_data_smooth[i-1];
						}
						line_data_diff[0] = line_data_diff[1];
						line_data_diff[128-1] = line_data_diff[128-2];
						//UART0_put("-1\r\n");
						//for (int i = 0; i < 126; i++) {
						//	UART0_printDec(line_data_diff[i]);
						//	UART0_put("\r\n");
						//}
						//UART0_put("-2\r\n");
						
						double valk = LineSensor_Calculate_Error(line_data_diff);
						//UART1_printFloat(valk);
						//if (valk <= 1) {
							//Motor_Set_Speed(25, 25);
						SteeringServo_Set_Turn(valk);
					//}
						//else SteeringServo_Set_Turn(0);
						//}
						//else (Motor_Stop());*/
						
				}
			}
		}
