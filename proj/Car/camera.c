#include "camera.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"
#include "timers.h"
#include "adc12.h"
#include "uart.h"
#include "isrs.h"
#include "uart_extras.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

// Threshold to distinguish line from floor (Adjust 70-120 based on lighting)
#define THRESHOLD_NOISE 0 

void init_CLK(void) {
    TIMG0_init(320, 0); 
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }
    IOMUX->SECCFG.PINCM[IOMUX_PINCM34] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM34] &= ~IOMUX_PINCM_INENA_ENABLE;
    GPIOA->DOESET31_0 |= (1 << 12);
}

void init_SI(void) {
    // 6000 = 6ms integration time (approx 166 FPS)
    TIMG6_init(6000, 3); 
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] &= ~IOMUX_PINCM_INENA_ENABLE;
    TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
    GPIOA->DOESET31_0 |= (1 << 28);
}

void Camera_init(void) {
    init_CLK();
    init_SI();
    cameraData_complete = 0; 
    pixelCounter = 0;        
}

uint8_t Camera_isDataReady(void) {
    return cameraData_complete;
}

uint16_t* Camera_getData(void) {
    cameraData_complete = 0;   
    return (uint16_t*)cameraData;
}

/**
 * @brief Finds the OUTERMOST walls to handle Intersections & Ignore Noise
 */
double LineSensor_Calculate_Error(int16_t *sensorValues)
{
		int left = -1, right = -1;

    // --- 1. Get min/max ---
    for (int i = 4; i < 128; i++) {
			if (sensorValues[i] > 100) {
				left = i;
			}
			if (sensorValues[131-i] < -100) {
				right = i;
			}
		}
		if (left == -1 && right == -1) {
			return 0;
		}
		if (left == -1) {
			return ((double)(right-128) / (double) 25);
		}
		if (right == -1) {
			return ((double)(left) / (double) 25);
		}
		return (double) (64 - (right + left)/2) / (double) 20;
		/*UART1_printDec(left);
		UART1_put(", ");
		UART1_printDec(right);
		UART1_put("\r\n");*
    */
	  return 0;
}
