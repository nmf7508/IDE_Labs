#include "lab4/uart.h"
#include "lab5/switches.h"
#include "lab5/timers.h"
#include "lab1/leds.h"
#include "isrs.h"
#include "lab5/adc12.h"
//#include <stdio.h>

int main() {
	UART0_init();
	LED1_init();
	LED2_init();
	S1_init_interrupt();
	S2_init_interrupt();
	ADC0_init();
	TIMG6_init(31250/4, 255);
	TIMG12_init(32000);
	//TIMG12_init(0);
	
	while(1) {
	}
}

