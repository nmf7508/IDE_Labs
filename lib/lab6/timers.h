/**
 * ******************************************************************************
 * @file    : timers.h
 * @brief   : Timers module header file
 * @details : Timers initialization and interaction
 * 
 * @author 
 * @date 
 * ******************************************************************************
*/

#ifndef _TIMERS_H_
#define _TIMERS_H_

#include <stdint.h>

/**
 * @brief Timer G0 module initialization. General purpose timer
 * @note Timer G0 is in Power Domain 0. Check page 3 of the Data Sheet
*/
void TIMG0_init(uint32_t period, uint32_t prescaler);


/**
 * @brief Timer G6 module initialization. General purpose timer
*/
void TIMG6_init(uint32_t period, uint32_t prescaler);


/**
 * @brief Timer G12 module initialization. General purpose timer
 * @note Timer G12 has no prescaler
*/
void TIMG12_init(uint32_t period);


/**
 * @brief Timer A0 module PWM initialization
 * @param[in] pin - Timer PWM output pin / channel
 * @param[in] period - Timer load value
 * @param[in] prescaler - Timer prescale value
 * @param[in] percenetDutyCycle - PWM duty cycle positive
 * @note Store period to be able to adjust duty cycle percentage later
*/
void TIMA0_PWM_init(uint8_t pin, uint32_t period, uint32_t prescaler, double percentDutyCycle);


/**
 * @brief Timer A1 module PWM initialization
 * @param[in] pin - Timer PWM output pin / channel
 * @param[in] period - Timer load value
 * @param[in] prescaler - Timer prescale value
 * @param[in] percenetDutyCycle - PWM duty cycle positive
 * @note Store period to be able to adjust duty cycle percentage later
*/
void TIMA1_PWM_init(uint8_t pin, uint32_t period, uint32_t prescaler, double percentDutyCycle);


/**
 * @brief Change PWM duty cycle for all Timer A0 channels
 * @param[in] pin - Timer PWM output pin / channel
 * @param[in] percentDutyCycle - Duty cycle to change to
*/
void TIMA0_PWM_DutyCycle(uint8_t pin, double percentDutyCycle);


/**
 * @brief Change PWM duty cycle for all Timer A1 channels
 * @param[in] pin - Timer PWM output pin / channel
 * @param[in] percentDutyCycle - Duty cycle to change to
*/
void TIMA1_PWM_DutyCycle(uint8_t pin, double percentDutyCycle);


#endif // _TIMERS_H_
