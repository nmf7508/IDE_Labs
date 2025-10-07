#include "lab4/uart.h"
#include "lab5/switches.h"
#include "lab5/timers.h"
#include "lab1/leds.h"
#include "isrs.h"
#include "lab5/adc12.h"
#include "uart_extras.h"
#include "lab5/camera.h"
//#include <stdio.h>

int main() {
	UART0_init();
	LED1_init();
	LED2_init();
	S1_init_interrupt();
	S2_init_interrupt();
	ADC0_init();
#if MODE == 0
	TIMG6_init(31250, 255); //2Hz freq for LED1
	TIMG12_init(32000);     //1kHz freq to track ms
	while (1) {}
#elif MODE == 1 || MODE == 2
	TIMG6_init(31250, 255); //period for ADC reads
	while (1) {}
#elif MODE == 3
	Camera_init();         // ADC + SI/CLK timers

	//UART0_put((uint8_t*)"\r\nLab 5 - Camera Test Start\r\n");

	while (1) {
			// Wait for camera
			if (Camera_isDataReady()) {
					UART0_put((uint8_t*)"-1\r\n");
					uint16_t* data = Camera_getData();

					// Indicate capture complete (toggle LED1)
					LED1_set(LED1_TOGGLE);

					// Stream pixel data over UART as CSV
					for (int i = 0; i < 128; i++) {
						UART0_printDec(data[i]);    // raw ADC value (0–4095)
						UART0_put((uint8_t*)"\r\n");
					}
					UART0_put((uint8_t*)"-2\r\n");
			}
	}
#endif
}


