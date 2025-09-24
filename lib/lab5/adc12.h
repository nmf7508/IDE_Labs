/**
 * ******************************************************************************
 * @file    : adc12.h
 * @brief   : ADC module header file
 * @details : ADC initialization and interaction
 * @note    : ADC does not require IOMUX interaction
 * 
 * @author 
 * @date 
 * ******************************************************************************
*/

#ifndef _ADC12_H_
#define _ADC12_H_

#include <stdint.h>

/**
 * @brief Initialize ADC0
*/
void ADC0_init(void);


/**
 * @brief Retrieve a the value from the ADC0
 * @note The ADC channel in use is set during initialization
 *       The channel is not the same as where the module stores the value
 * @return ADC0 processed value
*/
uint32_t ADC0_getVal(void);


#endif // _ADC12_H_
