/**
 * ******************************************************************************
 * @file    : uart.h
 * @brief   : UART module header file
 * @details : UART initialization and interaction
 * 
 * @author 
 * @date 
 * ******************************************************************************
*/

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

/**
 * @brief Initialize UART0
*/
void UART0_init(void);


/**
 * @brief Put a character over UART0
 * @param[in] ch - Character to print
*/
void UART0_putchar(uint8_t ch);


/**
 * @brief Retrieve a single character from UART0
*/
char UART0_getchar(void);


/**
 * @brief Send a full character string over UART0
 * @param[in] ptr_str - Pointer to the string to print
*/
void UART0_put(uint8_t *ptr_str);


#endif // _UART_H_
