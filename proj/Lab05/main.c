/**
 * ******************************************************************************
 * @file    : main.c
 * @brief   : Main Application Entry for Lab 5 – Multi-Mode Data Acquisition
 * @details :
 *   This program initializes and manages various system configurations for 
 *   the MSP microcontroller based on the selected operating mode (`MODE` macro).
 *   Each mode demonstrates different hardware functionalities including timers, 
 *   LEDs, ADC sampling, UART communication, and camera interfacing.
 * 
 *   **Operating Modes:**
 *   - **MODE 0 – Stopwatch / Timer Demo:**
 *       - Uses timers to simulate a stopwatch with LED indicators.
 *       - External switch interrupts start/stop timing, UART displays elapsed time.
 *   - **MODE 1 – ADC Sampling Demo:**
 *       - Samples analog input periodically and transmits raw ADC readings via UART.
 *   - **MODE 2 – Temperature Sensor Demo:**
 *       - Converts ADC voltage readings into temperature values in °C and °F and sends them to UART.
 *   - **MODE 3 – Camera Capture:**
 *       - Interfaces with a line-scan camera.
 *       - Uses timers for SI (Start Integration) and CLK (pixel clock) control.
 *       - Collects 128 ADC samples per frame line and transmits pixel data through UART.
 *
 *   The core of the program relies on interrupt-driven behavior handled in `isrs.c`,
 *   while the main loop primarily monitors status flags and transfers data when ready.
 * 
 * @authors 
 *   Nick Fair  
 *   Nathan Winiarski
 * 
 * @date   10/07/2025
 * ******************************************************************************
 */

#include "lab4/uart.h"
#include "lab5/switches.h"
#include "lab5/timers.h"
#include "lab1/leds.h"
#include "isrs.h"
#include "lab5/adc12.h"
#include "uart_extras.h"
#include "lab5/camera.h"


/**
 * @brief Program entry point.
 * @details
 *   Initializes all necessary peripherals (UART, LEDs, switches, ADC, and timers) 
 *   based on the selected mode. Operation is mostly interrupt-driven, so each mode 
 *   runs an infinite loop that waits for interrupts or data flags to trigger actions.
 */
int main() {
    // Initialize communication and hardware peripherals
    UART0_init();           // UART for debugging and data transmission
    LED1_init();            // Status indicator LED1
    LED2_init();            // Status indicator LED2
    S1_init_interrupt();    // Configure Switch 1 (S1) with interrupt capability
    S2_init_interrupt();    // Configure Switch 2 (S2) with interrupt capability
    ADC0_init();            // Initialize ADC for data acquisition

#if MODE == 0
    // ------------------------------------------------------------
    // MODE 0: Stopwatch / LED Timer Demo
    // ------------------------------------------------------------
    TIMG6_init(31250, 255); // Timer6: ~2 Hz event to toggle LED1
    TIMG12_init(32000);     // Timer12: ~1 kHz tick counter for ms tracking

    // Infinite loop – all timing handled by interrupts
    while (1) {}

#elif MODE == 1 || MODE == 2
    // ------------------------------------------------------------
    // MODE 1: ADC Sampling
    // MODE 2: Temperature Measurement
    // ------------------------------------------------------------
    TIMG6_init(31250, 255); // Timer6: Triggers ADC sampling at set frequency

    // Infinite loop – samples processed and printed in ISR
    while (1) {}

#elif MODE == 3
    // ------------------------------------------------------------
    // MODE 3: Camera Line Capture
    // ------------------------------------------------------------
    Camera_init();           // Initialize camera interface (ADC + SI/CLK control)

    //UART0_put((uint8_t*)"\r\nLab 5 - Camera Test Start\r\n");

    while (1) {
        // Wait for the camera data ready flag to be set by ISR
        if (Camera_isDataReady()) {
            UART0_put((uint8_t*)"-1\r\n"); // Start-of-frame marker for data parsing

            uint16_t* data = Camera_getData(); // Retrieve 128-sample buffer

            // Toggle LED1 to indicate capture completion
            LED1_set(LED1_TOGGLE);

            // Stream pixel data over UART (raw 12-bit ADC values)
            for (int i = 0; i < 128; i++) {
                UART0_printDec(data[i]);     // Print ADC sample
                UART0_put((uint8_t*)"\r\n"); // Line break
            }

            UART0_put((uint8_t*)"-2\r\n"); // End-of-frame marker
        }
    }
#endif
}
