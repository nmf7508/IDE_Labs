/**
 * ******************************************************************************
 * @file    : switches.h
 * @brief   : Switches module header file
 * @details : Switches initialization and interaction
 * 
 * @author  : Nicholas Fair
 * @date    : 09/01/2025
 * ******************************************************************************
*/

#ifndef _SWITCHES_H_
#define _SWITCHES_H_

/**
 * @brief Switch 1 polling initialization
 * @hint You might want to check out the schematics in the MSP User Guide
 *       The IOMUX has a hardware inversion bit
*/
void S1_init(void);


/**
 * @brief Switch 2 polling initialization
*/
void S2_init(void);


/**
 * @brief Check if switch 1 was pressed
 * @return True(1)/False(0) if switch 1 was pressed
*/
int S1_pressed(void);


/**
 * @brief Check if switch 2 was pressed
 * @return True(1)/False(0) if switch 2 was pressed
*/
int S2_pressed(void);


#endif // _SWITCHES_H_
