#include <ti\devices\msp\msp.h>
#include "lab1/leds.h"
#include "isrs.h"
#include "lab4/uart.h"
#include "uart_extras.h"
#include "lab5/adc12.h"
#include "lab5/camera.h"
#include <stdio.h>

#if MODE == 0
static int timerOn = 0;
static long int timeElapsed = 0;
#endif

volatile uint8_t cameraData_complete = 0;
volatile int pixelCounter = 0;       // counts CLK edges (including dummy cycles)
uint16_t cameraData[128]; // store 128 pixels
#if MODE == 3
static bool read;
#endif


void GROUP1_IRQHandler(void) {
	switch(CPUSS->INT_GROUP[1].IIDX) {
		case 1:
			CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT0;
#if MODE == 0 || MODE == 1 || MODE == 2
			TIMG6->COUNTERREGS.CTRCTL ^= GPTIMER_CTRCTL_EN_ENABLED;
			LED1_set(LED1_TOGGLE);
#endif
			break;
		case 2:
			CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT1;
#if MODE == 0
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
#endif
		break;
		default:
			break;
	}
}

void TIMG0_IRQHandler(void) {
	TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
#if MODE == 3
	GPIOA->DOUTTGL31_0 |= (1 << 12);
	if (pixelCounter == 1) {
		GPIOA->DOUTCLR31_0 |= (1 << 28);
	}
	if (read) {
	pixelCounter++;
	// Skip first 18 dummy cycles, then read 128 pixels
	if (pixelCounter > 18) {
			if (pixelCounter <= (18 + 128)) {
				int idx = pixelCounter - 19;
				cameraData[idx] = (uint16_t)ADC0_getVal();
			}
			
	}

	// Done capturing a full line
	if (pixelCounter >= (18 + 128)) {
			cameraData_complete = 1;
			pixelCounter = 0;
			TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED;  // stop CLK until next SI
	}
}
	read = !read;
#endif
}

void TIMG6_IRQHandler(void) {
	TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
#if MODE == 0
	LED1_set(LED1_TOGGLE);
#elif MODE == 1
	int val = (int) ADC0_getVal();
	UART0_put((uint8_t *)"Sample: ");
	UART0_printDec(val);
	UART0_put((uint8_t *)"\r\n");
#elif MODE == 2
	int val = (int) ADC0_getVal();
	double tempC = ((((double) val * (double) 3.3)/(double) 4095) - (double) 0.5) * (double) 100;
	UART0_put((uint8_t *) "Temp in C: ");
	UART0_printFloat(tempC);
	UART0_put((uint8_t *) ", in F: ");
	UART0_printFloat(tempC * 9.0 / 5.0 + 32.0);
	UART0_put((uint8_t *) "\r\n");
#elif MODE == 3
	//UART0_put("IN TIMG6\r\n");
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

    if (!cameraData_complete) {
        // Pulse SI high then low
        GPIOA->DOUTSET31_0 = (1 << 28);
        //GPIOA->DOUTCLR31_0 = (1 << 28);

        // Reset counters and start CLK
        pixelCounter = 0;
        TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
    }
		read = 1;
#endif
	
}

void TIMG12_IRQHandler(void) {
	TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
#if MODE == 0
	if (timeElapsed % 500 == 0 && timeElapsed/500 < 7) {
		LED2_set((LED2State)(timeElapsed/500 + 1));
	}
	timeElapsed++;
#endif
}
