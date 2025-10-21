#include "lab6/timers.h"
#include "lab5/switches.h"
#include "lab4/uart.h"
#include "lab1/leds.h"
#include <ti/devices/msp/msp.h>
#include "motor.h"
#include "servo.h"

// Delay function
static void delay(void) {
    volatile uint32_t i;
    for (i = 0; i < 2000000U; i++) {
    }
}

static volatile double duty = 0.3;

int main(void) {
    __disable_irq();

    LED1_init();
    UART0_init();
    S1_init_interrupt();
    S2_init_interrupt();

    TIMA0_PWM_init(0, 400, 0, duty);  // channel 0 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(1, 400, 0, 0);  // channel 1 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(2, 400, 0, duty);  // channel 2 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(3, 400, 0, 0);  // channel 3 - 20% Duty cycle @ 10KHz
    TIMA1_PWM_init(0, 313, 255, .075);  // channel 0 - 7.5% Duty cycle @ 50Hz
		TIMG0_init(1250, 255);

    __enable_irq();

    UART0_put((uint8_t *)"Motor Control Lab 6\r\n");
	
	    while (1) {
			 // LAB 6 Part 1 Code
			TIMA0_PWM_DutyCycle(1, 0);				delay();
			TIMA0_PWM_DutyCycle(0, 0);				delay();
			TIMA0_PWM_DutyCycle(0, 0.1);			delay();
			TIMA0_PWM_DutyCycle(0, 0.3);			delay();
			TIMA0_PWM_DutyCycle(0, 0.5);			delay();
			TIMA0_PWM_DutyCycle(0, 0.7);			delay();
			TIMA0_PWM_DutyCycle(0, 0.9);			delay();
			TIMA0_PWM_DutyCycle(0, 1);				delay();		delay();
			TIMA0_PWM_DutyCycle(0, 0.9);			delay();
			TIMA0_PWM_DutyCycle(0, 0.7);			delay();
			TIMA0_PWM_DutyCycle(0, 0.5);			delay();
			TIMA0_PWM_DutyCycle(0, 0.3);			delay();
			TIMA0_PWM_DutyCycle(0, 0.1);			delay();
			TIMA0_PWM_DutyCycle(0, 0);				delay();
			TIMA0_PWM_DutyCycle(1, 0.1);			delay();
			TIMA0_PWM_DutyCycle(1, 0.3);			delay();
			TIMA0_PWM_DutyCycle(1, 0.5);			delay();
			TIMA0_PWM_DutyCycle(1, 0.7);			delay();
			TIMA0_PWM_DutyCycle(1, 0.9);			delay();
			TIMA0_PWM_DutyCycle(1, 1);				delay();		delay();
			TIMA0_PWM_DutyCycle(1, 0.9);			delay();
			TIMA0_PWM_DutyCycle(1, 0.7);			delay();
			TIMA0_PWM_DutyCycle(1, 0.5);			delay();
			TIMA0_PWM_DutyCycle(1, 0.3);			delay();
			TIMA0_PWM_DutyCycle(1, 0.1);			delay();
			TIMA0_PWM_DutyCycle(1, 0);				delay();
					
    } 
	
		// Lab 6 Part 2 Code
		/** 
		Stepper_Motor_Init();

    UART0_put((uint8_t *)"Lab 6: Stepper Motor Demo\r\n");

    int forward = 0; // 1 for forward, 0 for reverse

    while (1) {
        Stepper_Motor_Step(forward);
        delay_ms(3); // Adjust this delay to change the motor speed
    }
		**/
		// Lab 6 Part 3 Code
		/**
		UART0_put((uint8_t *)"Lab 6: Servo Motor Demo\r\n");
		while(1) {
			delay_ms(1000);
			TIMA1_PWM_DutyCycle(0, .05);
			delay_ms(1000);
			TIMA1_PWM_DutyCycle(0, .075);
			delay_ms(1000);
			TIMA1_PWM_DutyCycle(0, .1);
			delay_ms(1000);
			TIMA1_PWM_DutyCycle(0, .075);
		}
		**/
		
}


