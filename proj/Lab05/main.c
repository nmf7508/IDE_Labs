#include "lab4/uart.h"
#include "lab5/switches.h"
#include "lab5/timers.h"
#include "lab1/leds.h"

int main() {
	UART0_init();
	LED1_init();
	LED2_init();
	S1_init_interrupt();
	S2_init_interrupt();
	TIMG6_init(0, 0);
	TIMG12_init(0);
	
	while(1) {
		;
	}
}
