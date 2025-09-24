/**
 * ******************************************************************************
 * @file    : leds.h
 * @brief   : LEDs module header file
 * @details : LED initialization and interaction
 * 
 * @author Nicholas Fair
 * @date   09/01/2025
 * ******************************************************************************
*/

#ifndef _LEDS_H_
#define _LEDS_H_

//Bit in GPIO DOE/DOUT regs for each LED
#define GPIOA_LED1 		(1 << 0)
#define GPIOB_LED2_R	(1 << 26)
#define GPIOB_LED2_G  (1 << 27)
#define GPIOB_LED2_B  (1 << 22)

//Type definition for all possible LED1 states
typedef enum {
	LED1_OFF,
	LED1_ON,
	LED1_TOGGLE
} LED1State;

//Type definition for all used LED2 states
typedef enum {
	LED2_OFF,
	LED2_RED,
	LED2_GREEN,
	LED2_BLUE,
	LED2_CYAN,
	LED2_MAGENTA,
	LED2_YELLOW,
	LED2_WHITE
} LED2State;

/**
 * @brief Initialze LED1
 * @hint You might want to check out the schematics in the MSP User Guide
 *       The IOMUX has a hardware inversion bit
*/
void LED1_init(void);


/**
 * @brief Initialize LED2
 * @note You must account for each LED color
*/
void LED2_init(void);


/**
 * @brief Set LED1 output state
 * @note ON, OFF, TOGGLE
*/
void LED1_set(LED1State state);


/**
 * @brief Set LED2 output state
 * @note RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, WHITE, OFF
*/
void LED2_set(LED2State state);


#endif // _LEDS_H_
