/**
 * ******************************************************************************
 * @file    : sysctl.h
 * @brief   : SYSCTL module header file
 * @details : Header instantiation for SYSCTL functions
 * 
 * @author tlk1160
 * @date 3/15/2025
 * ******************************************************************************
*/

#ifndef _SYSCTL_H_
#define _SYSCTL_H_

/* Enum provided for SYSCLK frequencies */
// These values were purposefully selected
enum SYSCTL_SYSCLK_FREQ {
    SYSCLK_02MHZ = 2000000,
    SYSCLK_04MHZ = 4000000,
    SYSCLK_08MHZ = 8000000,
    SYSCLK_16MHZ = 16000000,
    SYSCLK_32MHZ = 32000000,
    SYSCLK_48MHZ = 48000000,
    SYSCLK_64MHZ = 64000000,
    SYSCLK_80MHZ = 80000000
};

void SYSCTL_SYSCLK_set(enum SYSCTL_SYSCLK_FREQ clk_freq);
enum SYSCTL_SYSCLK_FREQ SYSCTL_SYSCLK_getMCLK(void);
enum SYSCTL_SYSCLK_FREQ SYSCTL_SYSCLK_getULPCLK(void);

#endif // _SYSCTL_H_
