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
	 *   Configures TIMG6 to control integration timing ? the time between 
	 *   successive image captures. Each SI pulse begins a new line capture by 
	 *   resetting the pixel counter and enabling the CLK signal.
	 *
	 *   - TIMG6: Controls integration period (frame timing)
	 *   - PA28: SI output pin
	 */
	void init_SI(void) {
			// Initialize Timer6 with a period and duty cycle for integration timing
			TIMG6_init(100, 255);

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

			// Ensure CLK is disabled until first SI pulse starts capture
			TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED;
			TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
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
