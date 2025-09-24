/**
 * ******************************************************************************
 * @file    : uart.h
 * @brief   : UART module header file
 * @details : UART initialization and interaction
 * @note    : UART1 requires SW1 header to be moved to connect PA9 for bluetooth
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


/**
 * @brief Initialize UART1
*/
void UART1_init(void);


/**
 * @brief Put a character over UART1
 * @param[in] ch - Character to print
*/
void UART1_putchar(char ch);


/**
 * @brief Retrieve a single character from UART1
*/
char UART1_getchar(void);


/**
 * @brief Send a full character string over UART1
 * @param[in] ptr_str - Pointer to the string to print
*/
void UART1_put(char *ptr_str);


/**
 * @brief Check if data is available over UART1. Useful for Bluetooth
*/
int UART1_isDataAvailable(void);


#endif // _UART_H_
