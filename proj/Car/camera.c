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
#define EDGE_THRESH 150

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
		int left_idx = -1;
		int right_idx = -1;
		int left_val =  0;
		int right_val = 0;

		for (int i = 10; i < 118; i++) {
				int d = sensorValues[i];

				if (d < left_val) {          // most negative -> left edge
						left_val = d;
						left_idx = i;
				}
				if (d > right_val) {         // most positive -> right edge
						right_val = d;
						right_idx = i;
				}
		}
		if (abs(left_val)  < EDGE_THRESH)  left_idx  = -1;
		if (abs(right_val) < EDGE_THRESH)  right_idx = -1;
				
		double center;
		if (left_idx >= 0 && right_idx >= 0) {
				center = 0.5 * (double)(left_idx + right_idx);
		}
		else if (left_idx >= 0) {
				center = left_idx + 64.0;
		} else if (right_idx >= 0) {
				center = right_idx - 64.0;
		} else {
				// no line detected -> use last_center or drive straight/slow
				center = 64;
		}
		//last_center = center;
		double cam_center = 64;
		double error_pix = center - cam_center;
		double max_offset = 54;
		double error_norm = error_pix / max_offset;   // ~[-1,1]
		return error_norm;


}
