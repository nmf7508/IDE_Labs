/**
 * @file    main.c
 * @brief   Group 3 CMPE-460 Car Project (Line Follower)
 * @details Main control loop for the NXP Cup car. Handles initialization,
 * menu selection for speed, and the primary PID control loop.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
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

/* --- Constants & Macros --- */
#define MENU_WAIT_STATE     1024    // Value of g_car_running indicating menu mode
#define LINE_NOT_FOUND      -10000  // Error code when line is not visible
#define LINE_LOST_HISTORY   -60000  // Error code indicating we need to use last known error
#define TURN_GAIN_OFFSET    10.0    // Offset to keep turn gain lower than speed

/* --- Global Variables --- */
extern volatile int g_car_running;
extern volatile int selection;

int main(void) {
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
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

    // -------------------------------------------------------------------------
    // Menu Selection Loop
    // -------------------------------------------------------------------------
    const int speed_array[4] = {35, 40, 45, 50};
    int last_selection = -1;

    Motor_Stop();

    // Wait until S1 is pressed
    while(g_car_running == MENU_WAIT_STATE) {
        if(selection != last_selection) {
            OLED_display_clear();
            OLED_Print(1, 1, "S1: Start");
            OLED_Print(2, 1, "S2: Speed");

            char buf[16];
            sprintf(buf, "Speed: %d%%", speed_array[selection]);
            OLED_Print(3, 1, buf);

            last_selection = selection;
        }
        // Delay to prevent OLED flickering/bounce issues
        for(int i=0; i<10000; i++) __asm("nop");
    }

    // -------------------------------------------------------------------------
    // Control Setup
    // -------------------------------------------------------------------------
    double base_speed = (double)speed_array[selection];

    // Keep turn_gain 10 less than current speed
    double turn_gain = base_speed - TURN_GAIN_OFFSET;

    OLED_display_clear();
    OLED_Print(1, 1, "Running!");
    LED2_set(LED2_BLUE);

    static uint16_t line_data_smooth[128];
    double last_error = 0;

    // -------------------------------------------------------------------------
    // Main Control Loop
    // -------------------------------------------------------------------------
    while (1) {
        if (Camera_isDataReady()) {

            uint16_t* line_data = Camera_getData();

            // Simple 3-point Moving Average Filter for smoothing
            for (int i = 1; i < 127; i++) {
                line_data_smooth[i] = (line_data[i-1] + line_data[i] + line_data[i+1]) / 3;
            }
            // Handle edges
            line_data_smooth[0]   = line_data_smooth[1];
            line_data_smooth[127] = line_data_smooth[126];

            /*
            // MATLAB Debugging
            UART0_put("-1\r\n");
            for (int j = 0; j < 128; j++) {
                UART0_printDec(line_data[j]);
                UART0_put("\r\n");
            }
            UART0_put("-2\r\n");
            */

            // Calculate Error
            double raw_error = LineSensor_Calculate_Error(line_data_smooth);

            // Handle lost line or obscure sensor readings
            if ((int)raw_error == LINE_NOT_FOUND) {
                // Line completely gone -> Stop
                Motor_Stop();
            }
            else {
                // Check if we need to rely on previous error (Line Lost logic)
                if ((int)raw_error == LINE_LOST_HISTORY) {
                    raw_error = last_error;
                }

                // PID Calculation
                double control = PID_Update(raw_error);
                last_error = control; // Update history

                // Apply Steering
                SteeringServo_Set_Turn(control);

                // Differential Motor Control
                // Clamp error to ensure we don't speed up the inside wheel too much
                double clamped_error_l = (control > -0.3) ? 0 : control;
                double clamped_error_r = (control <  0.3) ? 0 : control;

                int16_t left_speed  = (int16_t)(base_speed + (turn_gain * clamped_error_l));
                int16_t right_speed = (int16_t)(base_speed - (turn_gain * clamped_error_r));

                Motor_Set_Speed(left_speed, right_speed);
            }
        }
    }
}
