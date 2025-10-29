#include "lab4/uart.h"
#include "lab5/adc12.h"
#include "uart_extras.h"
#include "lab5/timers.h"
#include <ti/devices/msp/msp.h>


static int low_counter = 0;
static int threshold = 2000;
static uint32_t heart_rate = 0;

int main() {
	UART0_init();           // UART for debugging and data transmission
	ADC0_init();            // Initialize ADC for data acquisition
	TIMG6_init(100, 255);
	
    // Initialize Timer6 with a period and duty cycle for integration timing
    TIMG6_init(100, 255);

    // Enable and configure GPIOA peripheral if not already active
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    // Configure PA28 as output (no input, no inversion)
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] &= ~IOMUX_PINCM_INENA_ENABLE;

    // Enable Timer6 (starts integration timing) and PA28 output
    TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
    GPIOA->DOESET31_0 |= (1 << 28);

	
	while (1) {
		int val = (int)ADC0_getVal();
		UART0_put((uint8_t*)"value is: ");
		UART0_printDec(val);
		UART0_put((uint8_t*)"\r\n");
		if (val <= threshold){
				low_counter++;
		}
		else{
			heart_rate = (uint32_t)(60000/low_counter);
			UART0_put((uint8_t*)"Heart Rate is: ");
			UART0_printDec((int)heart_rate);
			UART0_put((uint8_t*)"\r\n");
			low_counter = 0;
		}
	}
}
