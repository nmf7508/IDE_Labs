// camera.c - OUTERMOST EDGE DETECTION (Robust Version)

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
#define THRESHOLD_NOISE 100 

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
double LineSensor_Calculate_Error(int16_t *diff_data)
{
    int first_left_idx = -1;
    int last_right_idx = -1;

    // 1. Scan Left->Right for FIRST Rising Edge (Left Wall)
    // Start at 20 to ignore hardware startup noise (Pedestal)
    for (int i = 20; i < 120; i++) {
        if (diff_data[i] > THRESHOLD_NOISE) {
            first_left_idx = i;
            break; // Found outermost wall, stop looking
        }
    }

    // 2. Scan Right->Left for LAST Falling Edge (Right Wall)
    // Stop at 20 to ignore hardware noise
    for (int i = 123; i > 20; i--) {
        if (diff_data[i] < -THRESHOLD_NOISE) {
            last_right_idx = i;
            break; // Found outermost wall, stop looking
        }
    }

    // --- CROSSED EYES SAFETY CHECK ---
    if (first_left_idx != -1 && last_right_idx != -1) {
        if (first_left_idx > last_right_idx) {
            // Impossible scenario. Trust the edge closer to the screen border.
            if (first_left_idx > 64) last_right_idx = -1;
            else first_left_idx = -1;
        }
    }

    // --- ERROR CALCULATION ---

    // CASE 1: Perfect Track
    if (first_left_idx != -1 && last_right_idx != -1) {
        double center = (double)(first_left_idx + last_right_idx) / 2.0;
        return (center - 64.0) / 64.0;
    }

    // CASE 2: Only Left Wall Visible (Steer Right)
    if (first_left_idx != -1 && last_right_idx == -1) {
        return 1.0; // Hard Right
    }

    // CASE 3: Only Right Wall Visible (Steer Left)
    if (first_left_idx == -1 && last_right_idx != -1) {
        return -1.0; // Hard Left
    }

    // CASE 4: No Walls (Intersection or Lost)
    return -99.0;
}
