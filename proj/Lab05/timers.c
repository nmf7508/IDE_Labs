#include "lab5/timers.h"
#include <ti\devices\msp\msp.h>
#include "sysctl.h"


/**
 * @brief Timer G0 module initialization. General purpose timer
 * @note Timer G0 is in Power Domain 0. Check page 3 of the Data Sheet
*/
void TIMG0_init(uint32_t period, uint32_t prescaler){
	//Check if power sequence has been run already, if not run reset and enable power
	if (!(TIMG0->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
		//Unlock RSTCTL assert reset then lock RSTCTL again
		TIMG0->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
		TIMG0->GPRCM.RSTCTL = TIMG0->GPRCM.RSTCTL & ~GPTIMER_RSTCTL_KEY_UNLOCK_W;
		
		//Unlock PWREN assert power enable then lock PWREN again
		TIMG0->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
		TIMG0->GPRCM.PWREN = TIMG0->GPRCM.PWREN & ~GPTIMER_PWREN_KEY_UNLOCK_W;
	}
	// Select BUSCLK (PD0)
	TIMG0->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;
	
	//Clear Counter Control Register
	TIMG0->COUNTERREGS.CTRCTL = 0;
	//Set timer for countdown mode
	TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
	//Set timer to load the load value on a zero event
	TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
	//Set timer to repeat on a zero event
	TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;
	//Set prescaler value to the inputted prescaler variable
	TIMG0->COMMONREGS.CPS = prescaler;
	//Set load value to the inputted period variable
	TIMG0->COUNTERREGS.LOAD = period;
	
	// Enable TIMCLK
	TIMG0->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;
	
	//disable interrupts
	__disable_irq();
	//Clear pending zero event interrupt
	TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
	//Enable zero event mask
	TIMG0->CPU_INT.IMASK |= GPTIMER_GEN_EVENT1_IMASK_Z_SET;
	//Enable TIMG0 interrupt
	TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
	//Enable TIMG0 interrupt on NVIC
	NVIC_EnableIRQ(TIMG0_INT_IRQn);
	//enable interrupts
	__enable_irq();
}


/**
 * @brief Timer G6 module initialization. General purpose timer
*/
void TIMG6_init(uint32_t period, uint32_t prescaler){
	//Check if power sequence has been run already, if not run reset and enable power
	if (!(TIMG6->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
		//Unlock RSTCTL assert reset then lock RSTCTL again
		TIMG6->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
		TIMG6->GPRCM.RSTCTL = TIMG6->GPRCM.RSTCTL & ~GPTIMER_RSTCTL_KEY_UNLOCK_W;
		
		//Unlock PWREN assert power enable then lock PWREN again
		TIMG6->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
		TIMG6->GPRCM.PWREN = TIMG6->GPRCM.PWREN & ~GPTIMER_PWREN_KEY_UNLOCK_W;
	}
	// Select BUSCLK (PD0)
	TIMG6->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;
	
	//Clear Counter Control Register
	TIMG6->COUNTERREGS.CTRCTL = 0;
	//Set timer for countdown mode
	TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
	//Set timer to load the load value on a zero event
	TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
	//Set timer to repeat on a zero event
	TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;
	//Set clock div to 8
	TIMG6->CLKDIV |= GPTIMER_CLKDIV_RATIO_DIV_BY_8;
	//Set prescaler value to the inputted prescaler variable
	TIMG6->COMMONREGS.CPS = prescaler;
	//Set load value to the inputted period variable
	TIMG6->COUNTERREGS.LOAD = period;
	
	// Enable TIMCLK
	TIMG6->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;
	
	//disable interrupts
	__disable_irq();
	//Clear pending zero event interrupt
	TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
	//Enable zero event mask
	TIMG6->CPU_INT.IMASK |= GPTIMER_GEN_EVENT1_IMASK_Z_SET;
	//Enable TIMG6 interrupt on NVIC
	NVIC_EnableIRQ(TIMG6_INT_IRQn);
	//enable interrupts
	__enable_irq();
}


/**
 * @brief Timer G12 module initialization. General purpose timer
 * @note Timer G12 has no prescaler
*/
void TIMG12_init(uint32_t period){
	//Check if power sequence has been run already, if not run reset and enable power
	if (!(TIMG12->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
		//Unlock RSTCTL assert reset then lock RSTCTL again
		TIMG12->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
		TIMG12->GPRCM.RSTCTL = TIMG12->GPRCM.RSTCTL & ~GPTIMER_RSTCTL_KEY_UNLOCK_W;
		
		//Unlock PWREN assert power enable then lock PWREN again
		TIMG12->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
		TIMG12->GPRCM.PWREN = TIMG12->GPRCM.PWREN & ~GPTIMER_PWREN_KEY_UNLOCK_W;
	}
	// Select BUSCLK (PD0)
	TIMG12->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;
	
	//Clear Counter Control Register
	TIMG12->COUNTERREGS.CTRCTL = 0;
	//Set timer for countdown mode
	TIMG12->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
	//Set timer to load the load value on a zero event
	TIMG12->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
	//Set timer to repeat on a zero event
	TIMG12->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;
	//Set load value to the inputted period variable
	TIMG12->COUNTERREGS.LOAD = period;
	
	// Enable TIMCLK
	TIMG12->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;
	
	//disable interrupts
	__disable_irq();
	//Clear pending zero event interrupt
	TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
	//Enable zero event mask
	TIMG12->CPU_INT.IMASK |= GPTIMER_GEN_EVENT1_IMASK_Z_SET;
	//Enable TIMG12 interrupt on NVIC
	NVIC_EnableIRQ(TIMG12_INT_IRQn);
	//enable interrupts
	__enable_irq();
}
