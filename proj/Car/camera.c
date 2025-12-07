/**
 * @file    camera.c
 * @brief   Camera Sensor and PID Implementation
 * @details Implements the Linescan camera timing logic and the 
 * weighted average algorithm for line detection.
 *
 * @author  Nick Fair
 * @author  Nathan Winiarski
 * @date    Fall 2025
 */

#include "camera.h"
#include <ti/devices/msp/msp.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

/* Local Drivers */
#include "sysctl.h"
#include "timers.h"
#include "adc12.h"
#include "uart.h"
#include "isrs.h"
#include "uart_extras.h"

/* --- Hardware Configuration Macros --- */
#define CAM_CLK_PIN_MASK    (1 << 12)   // PA12
#define CAM_SI_PIN_MASK     (1 << 28)   // PA28

/* --- Image Processing Constants --- */
#define IGNORE_EDGE_PIXELS  10      // Number of pixels to ignore on left/right
#define THRESH_CARPET       800     // Min avg intensity (Black/Carpet)
#define THRESH_DISCONNECT   3500    // Max avg intensity (Glare/Disconnect)
#define CAM_CENTER_INDEX    63.0    // Optical center of the array

/* --- PID Tuning Constants --- */
#define KP_GAIN             0.75
#define KI_GAIN             3.5
#define KD_GAIN             0.5
#define PID_SAMPLE_TIME     0.010
#define INTEGRAL_LIMIT      1.0
#define OUTPUT_LIMIT        1.0

/* --- Private Globals (PID State) --- */
static double integral   = 0.0;
static double prev_error = 0.0;

/* --- Private Helper Functions --- */

/**
 * @brief Configures Timer G0 for Camera Clock (CLK) generation.
 * @note  Pin: PA12 (IOMUX_PINCM34)
 */
static void init_CLK(void) {
    TIMG0_init(320, 0); 
    
    // Power up GPIOA
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    // Configure PA12 as Output
    IOMUX->SECCFG.PINCM[IOMUX_PINCM34] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM34] &= ~IOMUX_PINCM_INENA_ENABLE;
    
    GPIOA->DOESET31_0 |= CAM_CLK_PIN_MASK;
}

/**
 * @brief Configures Timer G6 for Camera Start Integration (SI) pulse.
 * @note  Pin: PA28 (IOMUX_PINCM3)
 */
static void init_SI(void) {
    TIMG6_init(10000, 3); 

    // Power up GPIOA
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    // Configure PA28 as Output
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] &= ~IOMUX_PINCM_INENA_ENABLE;
    
    // Enable Timer and set Output Enable
    TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
    GPIOA->DOESET31_0 |= CAM_SI_PIN_MASK;
}

/* --- Function Implementations --- */

void Camera_init(void) {
    init_CLK();
    init_SI();
    
    // Reset Globals defined in isrs.h/c
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

double LineSensor_Calculate_Error(uint16_t *sensorValues) {
    double w_sum = 0.0;
    double xw_sum = 0.0;
    
    // Loop through active pixels
    // Range: 10 to 117
    for (int i = IGNORE_EDGE_PIXELS; i < (128 - IGNORE_EDGE_PIXELS); i++) {
        double w = sensorValues[i];
        w_sum  += w;
        xw_sum += w * i;
    }

    double average = w_sum / 108.0;

    // Check for Carpet
    if (average < THRESH_CARPET) {
        return LINE_NOT_FOUND;
    }
    // Check for Disconnect
    if (average > THRESH_DISCONNECT) {
        return LINE_LOST_HISTORY;
    }

    // Calculate Center of Mass
    double center;
    if (w_sum > 0.0) {
        center = xw_sum / w_sum;
    } else {
        center = CAM_CENTER_INDEX;
    }
    
    double error_pix  = CAM_CENTER_INDEX - center;
    double error_norm = error_pix / 3.0;
    
    return error_norm;
}

double PID_Update(double error_norm) {
    // Proportional
    double P = KP_GAIN * error_norm;

    // Integral
    integral += error_norm * PID_SAMPLE_TIME;
    
    // Clamping
    if (integral >  INTEGRAL_LIMIT) integral =  INTEGRAL_LIMIT;
    if (integral < -INTEGRAL_LIMIT) integral = -INTEGRAL_LIMIT;
    
    double I = KI_GAIN * integral;

    // Derivative
    double derivative = (error_norm - prev_error) / PID_SAMPLE_TIME;
    double D = KD_GAIN * derivative;
    
    prev_error = error_norm;

    // Sum terms
    double u = P + I + D;

    // Output clamping
    if (u >  OUTPUT_LIMIT) u =  OUTPUT_LIMIT;
    if (u < -OUTPUT_LIMIT) u = -OUTPUT_LIMIT;

    return u;
}
