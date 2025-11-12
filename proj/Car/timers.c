/**
 * ******************************************************************************
 * @file    timers.c
 * @brief   Timer initialization and PWM configuration for MSPM0 GPTIMER modules.
 * @details
 *   This file provides initialization and configuration functions for multiple
 *   general-purpose timer modules (TIMG0, TIMG6, TIMG12, TIMA0, TIMA1) used
 *   across different Lab 6 parts:
 *
 *   - TIMG0 -> Camera pixel clock generation (MODE 3)
 *   - TIMG6 -> Periodic interrupts (ADC sampling, SI pulse, LED toggle)
 *   - TIMG12 -> Stopwatch timing (MODE 0)
 *   - TIMA0 / TIMA1 -> PWM generation for motor and servo control (Part 2)
 *
 *   Each timer can be configured for countdown timing, auto-reload, and
 *   interrupt generation or for PWM output on specific GPIO pins.
 *
 * @authors
 *   Nick Fair  
 *   Nathan Winiarski
 *
 * @date   10/21/2025
 * ******************************************************************************
 */

#include "timers.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"
#include "uart.h"
#include "isrs.h"

// ============================================================================
// TIMG0 ? Camera Clock / Delay Timer Initialization
// ============================================================================
/**
 * @brief Initializes Timer G0 as a general-purpose countdown timer.
 * @param period     Load value defining timer duration before interrupt.
 * @param prescaler  Clock prescaler dividing the BUSCLK input.
 * @details
 *   Configures TIMG0 for countdown with auto-reload and repeat mode.
 *   Used primarily in **MODE 3** for camera pixel clock generation.
 */
void TIMG0_init(uint32_t period, uint32_t prescaler) {
    // Ensure TIMG0 is powered and reset properly
    if (!(TIMG0->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
        // Reset the module
        TIMG0->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
        TIMG0->GPRCM.RSTCTL &= ~GPTIMER_RSTCTL_KEY_UNLOCK_W;

        // Enable power
        TIMG0->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
        TIMG0->GPRCM.PWREN &= ~GPTIMER_PWREN_KEY_UNLOCK_W;
    }

    // Select BUSCLK (PD0) as the clock source
    TIMG0->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;

    // Configure timer in countdown mode with auto-reload and repeat
    TIMG0->COUNTERREGS.CTRCTL = 0;
    TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
    TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
    TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;

    // Set prescaler and period
    TIMG0->COMMONREGS.CPS = prescaler;
    TIMG0->COUNTERREGS.LOAD = period;

    // Enable the timer clock
    TIMG0->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;

    // Disable global interrupts during configuration
    __disable_irq();

    // Clear pending zero-event interrupt and enable new ones
    TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
    TIMG0->CPU_INT.IMASK |= GPTIMER_GEN_EVENT1_IMASK_Z_SET;

    // Enable timer counting
    TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;

    // Enable timer interrupt in NVIC
    NVIC_EnableIRQ(TIMG0_INT_IRQn);

    // Re-enable global interrupts
    __enable_irq();
}

// ============================================================================
// TIMG6 ? General Purpose Periodic Timer (ADC / LED / SI Pulse)
// ============================================================================
/**
 * @brief Initializes Timer G6 for periodic interrupts and timing.
 * @param period     Load value defining timer interval.
 * @param prescaler  Prescaler value dividing input clock.
 * @details
 *   Provides general-purpose timing functionality used for:
 *   - Periodic ADC sampling (MODE 1)
 *   - Temperature sensor conversion (MODE 2)
 *   - Camera SI pulse generation (MODE 3)
 */
void TIMG6_init(uint32_t period, uint32_t prescaler) {
    // Ensure TIMG6 is powered and reset properly
    if (!(TIMG6->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
        // Reset the module
        TIMG6->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
        TIMG6->GPRCM.RSTCTL &= ~GPTIMER_RSTCTL_KEY_UNLOCK_W;

        // Enable power
        TIMG6->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
        TIMG6->GPRCM.PWREN &= ~GPTIMER_PWREN_KEY_UNLOCK_W;
    }

    // Select BUSCLK as the clock source
    TIMG6->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;

    // Configure timer behavior
    TIMG6->COUNTERREGS.CTRCTL = 0;
    TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
    TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
    TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;

    // Set clock divider and prescaler
    TIMG6->CLKDIV |= GPTIMER_CLKDIV_RATIO_DIV_BY_8;
    TIMG6->COMMONREGS.CPS = prescaler;

    // Set period (reload value)
    TIMG6->COUNTERREGS.LOAD = period;

    // Enable timer clock
    TIMG6->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;

    // Disable interrupts during configuration
    __disable_irq();

    // Clear and enable zero-event interrupt
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
    TIMG6->CPU_INT.IMASK |= GPTIMER_GEN_EVENT1_IMASK_Z_SET;

    // Enable NVIC interrupt for TIMG6
    NVIC_EnableIRQ(TIMG6_INT_IRQn);

    // Re-enable interrupts
    __enable_irq();
}

// ============================================================================
// TIMG12 ? Stopwatch / Timekeeping Timer Initialization
// ============================================================================
/**
 * @brief Initializes Timer G12 for high-frequency timing.
 * @param period  Timer load value (counts down before interrupt).
 * @details
 *   Used in **MODE 0** for stopwatch timing and LED pattern display.
 */
void TIMG12_init(uint32_t period) {
    // Ensure TIMG12 is powered and reset properly
    if (!(TIMG12->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
        // Reset module
        TIMG12->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
        TIMG12->GPRCM.RSTCTL &= ~GPTIMER_RSTCTL_KEY_UNLOCK_W;

        // Enable power
        TIMG12->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
        TIMG12->GPRCM.PWREN &= ~GPTIMER_PWREN_KEY_UNLOCK_W;
    }

    // Select BUSCLK as clock source
    TIMG12->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;

    // Configure countdown and reload behavior
    TIMG12->COUNTERREGS.CTRCTL = 0;
    TIMG12->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
    TIMG12->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
    TIMG12->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;

    // Set load value
    TIMG12->COUNTERREGS.LOAD = period;

    // Enable timer clock
    TIMG12->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;

    // Disable interrupts during setup
    __disable_irq();

    // Clear pending and enable zero-event interrupts
    TIMG12->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
    TIMG12->CPU_INT.IMASK |= GPTIMER_GEN_EVENT1_IMASK_Z_SET;

    // Enable NVIC interrupt for TIMG12
    NVIC_EnableIRQ(TIMG12_INT_IRQn);

    // Re-enable interrupts
    __enable_irq();
}

// ============================================================================
// TIMA0 ? PWM Generator (Channels 0?3: PB8, PB12, PB17, PB13)
// ============================================================================
/**
 * @brief Initializes Timer A0 for PWM signal generation.
 * @param pin PWM channel index (0?3).
 * @param period PWM period value.
 * @param prescaler Clock prescaler.
 * @param percentDutyCycle Duty cycle (0.0?1.0).
 * @details
 *   Configures PWM outputs on GPIOB pins:
 *   - Channel 0 -> PB8
 *   - Channel 1 -> PB12
 *   - Channel 2 -> PB17
 *   - Channel 3 -> PB13
 *
 *   Used in Lab 6, Part 2 for stepper or DC motor PWM control.
 */
void TIMA0_PWM_init(uint8_t pin, uint32_t period, uint32_t prescaler, double percentDutyCycle) {
    if (!(TIMA0->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
        TIMA0->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
        TIMA0->GPRCM.RSTCTL &= ~GPTIMER_RSTCTL_KEY_UNLOCK_W;
        TIMA0->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
        TIMA0->GPRCM.PWREN &= ~GPTIMER_PWREN_KEY_UNLOCK_W;
    }

    TIMA0->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;
    TIMA0->COUNTERREGS.CTRCTL = GPTIMER_CTRCTL_CM_DOWN | GPTIMER_CTRCTL_CVAE_LDVAL | GPTIMER_CTRCTL_REPEAT_REPEAT_1;
    TIMA0->CLKDIV |= GPTIMER_CLKDIV_RATIO_DIV_BY_8;
    TIMA0->COMMONREGS.CPS = prescaler;
    TIMA0->COUNTERREGS.LOAD = period;
    TIMA0->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;

    // GPIO setup for PWM output pins
    if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOB->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOB->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOB->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    switch (pin) {
        case 0:
            IOMUX->SECCFG.PINCM[IOMUX_PINCM25] |= IOMUX_PINCM25_PF_TIMA0_CCP0 | IOMUX_PINCM_PC_CONNECTED;
            GPIOB->DOESET31_0 |= (1 << 8);
            TIMA0->COUNTERREGS.CC_01[0] = (uint32_t)((double)period * (1 - percentDutyCycle));
            TIMA0->COMMONREGS.CCPD |= 1;
            TIMA0->COUNTERREGS.CCACT_01[0] = GPTIMER_CCACT_01_LACT_CCP_HIGH | GPTIMER_CCACT_01_CDACT_CCP_LOW;
            break;

        case 1:
            IOMUX->SECCFG.PINCM[IOMUX_PINCM29] |= IOMUX_PINCM29_PF_TIMA0_CCP1 | IOMUX_PINCM_PC_CONNECTED;
            GPIOB->DOESET31_0 |= (1 << 12);
            TIMA0->COUNTERREGS.CC_01[1] = (uint32_t)((double)period * (1 - percentDutyCycle));
            TIMA0->COMMONREGS.CCPD |= 2;
            TIMA0->COUNTERREGS.CCACT_01[1] = GPTIMER_CCACT_01_LACT_CCP_HIGH | GPTIMER_CCACT_01_CDACT_CCP_LOW;
            break;

        case 2:
            IOMUX->SECCFG.PINCM[IOMUX_PINCM43] |= IOMUX_PINCM43_PF_TIMA0_CCP2 | IOMUX_PINCM_PC_CONNECTED;
            GPIOB->DOESET31_0 |= (1 << 17);
            TIMA0->COUNTERREGS.CC_23[0] = (uint32_t)((double)period * (1 - percentDutyCycle));
            TIMA0->COMMONREGS.CCPD |= 4;
            TIMA0->COUNTERREGS.CCACT_23[0] = GPTIMER_CCACT_23_LACT_CCP_HIGH | GPTIMER_CCACT_23_CDACT_CCP_LOW;
            break;

        case 3:
            IOMUX->SECCFG.PINCM[IOMUX_PINCM30] |= IOMUX_PINCM30_PF_TIMA0_CCP3 | IOMUX_PINCM_PC_CONNECTED;
            GPIOB->DOESET31_0 |= (1 << 13);
            TIMA0->COUNTERREGS.CC_23[1] = (uint32_t)((double)period * (1 - percentDutyCycle));
            TIMA0->COMMONREGS.CCPD |= 8;
            TIMA0->COUNTERREGS.CCACT_23[1] = GPTIMER_CCACT_23_LACT_CCP_HIGH | GPTIMER_CCACT_23_CDACT_CCP_LOW;
            break;

        default:
            UART0_put((uint8_t *)"Error: Invalid TIMA0 PWM pin.\r\n");
            return;
    }

    TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
}

// ============================================================================
// TIMA1 ? PWM Generator (Channel 0: PB4)
// ============================================================================
/**
 * @brief Initializes Timer A1 PWM output.
 * @param pin PWM channel (only 0 available).
 * @param period PWM period.
 * @param prescaler Clock prescaler.
 * @param percentDutyCycle Duty cycle (0.0?1.0).
 * @details
 *   Configures PB4 as TIMA1_CCP0 PWM output for small DC or servo motor control.
 */
void TIMA1_PWM_init(uint8_t pin, uint32_t period, uint32_t prescaler, double percentDutyCycle) {
		if (pin != 0) return;
    if (!(TIMA1->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
        TIMA1->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
        TIMA1->GPRCM.RSTCTL &= ~GPTIMER_RSTCTL_KEY_UNLOCK_W;
        TIMA1->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
        TIMA1->GPRCM.PWREN &= ~GPTIMER_PWREN_KEY_UNLOCK_W;
    }

    TIMA1->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;
    TIMA1->COUNTERREGS.CTRCTL = GPTIMER_CTRCTL_CM_DOWN | GPTIMER_CTRCTL_CVAE_LDVAL | GPTIMER_CTRCTL_REPEAT_REPEAT_1;
    TIMA1->CLKDIV |= GPTIMER_CLKDIV_RATIO_DIV_BY_8;
    TIMA1->COMMONREGS.CPS = prescaler;
    TIMA1->COUNTERREGS.LOAD = period;
    TIMA1->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;

    // Configure PB4 (TIMA1_CCP0)
    IOMUX->SECCFG.PINCM[IOMUX_PINCM17] |= IOMUX_PINCM17_PF_TIMA1_CCP0 | IOMUX_PINCM_PC_CONNECTED;
    GPIOB->DOESET31_0 |= (1 << 4);

    TIMA1->COUNTERREGS.CC_01[0] = (uint32_t)((double)period * (1 - percentDutyCycle));
    TIMA1->COMMONREGS.CCPD |= 1;
    TIMA1->COUNTERREGS.CCACT_01[0] = GPTIMER_CCACT_01_LACT_CCP_HIGH | GPTIMER_CCACT_01_CDACT_CCP_LOW;
    TIMA1->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
}

// ============================================================================
// PWM Duty Cycle Update Functions
// ============================================================================
/**
 * @brief Updates PWM duty cycle for TIMA0 channel.
 * @param pin PWM channel index (0?3)
 * @param percentDutyCycle Duty cycle (0.0?1.0)
 */
void TIMA0_PWM_DutyCycle(uint8_t pin, double percentDutyCycle) {
    uint32_t period = TIMA0->COUNTERREGS.LOAD;
    if (pin == 0) TIMA0->COUNTERREGS.CC_01[0] = (uint32_t)(period * (1 - percentDutyCycle));
    else if (pin == 1) TIMA0->COUNTERREGS.CC_01[1] = (uint32_t)(period * (1 - percentDutyCycle));
    else if (pin == 2) TIMA0->COUNTERREGS.CC_23[0] = (uint32_t)(period * (1 - percentDutyCycle));
    else if (pin == 3) TIMA0->COUNTERREGS.CC_23[1] = (uint32_t)(period * (1 - percentDutyCycle));
}

/**
 * @brief Updates PWM duty cycle for TIMA1 (channel 0 only).
 * @param pin Channel index (0)
 * @param percentDutyCycle Duty cycle (0.0?1.0)
 */
void TIMA1_PWM_DutyCycle(uint8_t pin, double percentDutyCycle) {
    if (pin != 0) return;
    uint32_t period = TIMA1->COUNTERREGS.LOAD;
    TIMA1->COUNTERREGS.CC_01[0] = (uint32_t)(period * (1 - percentDutyCycle));
}
