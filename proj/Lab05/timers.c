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
	
	enum SYSCTL_SYSCLK_FREQ clock_speed = SYSCTL_SYSCLK_getMCLK();
	double FtimCLK = (uint32_t)clock_speed / ((GPTIMER_CLKDIV_RATIO_DIV_BY_1 + 1) * (GPTIMER_CPS_PCNT_MASK & prescaler));
	
	
	// Enable TIMCLK
	TIMG0->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;
	
	TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
	
	TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
	
	TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;
	
	TIMG0->COMMONREGS.CPS = prescaler;
	
	TIMG0->COUNTERREGS.LOAD = period;
	
	__disable_irq();
	TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
	TIMG0->CPU_INT.IMASK |= GPTIMER_GEN_EVENT1_IMASK_Z_SET;
	TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
	NVIC_EnableIRQ(TIMG0_INT_IRQn);
	__enable_irq();
}


/**
 * @brief Timer G6 module initialization. General purpose timer
*/
void TIMG6_init(uint32_t period, uint32_t prescaler){
	
}


/**
 * @brief Timer G12 module initialization. General purpose timer
 * @note Timer G12 has no prescaler
*/
void TIMG12_init(uint32_t period){
	
}