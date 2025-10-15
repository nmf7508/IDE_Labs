#include "lab6/timers.h"
#include "lab5/switches.h"
#include "lab4/uart.h"
#include "lab1/leds.h"
#include <ti/devices/msp/msp.h>


static volatile uint8_t motorRunning = 0;
static volatile double duty = 0.2; // 50% start

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
			// main loop remains empty; interrupt-driven behavior
			TIMA0_PWM_DutyCycle(0, duty);
			TIMA0_PWM_DutyCycle(1, ((double)1-duty));
			TIMA0_PWM_DutyCycle(2, duty);
			TIMA0_PWM_DutyCycle(3, ((double)1-duty));
				
				
			duty = duty + 0.01;
			if (duty > 1) {
				duty = 0;
			}
			TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
			while (!delayOver){}
    }
}
