/**
 * ******************************************************************************
 * @file    : switches.c
 * @brief   : Switches module
 * @details : Switches initialization and interaction
 * 
 * @author  : Nicholas Fair
 * @date    : 09/01/2025
 * ******************************************************************************
*/

#include <ti\devices\msp\msp.h>
#include "lab5\switches.h"

/**
 * Reset and initialize power for switch 1 if not done already
 *
 * @return None
*/
void S1_init(void) {
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
	IOMUX->SECCFG.PINCM[IOMUX_PINCM40] |= (0x80 | 0x01);
	//Enable input and pull down resistor
	IOMUX->SECCFG.PINCM[IOMUX_PINCM40] |= IOMUX_PINCM_INENA_ENABLE;
	IOMUX->SECCFG.PINCM[IOMUX_PINCM40] |= IOMUX_PINCM_PIPD_ENABLE;
}

/**
 * Reset and initialize power for switch 2 if not done already
 *
 * @return None
*/
void S2_init(void) {
	//Check if power sequence has been run already, if not run reset and enable power
	if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
		//Unlock RSTCTL assert reset then lock RSTCTL again
		GPIOB->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
		GPIOB->GPRCM.RSTCTL = GPIOB->GPRCM.RSTCTL & ~GPIO_RSTCTL_KEY_UNLOCK_W;
		
		//Unlock PWREN assert power enable then lock PWREN again
		GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
		GPIOB->GPRCM.PWREN = GPIOB->GPRCM.PWREN & ~GPIO_PWREN_KEY_UNLOCK_W;
	}
	
	//Set peripheral connected and configure PF for GPIO
	IOMUX->SECCFG.PINCM[IOMUX_PINCM49] |= (0x80 | 0x01);
	//Enable input, input inversion, and pull up resistor
	IOMUX->SECCFG.PINCM[IOMUX_PINCM49] |= IOMUX_PINCM_INENA_ENABLE;
	IOMUX->SECCFG.PINCM[IOMUX_PINCM49] |= IOMUX_PINCM_INV_ENABLE;
	IOMUX->SECCFG.PINCM[IOMUX_PINCM49] |= IOMUX_PINCM_PIPU_ENABLE;
}

/**
 * Determine whether switch 1 is currently pressed
 *
 * @return 1 if pressed, else 0
*/
int S1_pressed(void) {
	//If button is pressed 19th bit of DIN31_0 will be set
	return GPIOA->DIN31_0 & (1 << 18);
}

/**
 * Determine whether switch 2 is currently pressed
 *
 * @return 1 if pressed, else 0
*/
int S2_pressed(void) {
	//If button is pressed 22nd bit of DIN31_0 will be set
	return GPIOB->DIN31_0 & (1 << 21);
}

/**
 * @brief Switch 1 interrupt initialization
 * @note Use NVIC_EnableIRQ() to register IRQn with the NVIC
 *       Check out `cmsis_armclang.h`
 * @hint Keep the polarity in mind
*/
void S1_init_interrupt(void){
	__disable_irq();
	S1_init();
}


/**
 * @brief Switch 2 interrupt initialization
 * @note Use NVIC_EnableIRQ() to register IRQn with the NVIC
 *       Check out `cmsis_armclang.h`
*/
void S2_init_interrupt(void){
	__disable_irq();
	S2_init();
}
