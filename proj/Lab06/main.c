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

static volatile double duty = 0.2;

int main(void) {
    __disable_irq();

    LED1_init();
    UART0_init();
    S1_init_interrupt();
    S2_init_interrupt();
		Servo_Init();

    TIMA0_PWM_init(0, 400, 0, duty);  // channel 0 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(1, 400, 0, duty);  // channel 1 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(2, 400, 0, duty);  // channel 2 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(3, 400, 0, duty);  // channel 3 - 20% Duty cycle @ 10KHz
    TIMA1_PWM_init(0, 400, 0, duty);  // channel 0
		TIMG0_init(1250, 255);

    TIMG6_init(800000, 1);  // blinking/heartbeat
    TIMG12_init(100000);     // timing or control loop

    __enable_irq();

    UART0_put((uint8_t *)"Motor Control Lab 6 Initialized\r\n");
	
		UART0_put((uint8_t *)"Lab 6 Part 3: Servo Control Demo\r\n");

    int8_t angle = 0;

    while (1) {
        // Sweep from -90 to +90 degrees
        UART0_put((uint8_t *)"Sweeping to +90 degrees...\r\n");
        for (angle = -90; angle <= 90; angle++) {
            Servo_Set_Position(angle);
            delay_ms(20); // Delay for smooth motion
        }
        
        delay_ms(500); // Pause at the end

        // Sweep from +90 back to -90 degrees
        UART0_put((uint8_t *)"Sweeping to -90 degrees...\r\n");
        for (angle = 90; angle >= -90; angle--) {
            Servo_Set_Position(angle);
            delay_ms(20);
        }

        delay_ms(500); // Pause at the end
    }
	
		/** LAB 6 PART 2 Code
		Stepper_Motor_Init();

    UART0_put((uint8_t *)"Lab 6: Stepper Motor Demo Initialized\r\n");

    int forward = 0; // 1 for forward, 0 for reverse

    while (1) {
        Stepper_Motor_Step(forward);
        delay_ms(3); // Adjust this delay to change the motor speed
    }
		**/
	
    /**while (1) {
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
					
    } **/
}
