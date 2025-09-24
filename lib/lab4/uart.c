/****************************************************************
* uart.c
* Lab 03/04 - UART Driver Implementation
*
* Description:
*   This file provides functions to initialize and use UART0
*   and UART1 on the MSPM0G3507 LaunchPad for serial communication.
*
* Functions:
*   - UART0_init()    : Configure UART0 module, pins, and baud rate
*   - UART0_putchar() : Transmit a single character
*   - UART0_getchar() : Receive a single character
*   - UART0_put()     : Transmit a null-terminated string
*   - UART1_init()    : Configure UART1 module, pins, and baud rate
*   - UART1_putchar() : Transmit a single character
*   - UART1_getchar() : Receive a single character
*   - UART1_put()     : Transmit a null-terminated string
*
* UART0 Pin Mapping:
* +---------+----------+--------+--------+
* | Signal  | Pin Name | Module | PINCMx |
* +---------+----------+--------+--------+
* | TX      | PA10     | UART0  | 21     |
* | RX      | PA11     | UART0  | 22     |
* +---------+----------+--------+--------+
* UART1 Pin Mapping:
* +---------+----------+--------+--------+
* | Signal  | Pin Name | Module | PINCMx |
* +---------+----------+--------+--------+
* | TX      | PA8 (J14)| UART1  | 19     |
* | RX      | PA9      | UART1  | 20     |
* +---------+----------+--------+--------+
*
* Author: Nathan Winiarski
* Rochester Institute of Technology
* CMPE-460 Interface & Digital Electronics
****************************************************************/

#include "uart.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"

void UART0_init(void){
	
	if(!(UART0->GPRCM.PWREN & UART_PWREN_ENABLE_ENABLE)){
		// Reset
		UART0->GPRCM.RSTCTL = UART_RSTCTL_KEY_UNLOCK_W | UART_RSTCTL_RESETSTKYCLR_CLR;
		
		// Power on
		UART0->GPRCM.PWREN = UART_PWREN_KEY_UNLOCK_W | UART_PWREN_ENABLE_ENABLE;
	}
	// Select PINCM22 for UART RX
	IOMUX->SECCFG.PINCM[IOMUX_PINCM22] |= (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM22_PF_UART0_RX);
	
	// Set PINCM22 to input mode (RX ONLY)
	IOMUX->SECCFG.PINCM[IOMUX_PINCM22] |= IOMUX_PINCM_INENA_ENABLE;
	
	// Select PINCM21 for UART TX
	IOMUX->SECCFG.PINCM[IOMUX_PINCM21] |= (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM21_PF_UART0_TX);
	
	// Set the busclock for the UART
	UART0->CLKSEL |= UART_CLKSEL_BUSCLK_SEL_ENABLE;
	
	// Set clock div to divide by 1
	UART0->CLKDIV |= UART_CLKDIV2_RATIO_DIV_BY_1;
	
	// Clear enable bit of CTL0
	UART0->CTL0 &= ~UART_CTL0_ENABLE_ENABLE;
	
	// Set the oversampling rate to 16x
	UART0->CTL0 &= ~UART_CTL0_HSE_MASK;
	
	// Enable transmit
	UART0->CTL0 |= UART_CTL0_TXE_ENABLE;
	
	// Enable receive
	UART0->CTL0 |= UART_CTL0_RXE_ENABLE;

	// Enable FIFO
	UART0->CTL0 |= UART_CTL0_FEN_ENABLE;
	
	// Get the current UARTclk
	enum SYSCTL_SYSCLK_FREQ clock_speed = SYSCTL_SYSCLK_getULPCLK();
	
	// calculate baud rate
	double baud_rate = (uint32_t)clock_speed / (16.0 * 9600);
	UART0->IBRD = (uint32_t)baud_rate;
	UART0->FBRD = (uint32_t)((baud_rate - (uint32_t)baud_rate) * 64 + 0.5);
	
	// set the line control register, 8 bits, 1 stop bit, no parity
	UART0->LCRH |= UART_LCRH_WLEN_DATABIT8 | UART_LCRH_STP2_DISABLE | UART_LCRH_PEN_DISABLE;
	
	// set enable bit of CTL0
	UART0->CTL0 |= UART_CTL0_ENABLE_ENABLE;
	
}

void UART0_putchar(uint8_t ch){
	// Check to see if the queue has anything in it
	while(UART0->STAT & UART_STAT_TXFF_SET){
		__asm("nop");
	}
	// If not, set the character in the TXDATA queue
	UART0->TXDATA = ch;
}

char UART0_getchar(void){
	// Check to see if the queue has anything in it
	while (UART0->STAT & UART_STAT_RXFE_SET) {
			__asm("nop");
	}
	// If so -- return it
	return (UART0->RXDATA & UART_RXDATA_DATA_MASK);
}

void UART0_put(uint8_t *ptr_str) {
		// perform a putchar on all of the charaters in a null-terminated string
    while(*ptr_str != '\0') {
        UART0_putchar(*ptr_str++);
    }
}


// -----------------------------------------------------UART1---------------------------------------------------------------------------------
void UART1_init(void){
	
	if(!(UART1->GPRCM.PWREN & UART_PWREN_ENABLE_ENABLE)){
		// Reset
		UART1->GPRCM.RSTCTL = UART_RSTCTL_KEY_UNLOCK_W | UART_RSTCTL_RESETSTKYCLR_CLR;
		
		// Power on
		UART1->GPRCM.PWREN = UART_PWREN_KEY_UNLOCK_W | UART_PWREN_ENABLE_ENABLE;
	}
	// Select PINCM22 for UART RX
	IOMUX->SECCFG.PINCM[IOMUX_PINCM20] |= (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM22_PF_UART0_RX);
	
	// Set PINCM22 to input mode (RX ONLY)
	IOMUX->SECCFG.PINCM[IOMUX_PINCM20] |= IOMUX_PINCM_INENA_ENABLE;
	
	// Select PINCM21 for UART TX
	IOMUX->SECCFG.PINCM[IOMUX_PINCM19] |= (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM21_PF_UART0_TX);
	
	// Set the busclock for the UART
	UART1->CLKSEL |= UART_CLKSEL_BUSCLK_SEL_ENABLE;
	
	// Set clock div to divide by 1
	UART1->CLKDIV |= UART_CLKDIV2_RATIO_DIV_BY_1;
	
	// Clear enable bit of CTL0
	UART1->CTL0 &= ~UART_CTL0_ENABLE_ENABLE;
	
	// Set the oversampling rate to 16x
	UART1->CTL0 &= ~UART_CTL0_HSE_MASK;
	
	// Enable transmit
	UART1->CTL0 |= UART_CTL0_TXE_ENABLE;
	
	// Enable receive
	UART1->CTL0 |= UART_CTL0_RXE_ENABLE;

	// Enable FIFO
	UART1->CTL0 |= UART_CTL0_FEN_ENABLE;
	
	// Get the current UARTclk
	enum SYSCTL_SYSCLK_FREQ clock_speed = SYSCTL_SYSCLK_getULPCLK();
	
	// calculate baud rate
	double baud_rate = (uint32_t)clock_speed / (16.0 * 9600);
	UART1->IBRD = (uint32_t)baud_rate;
	UART1->FBRD = (uint32_t)((baud_rate - (uint32_t)baud_rate) * 64 + 0.5);
	
	// set the line control register, 8 bits, 1 stop bit, no parity
	UART1->LCRH |= UART_LCRH_WLEN_DATABIT8 | UART_LCRH_STP2_DISABLE | UART_LCRH_PEN_DISABLE;
	
	// set enable bit of CTL0
	UART1->CTL0 |= UART_CTL0_ENABLE_ENABLE;
	
}

void UART1_putchar(char ch){
	// Check to see if the queue has anything in it
	while(UART1->STAT & UART_STAT_TXFF_SET){
		__asm("nop");
	}
	// If not, set the character in the TXDATA queue
	UART1->TXDATA = ch;
}

char UART1_getchar(void){
	// Check to see if the queue has anything in it
	while (UART1->STAT & UART_STAT_RXFE_SET) {
			__asm("nop");
	}
	// If so -- return it
	return (UART1->RXDATA & UART_RXDATA_DATA_MASK);
}

void UART1_put(char *ptr_str) {
		// perform a putchar on all of the charaters in a null-terminated string
    while(*ptr_str != '\0') {
        UART1_putchar(*ptr_str++);
    }
		UART1_putchar('\n');
		UART1_putchar('\r');
}

