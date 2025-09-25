#include <ti\devices\msp\msp.h>
#include "lab1/leds.h"
#include "isrs.h"

void GROUP1_IRQHandler(void) {
	switch(CPUSS->INT_GROUP[1].IIDX) {
		case 1:
			CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT0;
			LED1_set(LED1_TOGGLE);
			break;
		case 2:
			CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT1;
			LED2_set(LED2_MAGENTA);
		break;
		default:
			break;
	}
}
