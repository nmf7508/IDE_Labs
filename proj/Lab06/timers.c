/**
 ******************************************************************************
 * @file    timers.c
 * @brief   Timer initialization for the Lab 8 Heart Rate Monitor.
 * @details
 * This file provides the timer initialization for the Lab 8 Heart Rate Monitor.
 * It configures TIMG6 to generate a 1000 Hz periodic interrupt, which is
 * used to trigger the ADC sampling in `isrs.c`.
 *
 * @authors
 * Nick Fair  
 * Nathan Winiarski
 *
 * @date    November 10, 2025
 ******************************************************************************
 */

#include "lab6/timers.h"
#include <ti/devices/msp/msp.h>
#include "lab6/isrs.h"

// ============================================================================
// TIMG6 — Heart Rate Monitor
// ============================================================================
/**
 * @brief Initializes TIMG6 for 1000 Hz periodic interrupts.
 * @param period     Load value defining timer interval
 * @param prescaler  Prescaler value dividing input clock
 * @details
 * Configures TIMG6 to generate a 1ms (1000 Hz) interrupt for the
 * Lab 8 Heart Rate Monitor. This interrupt triggers the ADC sampling
 * in the `TIMG6_IRQHandler`.
 *
 * Calculation (32MHz BUSCLK):
 * - Clock: 32,000,000 Hz
 * - CLKDIV /8: 4,000,000 Hz
 * - Prescaler (0): /1
 * - Period (4000): 4,000,000 / 4000 = 1000 Hz
 *
 * This function initializes PA28 as a debug output pin, which
 * is toggled in the ISR on every detected heartbeat.
 */
void TIMG6_init(uint32_t period, uint32_t prescaler) {
    // Power on and reset TIMG6
    if (!(TIMG6->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
        TIMG6->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
        TIMG6->GPRCM.RSTCTL &= ~GPTIMER_RSTCTL_KEY_UNLOCK_W;
        TIMG6->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
        TIMG6->GPRCM.PWREN &= ~GPTIMER_PWREN_KEY_UNLOCK_W;
    }

    // Configure timer clock and counting mode
    TIMG6->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;
    TIMG6->COUNTERREGS.CTRCTL = GPTIMER_CTRCTL_CM_DOWN | GPTIMER_CTRCTL_CVAE_LDVAL | GPTIMER_CTRCTL_REPEAT_REPEAT_1;
    TIMG6->CLKDIV |= GPTIMER_CLKDIV_RATIO_DIV_BY_8;
    TIMG6->COMMONREGS.CPS = prescaler;
    TIMG6->COUNTERREGS.LOAD = period;
    
    // Enable the timer clock (but not the timer itself yet)
    TIMG6->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;
        
    // Enable and configure GPIOA peripheral for debug LED
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    // Configure PA28 as output
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] &= ~IOMUX_PINCM_INENA_ENABLE;

    // Enable Timer6 and PA28 output
    TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
    GPIOA->DOESET31_0 |= (1 << 28);

    // Configure and enable the TIMG6 interrupt
    __disable_irq();
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;
    TIMG6->CPU_INT.IMASK |= GPTIMER_GEN_EVENT1_IMASK_Z_SET;
    NVIC_EnableIRQ(TIMG6_INT_IRQn);
    __enable_irq();
}
