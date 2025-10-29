/**
 ******************************************************************************
 * @file    main.c
 * @brief   Main application file for Lab 6 - Motor Control
 * @details This program demonstrates PWM-based control of three types of motors:
 *          - Part 1: DC Motor (speed control via PWM duty cycle)
 *          - Part 2: Stepper Motor (sequential stepping control)
 *          - Part 3: Servo Motor (position control via PWM pulse width)
 *
 *          The program uses MSPM0 timers and peripherals to generate PWM signals,
 *          control GPIOs, and communicate over UART for debugging.
 *
 * @authors 
 *   Nick Fair
 *   Nathan Winiarski
 *
 * @date    October 2025
 ******************************************************************************
 */

#include "lab6/timers.h"
#include "lab5/switches.h"
#include "lab4/uart.h"
#include "lab1/leds.h"
#include <ti/devices/msp/msp.h>
#include "motor.h"

/* --------------------------------------------------------------------------
 *                               Configuration
 * -------------------------------------------------------------------------- */

/**
 * @brief Select which part of Lab 6 to run:
 *        1 = DC Motor Control
 *        2 = Stepper Motor Control
 *        3 = Servo Motor Control
 */
#define PART 1

/** 
 * @brief Initial PWM duty cycle for DC motor control (30%).
 */
static volatile double duty = 0.3;

/* --------------------------------------------------------------------------
 *                             Local Functions
 * -------------------------------------------------------------------------- */

/**
 * @brief Simple software delay loop.
 * @note  Used to create visible time intervals for PWM duty cycle changes.
 */
static void delay(void) {
    volatile uint32_t i;
    for (i = 0; i < 2000000U; i++) {
    }
}

/* --------------------------------------------------------------------------
 *                                 Main
 * -------------------------------------------------------------------------- */

int main(void) {

    /* ---------------- Disable global interrupts during setup ---------------- */
    __disable_irq();

    /* -------------------- Peripheral Initializations ------------------------ */
    LED1_init();              // Initialize onboard LED1
    UART0_init();             // Initialize UART0 for serial communication
    S1_init_interrupt();      // Initialize Switch 1 with interrupt
    S2_init_interrupt();      // Initialize Switch 2 with interrupt

    /**
     * @brief Initialize Timer A0 channels for PWM output (used for DC/Stepper).
     * @param (channel, period, phase, duty)
     *        - Channel 0–3 configured with 10kHz PWM frequency (400 count period)
     */
    TIMA0_PWM_init(0, 400, 0, duty);  // Channel 0: 30% duty cycle
    TIMA0_PWM_init(1, 400, 0, 0);     // Channel 1: initially off
    TIMA0_PWM_init(2, 400, 0, duty);  // Channel 2: 30% duty cycle
    TIMA0_PWM_init(3, 400, 0, 0);     // Channel 3: initially off

    /**
     * @brief Initialize Timer A1 channel for servo PWM (50Hz frequency).
     *        50Hz period ? ~20ms, duty cycle between 5–10% for servo range.
     */
    TIMA1_PWM_init(0, 313, 255, 0.075);  // Servo neutral position (7.5%)

    /**
     * @brief Initialize General Timer G0 (used for periodic timing functions).
     */
    TIMG0_init(1250, 255);

    /* ------------------ Enable global interrupts after setup ---------------- */
    __enable_irq();

    /* ---------------------- UART startup message --------------------------- */
    UART0_put((uint8_t *)"Motor Control Lab 6\r\n");

    /* =======================================================================
     *                            PART 1: DC MOTOR
     * ======================================================================= */
#if PART == 1
    while (1) {
        // Ramp up forward direction
        TIMA0_PWM_DutyCycle(1, 0);
        TIMA0_PWM_DutyCycle(0, 0); delay();
        TIMA0_PWM_DutyCycle(0, 0.1); delay();
        TIMA0_PWM_DutyCycle(0, 0.3); delay();
        TIMA0_PWM_DutyCycle(0, 0.5); delay();
        TIMA0_PWM_DutyCycle(0, 0.7); delay();
        TIMA0_PWM_DutyCycle(0, 0.9); delay();
        TIMA0_PWM_DutyCycle(0, 1.0); delay(); delay();

        // Ramp down
        TIMA0_PWM_DutyCycle(0, 0.9); delay();
        TIMA0_PWM_DutyCycle(0, 0.7); delay();
        TIMA0_PWM_DutyCycle(0, 0.5); delay();
        TIMA0_PWM_DutyCycle(0, 0.3); delay();
        TIMA0_PWM_DutyCycle(0, 0.1); delay();
        TIMA0_PWM_DutyCycle(0, 0.0); delay();

        // Reverse direction (swap channels)
        TIMA0_PWM_DutyCycle(1, 0.1); delay();
        TIMA0_PWM_DutyCycle(1, 0.3); delay();
        TIMA0_PWM_DutyCycle(1, 0.5); delay();
        TIMA0_PWM_DutyCycle(1, 0.7); delay();
        TIMA0_PWM_DutyCycle(1, 0.9); delay();
        TIMA0_PWM_DutyCycle(1, 1.0); delay(); delay();

        // Ramp down reverse
        TIMA0_PWM_DutyCycle(1, 0.9); delay();
        TIMA0_PWM_DutyCycle(1, 0.7); delay();
        TIMA0_PWM_DutyCycle(1, 0.5); delay();
        TIMA0_PWM_DutyCycle(1, 0.3); delay();
        TIMA0_PWM_DutyCycle(1, 0.1); delay();
        TIMA0_PWM_DutyCycle(1, 0.0); delay();
    }
#endif

    /* =======================================================================
     *                         PART 2: STEPPER MOTOR
     * ======================================================================= */
#if PART == 2
    Stepper_Motor_Init();
    UART0_put((uint8_t *)"Lab 6: Stepper Motor Demo\r\n");

    int forward = 1; // Direction: 1 = forward, 0 = reverse

    while (1) {
        Stepper_Motor_Step(forward); // Step one increment
        delay_ms(3);                 // Adjust speed with delay (smaller = faster)
    }
#endif

    /* =======================================================================
     *                           PART 3: SERVO MOTOR
     * ======================================================================= */
#if PART == 3
    UART0_put((uint8_t *)"Lab 6: Servo Motor Demo\r\n");

    while (1) {
        // Sweep servo between 0°, 90°, and 180° positions
        delay_ms(1000);
        TIMA1_PWM_DutyCycle(0, 0.05);   // 0° position
        delay_ms(1000);
        TIMA1_PWM_DutyCycle(0, 0.075);  // 90° (neutral)
        delay_ms(1000);
        TIMA1_PWM_DutyCycle(0, 0.10);   // 180° position
        delay_ms(1000);
        TIMA1_PWM_DutyCycle(0, 0.075);  // Return to center
    }
#endif
}
