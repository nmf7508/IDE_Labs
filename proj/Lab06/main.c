#include "lab6/timers.h"
#include "lab5/switches.h"
#include "lab4/uart.h"
#include "lab1/leds.h"
#include <ti/devices/msp/msp.h>


static volatile uint8_t motorRunning = 0;
static volatile double duty = 0.2; // 50% start
static volatile double direction = 0.01;
static volatile int counter = 0;
static volatile int counter2 = 0;

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
			if((counter%500) == 0){
				counter2 = 0;
			}
			if ((counter / 500)%2 == 0){
				TIMA0_PWM_DutyCycle(1, 0);
				if (counter2 > 20) {
				TIMA0_PWM_DutyCycle(0, duty);

				}
				counter2++;
			}
			else {
				if (counter2 > 20) {
					TIMA0_PWM_DutyCycle(0, 0);
				TIMA0_PWM_DutyCycle(1, duty);
				}
				counter2++;
			}
			//TIMA0_PWM_DutyCycle(1, duty);
			//TIMA0_PWM_DutyCycle(2, duty);
			//TIMA0_PWM_DutyCycle(3, ((double)1-duty));
			
			duty = duty + direction;
			if (duty >= 1 || duty <= 0) {
				direction = direction * (double)-1;
				//counter++;
			}
			TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
			while (!delayOver){
	
			}
			delayOver = 0;
			counter++;
    }
}
