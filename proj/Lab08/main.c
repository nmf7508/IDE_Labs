#include <ti/devices/msp/msp.h>
#include "lab4/uart.h"
#include "lab5/adc12.h"
#include "uart_extras.h"
#include "lab5/timers.h"
#include "lab6/isrs.h"

int main() {
	UART0_init();           // UART for debugging and data transmission
	ADC0_init();            // Initialize ADC for data acquisition
	TIMG6_init(4000, 0);
	
	while(1){
	}
}
