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

#define MARGIN 10;
 
static double integral   = 0.0;
static double prev_error = 0.0;
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
double LineSensor_Calculate_Error(uint16_t *sensorValues)
{
	double w_sum = 0.0;
	double xw_sum = 0.0;

	for (int i = 10; i < 118; i++) {
			double w = sensorValues[i];         // or (max - I[i]) if line is dark on bright floor
			w_sum  += w;
			xw_sum += w * i;
	}

	double center;
	double average = w_sum / 108.0;
	/*
	UART1_printDec((int)average);
	UART1_put("\r\n");
	*/
	if (average < 600) {
		// carpet stopping
		return -10000;
	}
	if (average > 3000) {
		// camera disconnecting
		return -60000;
	}

	if (w_sum > 0.0)
			center = xw_sum / w_sum;    // “center of mass” in pixel units
	else
			center = 64.3;               // fallback
	double error_pix  = 64.3 - center;
	//double max_off    = 54.0;
	double error_norm = error_pix/3;      // ˜ [-1,1]
	//if (error_norm < .30 && error_norm > -.30) {
	//	return 0;
	//}
	return error_norm;
}


///**
// * @brief Finds the OUTERMOST walls to handle Intersections & Ignore Noise
// */
//double LineSensor_Calculate_Error(int16_t *sensorValues)
//{
//		int left_idx = -1;
//		int right_idx = -1;
//		int left_val =  0;
//		int right_val = 0;

//		for (int i = 10; i < 118; i++) {
//				int d = sensorValues[i];

//				if (d < left_val) {          // most negative -> left edge
//						left_val = d;
//						left_idx = i;
//				}
//				if (d > right_val) {         // most positive -> right edge
//						right_val = d;
//						right_idx = i;
//				}
//		}
//		if (abs(left_val)  < EDGE_THRESH)  left_idx  = -1;
//		if (abs(right_val) < EDGE_THRESH)  right_idx = -1;
//				
//		double center;
//		if (left_idx >= 0 && right_idx >= 0) {
//				center = 0.5 * (double)(left_idx + right_idx);
//		}
//		else if (left_idx >= 0) {
//				center = left_idx + 64.0;
//		} else if (right_idx >= 0) {
//				center = right_idx - 64.0;
//		} else {
//				// no line detected -> use last_center or drive straight/slow
//				center = prev_error;
//		}
//		//last_center = center;
//		double cam_center = 64;
//		double error_pix = center - cam_center;
//		double max_offset = 54;
//		double error_norm = error_pix / max_offset;   // ~[-1,1]
//		prev_error = error_norm;
//		return error_norm;


//}

// Call this once per camera frame
double PID_Update(double error_norm)
{
    // --- Tuning gains ---
    const double Kp = .75;   // proportional
    const double Ki = 0.03;  // integral
    const double Kd = 0.25;  // derivative

    // if your loop runs e.g. 100 Hz => Ts = 0.01
    const double Ts = 0.006;  // seconds per update (adjust to your loop)

    // --- Persistent state ---

    // --- Proportional ---
    double P = Kp * error_norm;

    // --- Integral (with simple anti-windup clamp) ---
    integral += error_norm * Ts;
    const double INTEGRAL_MAX = 1.0;   // prevent runaway
    if (integral >  INTEGRAL_MAX) integral =  INTEGRAL_MAX;
    if (integral < -INTEGRAL_MAX) integral = -INTEGRAL_MAX;
    double I = Ki * integral;

    // --- Derivative ---
    double derivative = (error_norm - prev_error) / Ts;
    double D = Kd * derivative;
    prev_error = error_norm;

    // --- Sum terms ---
    double u = P + I + D;

    // --- Limit output to [-1, 1] for your servo ---
    if (u >  1.0) u =  1.0;
    if (u < -1.0) u = -1.0;

    return u;
}

