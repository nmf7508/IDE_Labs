/**
 * ******************************************************************************
 * @file    : main.c
 * @brief   : main file for UART program
 * @details : UART reads and writes to serial terminal
 * 
 * @author  : Nick Fair
 * @date    : 09/10/2004
 * ******************************************************************************
*/

#include "lab2/uart.h"

/*
int main() {
	UART0_init();
	UART0_put("IDE: Lab 2 Demonstration by Nick Fair\r\n");
}
*/
int main() {
	UART0_init();
	uint8_t text[13] = {0};
	int index = 0;
	while (1) {
		UART0_put("Enter a sentence (no more than 10 characters)\r\n");
		while(1) {
			text[index] = UART0_getchar();
			UART0_putchar(text[index]);
			if (text[index] == '\r') {
				break;
			}
			if (index == 9) {
				index++;
				text[index] = '\r';
				UART0_putchar('\r');
				break;
			}
			index++;
		}
		text[index + 1] = '\n';
		text[index + 2] = 0x0;
		UART0_putchar('\n');
		UART0_put(text);
		index = 0;
	}
}
