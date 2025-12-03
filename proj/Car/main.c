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
#define MAX_DUTY 50.0
#define MIN_DUTY 35.0
#define TURN_GAIN 7.0

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
    //Motor_Set_Speed(25, 25);

    OLED_display_clear();
    OLED_Print(1, 1, "VISION DEBUG");
    LED2_set(LED2_BLUE);

    // Static buffers
    static uint16_t line_data_smooth[128];

    while (1) {
			double last_error = 0;
        if (Camera_isDataReady()) {
            
            uint16_t* line_data = Camera_getData();

            for (int i = 1; i < 127; i++) {
                line_data_smooth[i] = (line_data[i-1] + line_data[i] + line_data[i+1]) / 3;
            }
            line_data_smooth[0] = line_data_smooth[1]; 
            line_data_smooth[127] = line_data_smooth[126];
						
						/*
						UART0_put("-1\r\n");
						for (int j = 0; j < 128; j++) {
							UART0_printDec(line_data[j]);
							UART0_put("\r\n");
						}
						UART0_put("-2\r\n");
						*/

            double raw_error = LineSensor_Calculate_Error(line_data_smooth);
						if ((int)raw_error == -10000) {
							Motor_Stop();
						}
						else if ((int)raw_error == -60000) {
							raw_error = last_error;
						}
						else {
						//UART0_printFloat(raw_error);
						//UART0_put("\r\n");

            //if (raw_error > -10.0) { 
						double control = clamp(raw_error, -1, 1);
						// saving last error incase camera disconnects
						last_error = control;
						//double control = PID_Update(error_norm);   // normalized [-1,1]
						SteeringServo_Set_Turn(control);

						//SteeringServo_Set_Turn(raw_error);
						double clamped_error = clamp(raw_error, -1, 1);
						double clamped_error_l = clamped_error > -.3 ? 1.0 : clamped_error;
						double clamped_error_r = clamped_error < .3 ? -1.0 : clamped_error; 
						Motor_Set_Speed((int16_t)(MIN_DUTY+(TURN_GAIN*(clamped_error_l))), (int16_t)(MIN_DUTY-(TURN_GAIN*(clamped_error_r))));

						//Motor_Set_Speed((int16_t)left_duty, (int16_t)right_duty);
              //   LED2_set(LED2_GREEN);
            //} else {
             //    SteeringServo_Set_Turn(0); // Center if line lost
              //   LED2_set(LED2_RED);
            //}
            
            //if (raw_error < -10.0) {
            //    OLED_ShowWave(line_data_smooth, 0.0);
            //} else {
                //OLED_ShowWave(line_data_smooth, -raw_error);
            //}
					}
        }
    }
}
