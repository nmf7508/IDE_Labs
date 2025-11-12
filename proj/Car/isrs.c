/**
 ******************************************************************************
 * @file     isrs.c
 * @brief    Interrupt Service Routines (ISRs) for the MSPM0 microcontroller.
 * @details  
 * This file implements interrupt handlers for hardware-triggered events
 * including GPIO interrupts, timer overflows, and ADC conversions.
 *
 * The behavior of each ISR is determined by the `MODE` macro, which enables
 * flexible use of the same interrupt architecture across multiple labs:
 *
 * | MODE | Functionality Description |
 * |------|----------------------------|
 * | 0 | Stopwatch/timing demonstration using timers and UART |
 * | 1 | Periodic ADC sampling with UART output |
 * | 2 | Temperature sensor sampling and unit conversion |
 * | 3 | Line-scan camera capture sequence with synchronized CLK and SI |
 *
 * Each ISR ensures interrupt flags are cleared, system timing remains
 * synchronized, and peripheral-specific behavior is executed based on the
 * current lab mode.
 *
 * @authors
 * Nick Fair  
 * Nathan Winiarski
 *
 * @date     October 7, 2025
 ******************************************************************************
 */

#include <ti/devices/msp/msp.h>
#include "leds.h"
#include "isrs.h"
#include "uart.h"
#include "uart_extras.h"
#include "adc12.h"
#include "camera.h"
#include "timers.h"
#include <stdio.h>
#include <stdbool.h> // <-- You may need to add this include

/* --------------------------------------------------------------------------
 * Global Variables and Flags
 * -------------------------------------------------------------------------- */

#if MODE == 0
static int timerOn = 0;           /**< Stopwatch state flag (1 = running, 0 = stopped). */
static long int timeElapsed = 0;    /**< Elapsed time counter in milliseconds. */
#endif

volatile uint8_t cameraData_complete = 0; /**< Set to 1 when a full camera line is captured. */
volatile int pixelCounter = 0;            /**< Counts CLK pulses for pixel sampling (includes dummy cycles). */
uint16_t cameraData[128];                 /**< Storage buffer for one 128-pixel line. */
volatile int delayOver = 0;               /**< Delay completion flag, used for timer-driven events. */
volatile bool g_car_running = false;      /**< ADD THIS LINE: Flag to start the car from main() */

#if MODE == 3
static bool read;                         /**< Toggles between read and idle states for camera CLK synchronization. */
#endif


/* --------------------------------------------------------------------------
 * CPUSS Group 1 Interrupt Handler
 * -------------------------------------------------------------------------- */

/**
 * @brief Handles Group 1 interrupts (external interrupt sources).
 * @details  
 * This ISR manages user input (e.g., push buttons) or GPIO-triggered
 * interrupts associated with Group 1.
 *
 * Behavior varies based on `MODE`:
 * - MODE 0: Toggles stopwatch start/stop and displays elapsed time.
 * - MODE 1–2: Toggles timer and LED1 for visual debugging.
 */
void GROUP1_IRQHandler(void) {
    switch (CPUSS->INT_GROUP[1].IIDX) {
        case 1: // External Interrupt 0 (e.g., Button S1)
            CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT0;

            g_car_running = true; // ADD THIS LINE: Set the flag for main()

#if MODE == 0 || MODE == 1 || MODE == 2
            // Toggle Timer 6 enable state and LED1 for visual feedback
            TIMG6->COUNTERREGS.CTRCTL ^= GPTIMER_CTRCTL_EN_ENABLED;
            LED1_set(LED1_TOGGLE);
#endif
            break;

        case 2: // External Interrupt 1 (e.g., Button S2)
            CPUSS->INT_GROUP[1].ICLR |= CPUSS_INT_GROUP_ICLR_INT_INT1;

#if MODE == 0
            // Stopwatch control (Timer12 toggled on/off)
            TIMG12->COUNTERREGS.CTRCTL ^= GPTIMER_CTRCTL_EN_ENABLED;

            if (timerOn) {
                // Stop stopwatch and print elapsed time
                LED2_set(0);
                timerOn = 0;
                char str[20];
                sprintf(str, "%ld", timeElapsed);
                UART0_put((uint8_t*)str);
                UART0_put((uint8_t*)" ms\r\n");
                timeElapsed = 0;
            } else {
                // Start stopwatch
                timerOn = 1;
            }
#endif
            break;

        default:
            break;
    }
}


/* --------------------------------------------------------------------------
 * Timer 0 Interrupt Handler (Camera CLK)
 * -------------------------------------------------------------------------- */

/**
 * @brief Timer 0 ISR — manages the camera pixel clock (CLK) signal.
 * @details  
 * Used in MODE 3 for line-scan camera synchronization.
 * Each interrupt corresponds to a CLK pulse, and ADC samples are collected
 * after the appropriate number of dummy cycles.
 *
 * - Generates 128 valid pixel reads after 18 dummy cycles.
 * - Stops CLK and raises the `cameraData_complete` flag once done.
 * - Toggles `read` to alternate between clock edges.
 */
void TIMG0_IRQHandler(void) {
    // Clear interrupt flag
    TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
    TIMG6->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED;
    delayOver = 1;

#if MODE == 3
    // Toggle GPIO pin for CLK output or debug observation
    GPIOA->DOUTTGL31_0 |= (1 << 12);

    // First clock edge after SI pulse — ensure SI is pulled low
    if (pixelCounter == 1) {
        GPIOA->DOUTCLR31_0 |= (1 << 28);
    }

    if (read) {
        pixelCounter++;

        // Skip first 18 dummy cycles
        if (pixelCounter > 18 && pixelCounter <= (18 + 128)) {
            int idx = pixelCounter - 19;
            cameraData[idx] = (uint16_t)ADC0_getVal();
        }

        // All 128 pixels captured
        if (pixelCounter >= (18 + 128)) {
            cameraData_complete = 1;  // Mark frame complete
            pixelCounter = 0;
            TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED; // Stop CLK
        }
    }

    // Alternate read state between rising/falling clock edges
    read = !read;
#endif
}


/* --------------------------------------------------------------------------
 * Timer 6 Interrupt Handler
 * -------------------------------------------------------------------------- */

/**
 * @brief Timer 6 ISR — performs periodic tasks based on `MODE`.
 * @details
 * - MODE 0: Toggles LED1 periodically (time demo).
 * - MODE 1: Samples ADC and prints raw value.
 * - MODE 2: Samples temperature sensor, converts to °C/°F, prints via UART.
 * - MODE 3: Generates camera "Start Integration" (SI) pulse and enables CLK timer.
 */
void TIMG6_IRQHandler(void) {
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

#if MODE == 0
    LED1_set(LED1_TOGGLE);

#elif MODE == 1
    int val = (int)ADC0_getVal();
    UART0_put((uint8_t*)"Sample: ");
    UART0_printDec(val);
    UART0_put((uint8_t*)"\r\n");

#elif MODE == 2
    int val = (int)ADC0_getVal();
    double tempC = ((((double)val * 3.3) / 4095.0) - 0.5) * 100.0;
    UART0_put((uint8_t*)"Temp in C: ");
    UART0_printFloat(tempC);
    UART0_put((uint8_t*)", in F: ");
    UART0_printFloat(tempC * 9.0 / 5.0 + 32.0);
    UART0_put((uint8_t*)"\r\n");

#elif MODE == 3
    // Generate SI pulse and start camera frame capture
    if (!cameraData_complete) {
        GPIOA->DOUTSET31_0 = (1 << 28);   // SI high
        pixelCounter = 0;               // Reset counter
        TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED; // Enable CLK
    }
    read = 1; // Set to ensure first CLK edge captures data
#endif
}


/* --------------------------------------------------------------------------
 * Timer 12 Interrupt Handler
 * -------------------------------------------------------------------------- */

/**
 * @brief Timer 12 ISR — stopwatch time tracking (MODE 0).
 * @details  
 * Increments an elapsed time counter and cycles LED2 indicators
 * every 500 ms to visualize time progression.
 */
void TIMG12_IRQHandler(void) {
    TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

#if MODE == 0
    // Toggle LED2 pattern every 500 ms
    if (timeElapsed % 500 == 0 && timeElapsed / 500 < 7) {
        LED2_set((LED2State)(timeElapsed / 500 + 1));
    }
    timeElapsed++; // Increment time counter (ms)
#endif
}
