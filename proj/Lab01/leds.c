/**
 * ******************************************************************************
 * @file    : leds.c
 * @brief   : LEDs module
 * @details : LED initialization and interaction
 * 
 * @author  : Nicholas Fair
 * @date    : 09/01/2025
 * ******************************************************************************
*/

#include <ti\devices\msp\msp.h>
#include "lab1\leds.h"

/**
 * Reset and initialize power for LED 1 if not done already
 *
 * @return None
*/
void LED1_init(void) {
	//Check if power sequence has been run already, if not run reset and enable power
	if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
		//Unlock RSTCTL assert reset then lock RSTCTL again
		GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
		GPIOA->GPRCM.RSTCTL = GPIOA->GPRCM.RSTCTL & ~GPIO_RSTCTL_KEY_UNLOCK_W;
		
		//Unlock PWREN assert power enable then lock PWREN again
		GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
		GPIOA->GPRCM.PWREN = GPIOA->GPRCM.PWREN & ~GPIO_PWREN_KEY_UNLOCK_W;
	}
	
	//Set peripheral connected and configure PF for GPIO
	IOMUX->SECCFG.PINCM[IOMUX_PINCM1] |= (0x80 | 0x01);
	//Clear input enable bit, and invert output logic
	IOMUX->SECCFG.PINCM[IOMUX_PINCM1] &= ~IOMUX_PINCM_INENA_ENABLE;
	IOMUX->SECCFG.PINCM[IOMUX_PINCM1] |= IOMUX_PINCM_INV_ENABLE;
	
	//Enable LED1 for output
	GPIOA->DOESET31_0 |= GPIOA_LED1;
}

/**
 * Reset and initialize power for LED 2 if not done already
 *
 * @return None
*/
void LED2_init(void) {
	//Check if power sequence has been run already, if not run reset and enable power
	if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
		//Unlock RSTCTL assert reset then lock RSTCTL again
		GPIOB->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
		GPIOB->GPRCM.RSTCTL = GPIOB->GPRCM.RSTCTL & ~GPIO_RSTCTL_KEY_UNLOCK_W;
		
		//Unlock PWREN assert power enable then lock PWREN again
		GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
		GPIOB->GPRCM.PWREN = GPIOB->GPRCM.PWREN & ~GPIO_PWREN_KEY_UNLOCK_W;
	}
	
	//Set regs for Red LED
	//Set peripheral connected and configure PF for GPIO
	IOMUX->SECCFG.PINCM[IOMUX_PINCM57] |= (0x80 | 0x01);
	//Clear input enable bit
	IOMUX->SECCFG.PINCM[IOMUX_PINCM57] &= ~IOMUX_PINCM_INENA_ENABLE;
	
	//Enable LED2_R for output
	GPIOB->DOESET31_0 |= GPIOB_LED2_R;
	
	//Set regs for Green LED
	//Set peripheral connected and configure PF for GPIO
	IOMUX->SECCFG.PINCM[IOMUX_PINCM58] |= (0x80 | 0x01);
	//Clear input enable bit
	IOMUX->SECCFG.PINCM[IOMUX_PINCM58] &= ~IOMUX_PINCM_INENA_ENABLE;
	
	//Enable LED2_G for output
	GPIOB->DOESET31_0 |= GPIOB_LED2_G;
	
	//Set regs for Blue LED
	//Set peripheral connected and configure PF for GPIO
	IOMUX->SECCFG.PINCM[IOMUX_PINCM50] |= (0x80 | 0x01);
	//Clear input enable bit
	IOMUX->SECCFG.PINCM[IOMUX_PINCM50] &= ~IOMUX_PINCM_INENA_ENABLE;
	
	//Enable LED2_B for output
	GPIOB->DOESET31_0 |= GPIOB_LED2_B;
}

/**
 * Set, clear, or toggle LED1
 *
 * @param state determines whether to set, clear or toggle LED1
 * @return None
*/
void LED1_set(LED1State state) {
	//Set state of LED1 based on input, clear it for off, set for on, else toggle
	if (state == LED1_OFF) {
		GPIOA->DOUTCLR31_0 = GPIOA_LED1;
	}
	else if (state == LED1_ON) {
		GPIOA->DOUTSET31_0 = GPIOA_LED1;
	}
	else {
		GPIOA->DOUTTGL31_0 = GPIOA_LED1;
	}
}

/**
 * Set RGB LEDs on/off depending on input state
 *
 * @param state color to set LED2 to, or off (RGB all cleared)
 * @return None
*/
void LED2_set (LED2State state) {
	//Determine whether red is on or off
	if (state == LED2_RED 
	 || state == LED2_MAGENTA 
	 || state == LED2_YELLOW 
	 || state == LED2_WHITE) {
		GPIOB->DOUTSET31_0 = GPIOB_LED2_R;
	}
	else {
		GPIOB->DOUTCLR31_0 = GPIOB_LED2_R;
	}
	
	//Determine whether Green is on or off
	if (state == LED2_GREEN 
	 || state == LED2_CYAN 
	 || state == LED2_YELLOW 
	 || state == LED2_WHITE) {
		GPIOB->DOUTSET31_0 = GPIOB_LED2_G;
	}
	else {
		GPIOB->DOUTCLR31_0 = GPIOB_LED2_G;
	}
	
	//Determine whether Blue is on or off
	if (state == LED2_BLUE 
 	 || state == LED2_CYAN 
	 || state == LED2_MAGENTA 
	 || state == LED2_WHITE) {
		GPIOB->DOUTSET31_0 = GPIOB_LED2_B;
	}
	else {
		GPIOB->DOUTCLR31_0 = GPIOB_LED2_B;
	}
}

