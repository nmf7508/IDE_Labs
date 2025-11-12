/**
 ******************************************************************************
 * @file     isrs.c
 * @brief    Interrupt Service Routines (ISRs) for the MSPM0 microcontroller.
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
#include <stdbool.h>
#include "DCMotors.h"

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

// --- Global Flags for Bluetooth Control ---
volatile bool g_car_running = false;
volatile bool g_debug_mode = false;     // Shared with main.c, toggled by S2 or 't'
volatile double g_Kp = 0.6;             // Initial Kp value, shared with main.c

#if MODE == 3
static bool read;                         /**< Toggles between read and idle states for camera CLK synchronization. */
#endif


/* --------------------------------------------------------------------------
 * GPIOA Interrupt Handler (S1 Button)
 * -------------------------------------------------------------------------- */

/**
 * @brief Handles GPIOA interrupts (S1 - PA18).
 */
void GPIOA_IRQHandler(void) {
    // Check if the interrupt is from S1 (PA18)
    if (GPIOA->CPU_INT.RIS & GPIO_GEN_EVENT1_RIS_DIO18_SET) {
        // Clear the interrupt flag
        GPIOA->CPU_INT.ICLR = GPIO_GEN_EVENT1_ICLR_DIO18_CLR;
        
        g_car_running = true; // Set the flag for main()
    }
}

/* --------------------------------------------------------------------------
 * GPIOB Interrupt Handler (S2 Button)
 * -------------------------------------------------------------------------- */

/**
 * @brief Handles GPIOB interrupts (S2 - PB21).
 */
void GPIOB_IRQHandler(void) {
    // Check if the interrupt is from S2 (PB21)
    if (GPIOB->CPU_INT.RIS & GPIO_GEN_EVENT1_RIS_DIO21_SET) {
        // Clear the interrupt flag
        GPIOB->CPU_INT.ICLR = GPIO_GEN_EVENT1_ICLR_DIO21_CLR;
        
        g_debug_mode = !g_debug_mode; // Toggle the debug flag
    }
}


/* --------------------------------------------------------------------------
 * Timer 0 Interrupt Handler (Camera CLK)
 * -------------------------------------------------------------------------- */
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

/* --------------------------------------------------------------------------
 * UART1 Interrupt Handler (Bluetooth Commands)
 * -------------------------------------------------------------------------- */
void UART1_IRQHandler(void) {
    // Check if the interrupt is a Receive Interrupt
    if (UART1->CPU_INT.RIS & UART_CPU_INT_RIS_RXINT_SET) {
        // Clear the receive interrupt flag
        UART1->CPU_INT.ICLR = UART_CPU_INT_ICLR_RXINT_CLR;

        // Read the character from the data register
        char cmd = (char)(UART1->RXDATA & UART_RXDATA_DATA_MASK);

        switch (cmd) {
            case 's': // STOP (Kill Command)
                g_car_running = false;
                Motor_Stop(); // Call Motor_Stop() immediately
                break;
                
            case 'g': // GO (Resume)
                g_car_running = true;
                break;
                
            case 't': // TOGGLE Debug
                g_debug_mode = !g_debug_mode;
                break;
                
            case 'p': // Kp UP
                g_Kp += 0.05;
                break;
                
            case 'o': // Kp DOWN (can't use 'p-')
                g_Kp -= 0.05;
                if (g_Kp < 0) g_Kp = 0; // Don't allow negative Kp
                break;
        }
    }
}
