#include "lab6/timers.h"
#include "lab5/switches.h"
#include "lab4/uart.h"
#include "lab1/leds.h"
#include <ti/devices/msp/msp.h>

// Delay function
static void delay(void) {
    volatile uint32_t i;
    for (i = 0; i < 2000000U; i++) {
    }
}

static volatile uint8_t motorRunning = 0;
static volatile double duty = 0.2;

// Delay function
static void delay(void) {
    volatile uint32_t i;
    for (i = 0; i < 2000000U; i++) {
    }
}

int main(void) {
    __disable_irq();

    LED1_init();
    UART0_init();
    S1_init_interrupt();
    S2_init_interrupt();

    TIMA0_PWM_init(0, 400, 0, duty);  // channel 0 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(1, 400, 0, duty);  // channel 0 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(2, 400, 0, duty);  // channel 0 - 20% Duty cycle @ 10KHz
		TIMA0_PWM_init(3, 400, 0, duty);  // channel 0 - 20% Duty cycle @ 10KHz
    TIMA1_PWM_init(0, 400, 0, duty);  // channel 0 for other side
		TIMG0_init(1250, 255);

    TIMG6_init(800000, 1);  // blinking/heartbeat
    TIMG12_init(100000);     // timing or control loop

    __enable_irq();

    UART0_put((uint8_t *)"Motor Control Lab 6 Initialized\r\n");
		duty = 0;
    while (1) {
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
}
