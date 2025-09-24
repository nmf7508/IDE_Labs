/**
 * ******************************************************************************
 * @file    : i2c.h
 * @brief   : I2C module header file
 * @details : I2C initialization and interaction
 * @note    : TI documentation switches terminology around, code is behind
 *             Controller <-> Master
 *             Target     <-> Slave
 *             This changes register access from what the documentation specifies
 *
 * @author 
 * @date 
 * ******************************************************************************
*/

#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

/**
 * @brief Initialize I2C1
 * @param[in] targetAddress - Passed by OLED file. I2C target/listener address
 * @note OLED file looks for this exact prototype
*/
void I2C1_init(uint16_t targetAddress);


/**
 * @brief Sends a single character byte over I2C1
 * @param[in] ch - Byte to send
*/
void I2C1_putchar(unsigned char ch);


/**
 * @brief Send full character string over I2C until limit is reached
 * @param[in] data - String pointer to data to send
 * @param[in] data_size - Amount of bytes to transmit
*/
void I2C1_put(unsigned char *data, uint16_t data_size);


#endif // _I2C_H_
