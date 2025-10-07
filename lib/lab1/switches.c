```c
/**
 * ******************************************************************************
 * @file    : switches.c
 * @brief   : Switches module for initialization and interaction
 * @details : Provides initialization, state checking, and interrupt configuration
 *             for two hardware switches connected to GPIO ports on the MSP microcontroller.
 * 
 * @author  : Nicholas Fair
 * @date    : 09/01/2025
 * ******************************************************************************
 */

#include <ti/devices/msp/msp.h>
#include "lab5/switches.h"

/**
 * @brief Initialize Switch 1 (S1) hardware connection.
 *
 * Powers up and configures GPIOA for Switch 1 input functionality.
 * The pin is set as an input with a pull-down resistor to ensure a defined state
 * when the switch is open.
 *
 * @return None
 */
void S1_init(void) {
    // Check if GPIOA is already powered; if not, power and reset it
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        // Reset GPIOA peripheral
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;

        // Enable power to GPIOA
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    // Configure PA18 (PINCM40) for GPIO with pull-down resistor
    IOMUX->SECCFG.PINCM[IOMUX_PINCM40] |= (0x80 | 0x01);       // Connect peripheral, GPIO mode
    IOMUX->SECCFG.PINCM[IOMUX_PINCM40] |= IOMUX_PINCM_INENA_ENABLE; // Enable input
    IOMUX->SECCFG.PINCM[IOMUX_PINCM40] |= IOMUX_PINCM_PIPD_ENABLE;  // Enable pull-down
}

/**
 * @brief Initialize Switch 2 (S2) hardware connection.
 *
 * Powers up and configures GPIOB for Switch 2 input functionality.
 * The pin is set as an input with a pull-up resistor and inverted polarity,
 * meaning the logic reads as active-low when pressed.
 *
 * @return None
 */
void S2_init(void) {
    // Check if GPIOB is already powered; if not, power and reset it
    if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        // Reset GPIOB peripheral
        GPIOB->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOB->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;

        // Enable power to GPIOB
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOB->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    // Configure PB21 (PINCM49) for GPIO with pull-up and inversion
    IOMUX->SECCFG.PINCM[IOMUX_PINCM49] |= (0x80 | 0x01);          // Connect peripheral, GPIO mode
    IOMUX->SECCFG.PINCM[IOMUX_PINCM49] |= IOMUX_PINCM_INENA_ENABLE; // Enable input
    IOMUX->SECCFG.PINCM[IOMUX_PINCM49] |= IOMUX_PINCM_INV_ENABLE;   // Enable input inversion
    IOMUX->SECCFG.PINCM[IOMUX_PINCM49] |= IOMUX_PINCM_PIPU_ENABLE;  // Enable pull-up
}

/**
 * @brief Check if Switch 1 (S1) is currently pressed.
 *
 * Reads the input state of GPIOA pin 18. If the bit is set, the switch is pressed.
 *
 * @return int Returns 1 if pressed, otherwise 0.
 */
int S1_pressed(void) {
    return (GPIOA->DIN31_0 & (1 << 18));
}

/**
 * @brief Check if Switch 2 (S2) is currently pressed.
 *
 * Reads the input state of GPIOB pin 21. If the bit is set, the switch is pressed.
 *
 * @return int Returns 1 if pressed, otherwise 0.
 */
int S2_pressed(void) {
    return (GPIOB->DIN31_0 & (1 << 21));
}

/**
 * @brief Initialize interrupt for Switch 1 (S1).
 *
 * Configures GPIOA to generate an interrupt when Switch 1 is pressed (falling edge).
 * The interrupt is then enabled in the NVIC for handling in the ISR.
 *
 * @note Uses `NVIC_EnableIRQ()` to register the interrupt with the Nested Vectored Interrupt Controller.
 * @see cmsis_armclang.h
 */
void S1_init_interrupt(void) {
    __disable_irq();  // Disable global interrupts during setup

    S1_init();  // Initialize switch hardware

    // Clear and enable DIO18 interrupt (falling edge)
    GPIOA->CPU_INT.ICLR |= GPIO_GEN_EVENT1_ICLR_DIO18_CLR;
    GPIOA->CPU_INT.IMASK |= GPIO_GEN_EVENT1_IMASK_DIO18_SET;
    GPIOA->POLARITY31_16 |= GPIO_POLARITY31_16_DIO18_FALL;

    // Enable interrupt in NVIC
    NVIC_EnableIRQ(GPIOA_INT_IRQn);

    __enable_irq();   // Re-enable global interrupts
}

/**
 * @brief Initialize interrupt for Switch 2 (S2).
 *
 * Configures GPIOB to generate an interrupt when Switch 2 is pressed (rising edge).
 * The interrupt is then enabled in the NVIC for handling in the ISR.
 *
 * @note Uses `NVIC_EnableIRQ()` to register the interrupt with the Nested Vectored Interrupt Controller.
 * @see cmsis_armclang.h
 */
void S2_init_interrupt(void) {
    __disable_irq();  // Disable global interrupts during setup

    S2_init();  // Initialize switch hardware

    // Clear and enable DIO21 interrupt (rising edge)
    GPIOB->CPU_INT.ICLR |= GPIO_GEN_EVENT1_ICLR_DIO21_CLR;
    GPIOB->CPU_INT.IMASK |= GPIO_GEN_EVENT1_IMASK_DIO21_SET;
    GPIOB->POLARITY31_16 |= GPIO_POLARITY31_16_DIO21_RISE;

    // Enable interrupt in NVIC
    NVIC_EnableIRQ(GPIOB_INT_IRQn);

    __enable_irq();   // Re-enable global interrupts
}
```
