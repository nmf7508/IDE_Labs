/**
 * ******************************************************************************
 * @file    timers.h
 * @brief   Timers module header file
 * @details
 *   This header defines initialization and configuration functions for the 
 *   MSPM0 general-purpose and PWM timer modules used in Lab 6.
 * 
 *   The timers provide timing control for:
 *   - Stepper motor PWM generation
 *   - ADC sampling intervals (light and temperature sensors)
 *   - Stopwatch timing functions
 *   - Camera synchronization signals (SI and CLK)
 * 
 *   Each function provides setup and control of a specific timer group or 
 *   channel, allowing flexible configuration of period, prescaler, and 
 *   duty cycle for PWM outputs.
 * 
 * @authors
 *   Nick Fair  
 *   Nathan Winiarski
 * 
 * @date 10/21/2025
 * ******************************************************************************
 */

#ifndef _TIMERS_H_
#define _TIMERS_H_

#include <stdint.h>

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/**
 * @brief Flag set when a delay period or timer-based event completes.
 * @details Used to signal completion of non-blocking delays or timing loops.
 */
extern volatile int delayOver;

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------

/**
 * @brief Initialize Timer Group 0 (TIMG0) for general-purpose use.
 * @param[in] period     Timer load value.
 * @param[in] prescaler  Timer prescaler value.
 * @note TIMG0 is in Power Domain 0 (refer to Data Sheet, p.3).
 */
void TIMG0_init(uint32_t period, uint32_t prescaler);

/**
 * @brief Initialize Timer Group 6 (TIMG6) for general-purpose use.
 * @param[in] period     Timer load value.
 * @param[in] prescaler  Timer prescaler value.
 */
void TIMG6_init(uint32_t period, uint32_t prescaler);

/**
 * @brief Initialize Timer Group 12 (TIMG12) for general-purpose use.
 * @param[in] period  Timer load value.
 * @note TIMG12 does not include a prescaler.
 */
void TIMG12_init(uint32_t period);

/**
 * @brief Initialize Timer A0 for PWM signal generation.
 * @param[in] pin               Timer PWM output pin or channel.
 * @param[in] period            Timer load value.
 * @param[in] prescaler         Timer prescaler value.
 * @param[in] percentDutyCycle  Initial PWM duty cycle (0?100%).
 * @note The period should be stored if dynamic duty-cycle updates are required.
 */
void TIMA0_PWM_init(uint8_t pin, uint32_t period, uint32_t prescaler, double percentDutyCycle);

/**
 * @brief Initialize Timer A1 for PWM signal generation.
 * @param[in] pin               Timer PWM output pin or channel.
 * @param[in] period            Timer load value.
 * @param[in] prescaler         Timer prescaler value.
 * @param[in] percentDutyCycle  Initial PWM duty cycle (0?100%).
 * @note The period should be stored if dynamic duty-cycle updates are required.
 */
void TIMA1_PWM_init(uint8_t pin, uint32_t period, uint32_t prescaler, double percentDutyCycle);

/**
 * @brief Update PWM duty cycle for Timer A0 channels.
 * @param[in] pin               Timer PWM output pin or channel.
 * @param[in] percentDutyCycle  New duty cycle percentage (0?100%).
 */
void TIMA0_PWM_DutyCycle(uint8_t pin, double percentDutyCycle);

/**
 * @brief Update PWM duty cycle for Timer A1 channels.
 * @param[in] pin               Timer PWM output pin or channel.
 * @param[in] percentDutyCycle  New duty cycle percentage (0?100%).
 */
void TIMA1_PWM_DutyCycle(uint8_t pin, double percentDutyCycle);

#endif /* _TIMERS_H_ */
