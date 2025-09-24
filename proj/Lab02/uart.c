/**
 * ******************************************************************************
 * @file    : uart.c
 * @brief   : UART functions
 * @details : UART initialize, get and put functions
 * 
 * @author  : Nick Fair
 * @date    : 09/10/2004
 * ******************************************************************************
*/

#include <ti\devices\msp\msp.h>
#include "lab2/uart.h"
#include "sysctl.h"


void UART0_init() {
	//Check if power sequence has been run already, if not run reset and enable power
	if (!(UART0->GPRCM.PWREN & UART_PWREN_ENABLE_ENABLE)) {
		//Unlock RSTCTL assert reset then lock RSTCTL again
		UART0->GPRCM.RSTCTL = UART_RSTCTL_KEY_UNLOCK_W | UART_RSTCTL_RESETASSERT_ASSERT;
		UART0->GPRCM.RSTCTL = UART0->GPRCM.RSTCTL & ~UART_RSTCTL_KEY_UNLOCK_W;
		
		//Unlock PWREN assert power enable then lock PWREN again
		UART0->GPRCM.PWREN = UART_PWREN_KEY_UNLOCK_W | UART_PWREN_ENABLE_ENABLE;
		UART0->GPRCM.PWREN = UART0->GPRCM.PWREN & ~UART_PWREN_KEY_UNLOCK_W;
	}
	
	//Set peripheral connected and configure PF for UART0_TX
	IOMUX->SECCFG.PINCM[IOMUX_PINCM21] |= (0x80 | IOMUX_PINCM21_PF_UART0_TX);
	//Set peripheral connected, configure PF for UART0_RX and enable input
	IOMUX->SECCFG.PINCM[IOMUX_PINCM22] |= (0x80 | IOMUX_PINCM22_PF_UART0_RX);
	IOMUX->SECCFG.PINCM[IOMUX_PINCM22] |= IOMUX_PINCM_INENA_ENABLE;
	
	//Set Clock Source as BUS_CLK
	UART0->CLKSEL = UART_CLKSEL_BUSCLK_SEL_ENABLE;
	
	//Set Clock Div to 1
	UART0->CLKDIV = UART_CLKDIV_RATIO_DIV_BY_1;
	
	//Clear enable bit in CTL0 reg
	UART0->CTL0 &= ~UART_CTL0_ENABLE_ENABLE;
	//Explicitly set oversampling to 16x
	UART0->CTL0 &= ~UART_CTL0_HSE_MASK;
	//Set RXE, TXE, FIFOEn
	UART0->CTL0 |= UART_CTL0_TXE_ENABLE;
	UART0->CTL0 |= UART_CTL0_RXE_ENABLE;
	UART0->CTL0 |= UART_CTL0_FEN_ENABLE;
	
	//get UARTClkRate
	enum SYSCTL_SYSCLK_FREQ ULPCLKRate = SYSCTL_SYSCLK_getULPCLK();
	
	//get Baud Rate Divisor
	double BRD = (ULPCLKRate / (16 * 9600));
	
	//set Baud Rate Divisor for UART
	UART0->IBRD = (uint32_t)BRD;
	UART0->FBRD = (uint32_t)((BRD-((double)((uint32_t)BRD)))*64 + 0.5);
	
	//8 databits, 1 stop bit, no parity bits
	UART0->LCRH |= UART_LCRH_WLEN_DATABIT8;
	UART0->LCRH &= ~UART_LCRH_STP2_ENABLE;
	UART0->LCRH &= ~UART_LCRH_PEN_ENABLE;
	
	//Enable enable bit in CTL0 reg again :)
	UART0->CTL0 |= UART_CTL0_ENABLE_ENABLE;
}

void UART0_putchar(uint8_t ch) {
	while (UART0->STAT & UART_STAT_TXFF_SET) {
		__asm("nop");
	}
	UART0->TXDATA = ch;
}

char UART0_getchar() {
	while (UART0->STAT & UART_STAT_RXFE_SET) {
		__asm("nop");
	}
	return UART0->RXDATA & UART_RXDATA_DATA_MASK;
}

void UART0_put(uint8_t *ptr_str){
	int i = 0;
	while (ptr_str[i] != 0x00) {
		UART0_putchar(ptr_str[i]);
		i++;
	}
}
