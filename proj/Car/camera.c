	/**
	 * ******************************************************************************
	 * @file    : camera.c
	 * @brief   : Camera Module Interface for MSP Microcontroller
	 * @details :
	 *   This file implements initialization and control functions for a simple 
	 *   line-scan camera system using the MSP microcontroller. The system uses 
	 *   two GPIO outputs to generate timing signals:
	 *   
	 *   - CLK (PA12): Provides the pixel clock that triggers ADC sampling for each pixel.
	 *   - SI (PA28): Acts as the "Start Integration" signal, beginning a new image capture.
	 *   
	 *   Two general-purpose timers (TIMG0 and TIMG6) are configured to generate 
	 *   these signals. The ADC12 peripheral is used to sample pixel data in sync 
	 *   with the CLK signal, and results are stored in a 128-pixel buffer.
	 *   
	 *   This module initializes, synchronizes, and provides access to camera data, 
	 *   while the corresponding ISRs (in `isrs.c`) handle the timing and data capture.
	 * 
	 * @authors 
	 *   Nick Fair  
	 *   Nathan Winiarski
	 * 
	 * @date   10/07/2025
	 * ******************************************************************************
	 */

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

#define CENTER_DEADBAND_PIXELS  3    // |error| <= 3 pixels = "centered"
#define NUM_PIXELS 128
#define MINIMUM_LINE_RANGE 2000   // tune for your environment

// You already have this somewhere, just keep using it:

	/**
	 * @brief Initialize the pixel clock (CLK) output on PA12.
	 * @details
	 *   Configures TIMG0 to generate the CLK signal that controls pixel timing 
	 *   for the camera. The clock frequency determines how fast pixels are sampled.
	 *   The corresponding GPIO pin (PA12 / PINCM34) is configured as an output.
	 *
	 *   - TIMG0: Acts as the clock source (~100 kHz typical)
	 *   - PA12: CLK output pin
	 */
	void init_CLK(void) {
			// Initialize Timer0 with period and prescaler values for ~100 kHz pixel clock
			TIMG0_init(320, 0);

			// Enable and configure GPIOA peripheral if not already active
			if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
					GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
					GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
					GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
					GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
			}

			// Configure PA12 as output (no input, no inversion)
			IOMUX->SECCFG.PINCM[IOMUX_PINCM34] |= (0x80 | 0x01);
			IOMUX->SECCFG.PINCM[IOMUX_PINCM34] &= ~IOMUX_PINCM_INENA_ENABLE;

			// Enable PA12 data output
			GPIOA->DOESET31_0 |= (1 << 12);
	}


	/**
	 * @brief Initialize the Start Integration (SI) signal on PA28.
	 * @details
	 *   Configures TIMG6 to control integration timing the time between 
	 *   successive image captures. Each SI pulse begins a new line capture by 
	 *   resetting the pixel counter and enabling the CLK signal.
	 *
	 *   - TIMG6: Controls integration period (frame timing)
	 *   - PA28: SI output pin
	 */
	void init_SI(void) {
			// Initialize Timer6 with a period and duty cycle for integration timing
			TIMG6_init(60000, 3);

			// Enable and configure GPIOA peripheral if not already active
			if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
					GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
					GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
					GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
					GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
			}

			// Configure PA28 as output (no input, no inversion)
			IOMUX->SECCFG.PINCM[IOMUX_PINCM3] |= (0x80 | 0x01);
			IOMUX->SECCFG.PINCM[IOMUX_PINCM3] &= ~IOMUX_PINCM_INENA_ENABLE;

			// Enable Timer6 (starts integration timing) and PA28 output
			TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
			GPIOA->DOESET31_0 |= (1 << 28);
	}


	/**
	 * @brief Initialize camera system (timers, GPIO, and state variables).
	 * @details
	 *   This function calls `init_CLK()` and `init_SI()` to set up both control signals,
	 *   then ensures the CLK timer (TIMG0) is disabled until the first SI pulse starts 
	 *   a frame. It also resets data flags and counters used in capture routines.
	 */
	void Camera_init(void) {
			init_CLK();
			init_SI();
			cameraData_complete = 0; // Reset data-ready flag
			pixelCounter = 0;        // Reset pixel index counter
	}


	/**
	 * @brief Check if new camera data is available.
	 * @return 1 if the current frame line is complete, 0 otherwise.
	 */
	uint8_t Camera_isDataReady(void) {
			return cameraData_complete;
	}


	/**
	 * @brief Retrieve pointer to captured camera data.
	 * @details 
	 *   Provides access to the 128-sample pixel buffer after a full line has been 
	 *   captured. The data-ready flag is automatically cleared once accessed.
	 *
	 * @return Pointer to `uint16_t` array containing 128 pixel values.
	 */
	uint16_t* Camera_getData(void) {
			cameraData_complete = 0;   // Clear ready flag after retrieval
			return (uint16_t*)cameraData;
	}
/*
int32_t LineSensor_Calculate_Error(uint16_t* sensorValues) {
	
	// --- 1. Find Min/Max and Calculate Dynamic Threshold ---
	uint16_t min_val = 4095;
	uint16_t max_val = 0;
	
	for (int i = 0; i < 128; i++) {
			if (sensorValues[i] < min_val) min_val = sensorValues[i];
			if (sensorValues[i] > max_val) max_val = sensorValues[i];
	}

	// Check if we even see a line (is there enough contrast?)
	if ((max_val - min_val) < MINIMUM_LINE_RANGE) {
			return 0; // We are "lost" or on a uniform surface, go straight.
	}

	// Calculate the dynamic threshold
	uint16_t dynamic_threshold = (min_val + max_val) / 2;
	int left = -1;
	int right = -1;
	// --- 2. Calculate Weighted Average using the new threshold ---
	for (int i = 0; i < 128; i++) {
			uint16_t val = sensorValues[i];
			bool hey = false;
			// Use the new dynamic threshold
			if (val > dynamic_threshold) {
					if (left == -1) {
						left = i;
					}
					hey = true;
			}
			else {
				if (hey) {
					right = i;
				}
				hey = false;
			}
	}
	
	if ((left - (128-right)) < 5 && (left - (128-right)) > -5) {
		return 0;
	}

	return (left > (128-right)) ? -1:1;
}
*/
double LineSensor_Calculate_Error(uint16_t *sensorValues)
{
    uint32_t left_sum  = 0;
    uint32_t right_sum = 0;

    // Left half: pixels 0–63
    for (int i = 0; i < 64; i++) {
        left_sum += sensorValues[i];
    }

    // Right half: pixels 64–127
    for (int i = 64; i < 128; i++) {
        right_sum += sensorValues[i];
    }

    // Avoid division by zero
    uint32_t total = left_sum + right_sum;
    if (total == 0) {
        return 0.0; // No signal ? go straight
    }

    // Normalize to [-1, 1]
    // left big  -> +1
    // right big -> -1
    double error = ((double)left_sum - (double)right_sum) / ((double)total/12);

    // Safety clamp
    if (error > 1.0)  error = 1.0;
    if (error < -1.0) error = -1.0;

    return error;
}

/*
int32_t LineSensor_Calculate_Error(uint16_t *sensorValues)
{
    // --- 1. Find Min/Max to get dynamic threshold ---
    uint16_t min_val = 4095;
    uint16_t max_val = 0;

    for (int i = 0; i < NUM_PIXELS; i++) {
        uint16_t v = sensorValues[i];
        if (v < min_val) min_val = v;
        if (v > max_val) max_val = v;
    }

    // If there isn't enough contrast, assume we lost the line ? "go straight"
    if ((uint16_t)(max_val - min_val) < MINIMUM_LINE_RANGE) {
        return 0;
    }

    // Dynamic threshold: anything above this is considered "line"
    uint16_t threshold = (uint16_t)((min_val + max_val) / 2);

    // --- 2. Compute center of the bright region (weighted average) ---
    int64_t weighted_sum = 0;   // sum(i * value)
    int64_t weight_total = 0;   // sum(value)

    for (int i = 0; i < NUM_PIXELS; i++) {
        uint16_t v = sensorValues[i];

        // Only use pixels that are "on the line"
        if (v > threshold) {
            weighted_sum  += (int64_t)i * (int64_t)v;
            weight_total  += (int64_t)v;
        }
    }

    // If nothing passed the threshold, fall back to "lost"
    if (weight_total == 0) {
        return 0;
    }

    // center * 2  (do it in integer math to avoid float)
    // center_times_2 = 2 * weighted_sum / weight_total
    int32_t center_times_2 = (int32_t)((2 * weighted_sum) / weight_total);

    // Middle of sensor array is (NUM_PIXELS - 1) / 2,
    // so middle*2 = (NUM_PIXELS - 1)
    int32_t mid_times_2 = (NUM_PIXELS - 1);

    // Error in "half-pixel" units; sign only matters to us
    int32_t error_times_2 = center_times_2 - mid_times_2;

    // Convert deadband to the same units (half-pixels)
    int32_t deadband_times_2 = CENTER_DEADBAND_PIXELS * 2;

    // --- 3. Classify: left / center / right ---
    if (error_times_2 > deadband_times_2) {
        // Line is to the LEFT side of the array (index < mid)
        // ? robot is too far right, needs to steer left ? return +1
        return 1;
    } else if (error_times_2 < -deadband_times_2) {
        // Line is to the RIGHT side of the array
        // ? robot is too far left, needs to steer right ? return -1
        return -1;
    } else {
        // Close enough to the center
        return 0;
    }
}
*/

