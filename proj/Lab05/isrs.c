#include <ti\devices\msp\msp.h>
#include "lab1/leds.h"
#include "isrs.h"
#include "lab4/uart.h"
#include "uart_extras.h"
#include "lab5/adc12.h"
#include <stdio.h>

static int timerOn = 0;
static long int timeElapsed = 0;


void GROUP1_IRQHandler(void) {
	switch(CPUSS->INT_GROUP[1].IIDX) {
		case 1:
			CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT0;
			TIMG6->COUNTERREGS.CTRCTL ^= GPTIMER_CTRCTL_EN_ENABLED;
			break;
		case 2:
			CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT1;
			TIMG12->COUNTERREGS.CTRCTL ^= GPTIMER_CTRCTL_EN_ENABLED;
			if (timerOn) { 
				LED2_set(0);
				timerOn = 0;
				char str[20];
				
				sprintf(str, "%ld", timeElapsed);
				UART0_put((uint8_t*) str);
				UART0_put((uint8_t*)" ms\r\n");
				timeElapsed = 0;
			}
			else {
				timerOn = 1;
			}
		break;
		default:
			break;
	}
}

void TIMG0_IRQHandler(void) {
	TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
	LED1_set(LED1_TOGGLE);
}

void TIMG6_IRQHandler(void) {
	TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
	LED1_set(LED1_TOGGLE);
	UART0_put((uint8_t *)"Sample: ");
	UART0_printDec((int)ADC0_getVal());
	UART0_put((uint8_t *)"\r\n");
}

void TIMG12_IRQHandler(void) {
	TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
	if (timeElapsed % 500 == 0 && timeElapsed/500 < 7) {
		LED2_set((LED2State)(timeElapsed/500 + 1));
	}
	timeElapsed++;
}
