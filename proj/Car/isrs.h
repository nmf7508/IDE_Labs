/**
 * ******************************************************************************
 * @file     isrs.h
 * @brief    Interrupt Service Routine (ISR) header definitions.
 * @details
 * This header declares the interrupt handler prototypes used in the MSPM0
 * microcontroller project. These routines handle events from GPIO, timers,
 * and ADC peripherals. The active behavior of each ISR depends on the selected
 * operating mode defined by the `MODE` macro:
 *
 * - MODE 0: Stopwatch and timing demonstration using timers and UART output.
 * - MODE 1: Light sensor mode – periodic ADC sampling with UART transmission.
 * - MODE 2: Temperature sensor mode – ADC temperature data acquisition
 * and conversion to °C and °F.
 * - MODE 3: Camera mode – synchronized ADC sampling for line capture using
 * timer-generated SI and CLK signals.
 *
 * Each interrupt handler defined in `isrs.c` responds to hardware-triggered
 * events, clears interrupt flags, and manages control logic for Lab 6 tasks.
 *
 * @authors
 * Nick Fair  
 * Nathan Winiarski
 *
 * @date 10/07/2025
 * ******************************************************************************
 */

#ifndef _ISRS_H_
#define _ISRS_H_

/**
 * @brief Selects which Lab 6 mode the ISR logic operates under.
 *
 * - 0 -> Stopwatch / Part 1 demonstration  
 * - 1 -> Light Sensor (ADC periodic sampling)  
 * - 2 -> Temperature Sensor (ADC conversion)  
 * - 3 -> Camera Capture (synchronized sampling)
 */
#define MODE 3  /* <<< THIS LINE IS CHANGED FROM 0 TO 3 */

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------

/**
 * @brief Handles Group 1 interrupts (e.g., external GPIO events).
 * @details Used to toggle timers and LEDs or trigger UART output depending on MODE.
 */
void GROUP1_IRQHandler(void);

/**
 * @brief Timer0 interrupt handler.
 * @details Generates camera CLK signal and reads ADC pixel data (MODE 3).
 */
void TIMG0_IRQHandler(void);

/**
 * @brief Timer6 interrupt handler.
 * @details Handles periodic timing for ADC sampling, temperature conversion,
 * or camera SI pulse generation.
 */
void TIMG6_IRQHandler(void);

/**
 * @brief Timer12 interrupt-handler.
 * @details Tracks elapsed time for stopwatch function (MODE 0).
 */
void TIMG12_IRQHandler(void);

#endif /* _ISRS_H_ */
