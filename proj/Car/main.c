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
#define MAX_DUTY 35.0
#define MIN_DUTY 25.0
#define TURN_GAIN 5.0

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
    //static int16_t line_data_diff[128];

    while (1) {
        if (Camera_isDataReady()) {
            
            uint16_t* line_data = Camera_getData();

            for (int i = 1; i < 127; i++) {
                line_data_smooth[i] = (line_data[i-1] + line_data[i] + line_data[i+1]) / 3;
            }
            line_data_smooth[0] = line_data_smooth[1]; 
            line_data_smooth[127] = line_data_smooth[126];
						
						/*UART0_put("-1\r\n");
						for (int j = 0; j < 128; j++) {
							UART0_printDec(line_data_diff[j]);
							UART0_put("\r\n");
						}
						UART0_put("-2\r\n");*/

            double raw_error = LineSensor_Calculate_Error(line_data_smooth);
						if (raw_error < -1000) {
							Motor_Stop();
						}
						else {
						//UART0_printFloat(raw_error);
						//UART0_put("\r\n");
						//double PID_error = PID_Update(raw_error);

            //if (raw_error > -10.0) { 
						double control = clamp(raw_error, -1, 1);
						//double control = PID_Update(error_norm);   // normalized [-1,1]
						SteeringServo_Set_Turn(control);

						// 1. Speed based on turning strength
						double turn_mag = fabs(control);
						if (turn_mag > 1.0) turn_mag = 1.0;

						// straightness: 1 when straight, 0 on full turn
						double straightness = 1.0 - turn_mag;

						// base in [MIN_DUTY, MAX_DUTY]
						double base = MIN_DUTY + (MAX_DUTY - MIN_DUTY) * straightness;


						double turn = TURN_GAIN * control;

						double left_duty  = base + turn;
						double right_duty = base - turn;

						// Clamp each wheel to [MIN_DUTY, MAX_DUTY]
						if (left_duty  < MIN_DUTY) left_duty  = MIN_DUTY;
						if (left_duty  > MAX_DUTY) left_duty  = MAX_DUTY;

						if (right_duty < MIN_DUTY) right_duty = MIN_DUTY;
						if (right_duty > MAX_DUTY) right_duty = MAX_DUTY;

						Motor_Set_Speed((int16_t)left_duty, (int16_t)right_duty);
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
