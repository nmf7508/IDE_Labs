/**
 * @file    main.c
 * @brief   VISION DEBUG MODE
 * @details Visualizes what the camera sees on the OLED screen. 
 * Motors are DISABLED.
 */

#include <ti/devices/msp/msp.h>
#include <stdio.h>
#include <math.h> 
#include <stdlib.h>
#include "motor.h"
#include "servo.h"
#include "camera.h"    
#include "adc12.h"      
#include "oled.h" 
#include "switches.h" 
#include "isrs.h"           
#include "uart.h"       
#include "leds.h"
#include "uart_extras.h"

// Tuning for the servo reaction test
#define KP 1.5 

extern volatile bool g_car_running;

int main(void) {
    // --- INITIALIZATION ---
    __disable_irq();
    UART0_init();
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

    // --- SAFETY: KILL MOTORS ---
    Motor_Stop();
    Motor_Set_Speed(25, 25);

    OLED_display_clear();
    OLED_Print(1, 1, "VISION DEBUG");
    LED2_set(LED2_BLUE);

    // Static buffers
    static uint16_t line_data_smooth[128];
    static int16_t line_data_diff[128];

    while (1) {
        if (Camera_isDataReady()) {
            
            uint16_t* line_data = Camera_getData();

            for (int i = 1; i < 127; i++) {
                line_data_smooth[i] = (line_data[i-1] + line_data[i] + line_data[i+1]) / 3;
            }
            line_data_smooth[0] = line_data_smooth[1]; 
            line_data_smooth[127] = line_data_smooth[126];

            for (int i = 1; i < 127; i++) {
								//if (line_data_smooth[i+1] > line_data_smooth[i-1])
									line_data_diff[i] = (int16_t)line_data_smooth[i+1] - (int16_t)line_data_smooth[i-1];
								//else 
									//line_data_diff[i] = (int16_t)line_data_smooth[i-1] - (int16_t)line_data_smooth[i+1];
            }
						line_data_diff[0] = line_data_diff[1]; 
            line_data_diff[127] = line_data_diff[126];
            //for(int k=0; k<3; k++) line_data_diff[k] = 0;
            //for(int k=125; k<128; k++) line_data_diff[k] = 0;
						
						/*UART0_put("-1\r\n");
						for (int j = 0; j < 128; j++) {
							UART0_printDec(line_data[j]);
							UART0_put("\r\n");
						}
						UART0_put("-2\r\n");*/

            double raw_error = LineSensor_Calculate_Error(line_data_diff);

            //if (raw_error > -10.0) {
            SteeringServo_Set_Turn(raw_error); 
              //   LED2_set(LED2_GREEN);
            //} else {
             //    SteeringServo_Set_Turn(0); // Center if line lost
              //   LED2_set(LED2_RED);
            //}
            
            //if (raw_error < -10.0) {
            //    OLED_ShowWave(line_data_smooth, 0.0);
            //} else {
                OLED_ShowWave(line_data_smooth, -raw_error);
            //}
        }
    }
}
