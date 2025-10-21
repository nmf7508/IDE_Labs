/**
 * @file timers.c
 * @brief Timer initialization functions for multiple General Purpose Timer (GPTIMER) modules.
 *
 * This file configures and enables Timer G0, G6, and G12 peripherals on the MSP microcontroller.
 * Each function powers up the timer, sets its clock source, mode, prescaler, and interrupt configuration.
 * These timers are used for timing, delays, periodic interrupts, and peripheral control (e.g., ADC sampling).
 *
 * @authors 
 *   Nick Fair  
 *   Nathan Winiarski
 * 
 * @date   10/21/2025
 */

#include "lab6/timers.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"
#include "lab4/uart.h"
#include "lab6/isrs.h"

/**
 * @brief Initializes Timer G0 as a general-purpose countdown timer.
 *
 * @param period     The value loaded into the timer counter. Determines the timer duration.
 * @param prescaler  The clock prescaler used to divide the timer’s input clock frequency.
 *
 * This function:
 * - Powers on and resets the TIMG0 peripheral (if not already enabled)
 * - Selects BUSCLK as the clock source
 * - Configures countdown, auto-reload, and repeat modes
 * - Enables interrupts for zero events
 * - Enables the timer in NVIC for interrupt handling
 *
 * @note Timer G0 resides in Power Domain 0 (PD0). Refer to the MSP Data Sheet, page 3.
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

/**
 * @brief Initializes Timer G6 for general-purpose timing with prescaler support.
 *
 * @param period     Timer load value that defines the timer duration.
 * @param prescaler  Prescaler value dividing the input clock frequency.
 *
 * This timer configuration is typically used for periodic ADC sampling or LED toggling.
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

/**
 * @brief Initializes Timer G12 as a general-purpose countdown timer without prescaler.
 *
 * @param period  Timer load value that defines the duration before a zero event occurs.
 *
 * This timer operates without prescaler control and is suitable for higher-frequency timing applications.
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

// Channel 0 - PB8, Channel 1 - PB12, Channel 2 - PB17, Channel 3 - PB13
void TIMA0_PWM_init(uint8_t pin, uint32_t period, uint32_t prescaler, double percentDutyCycle) {
	// Ensure TIMA0 is powered and reset properly
    if (!(TIMA0->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
        // Reset the module
        TIMA0->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
        TIMA0->GPRCM.RSTCTL &= ~GPTIMER_RSTCTL_KEY_UNLOCK_W;

        // Enable power
        TIMA0->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
        TIMA0->GPRCM.PWREN &= ~GPTIMER_PWREN_KEY_UNLOCK_W;
    }

    // Select BUSCLK as the clock source
    TIMA0->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;

    // Configure timer behavior
    TIMA0->COUNTERREGS.CTRCTL = 0;
    TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
    TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
    TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;

    // Set clock divider and prescaler
    TIMA0->CLKDIV |= GPTIMER_CLKDIV_RATIO_DIV_BY_8;
    TIMA0->COMMONREGS.CPS = prescaler;

    // Set period (reload value)
    TIMA0->COUNTERREGS.LOAD = period;

    // Enable timer clock
    TIMA0->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;
		
		if (pin <= 3){
			// Enable and configure GPIOB peripheral if not already active
			if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOB->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOB->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOB->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
			}
		}
		
		if (pin == 0){
			// Configure PB8 as output (no input, no inversion)
			IOMUX->SECCFG.PINCM[IOMUX_PINCM25] |= IOMUX_PINCM25_PF_TIMA0_CCP0 | IOMUX_PINCM_PC_CONNECTED;
			IOMUX->SECCFG.PINCM[IOMUX_PINCM25] &= ~IOMUX_PINCM_INENA_ENABLE;
			GPIOB->DOESET31_0 |= (1 << 8);
			
			TIMA0->COUNTERREGS.CC_01[0] = (uint32_t)((double)period * (1-percentDutyCycle));
			TIMA0->COUNTERREGS.CCCTL_01[0] = 0;
			TIMA0->COMMONREGS.CCPD |= 1;
			TIMA0->COUNTERREGS.CCACT_01[0] = GPTIMER_CCACT_01_LACT_CCP_HIGH | GPTIMER_CCACT_01_CDACT_CCP_LOW;
			TIMA0->COUNTERREGS.OCTL_01[0] = 0;

			// Enable TimerA0 Channel 0 (starts integration timing)
			TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
			}
		else if (pin == 1){
			// Configure PB12 as output (no input, no inversion)
			IOMUX->SECCFG.PINCM[IOMUX_PINCM29] |= (IOMUX_PINCM29_PF_TIMA0_CCP1 | IOMUX_PINCM_PC_CONNECTED);
			IOMUX->SECCFG.PINCM[IOMUX_PINCM29] &= ~IOMUX_PINCM_INENA_ENABLE;
			GPIOB->DOESET31_0 |= (1 << 12);
			
			TIMA0->COUNTERREGS.CC_01[1] = (uint32_t)((double)period * (1-percentDutyCycle));
			TIMA0->COUNTERREGS.CCCTL_01[1] = 0;
			TIMA0->COMMONREGS.CCPD |= 2;
			TIMA0->COUNTERREGS.CCACT_01[1] = GPTIMER_CCACT_01_LACT_CCP_HIGH | GPTIMER_CCACT_01_CDACT_CCP_LOW;
			TIMA0->COUNTERREGS.OCTL_01[1] = 0;
			
			// Enable TimerA0 Channel 1 (starts integration timing)
			TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
		}
		else if (pin == 2){
			// Configure PB17 as output (no input, no inversion)
			IOMUX->SECCFG.PINCM[IOMUX_PINCM43] |= (IOMUX_PINCM43_PF_TIMA0_CCP2 | IOMUX_PINCM_PC_CONNECTED);
			IOMUX->SECCFG.PINCM[IOMUX_PINCM43] &= ~IOMUX_PINCM_INENA_ENABLE;
			GPIOB->DOESET31_0 |= (1 << 17);
			
			TIMA0->COUNTERREGS.CC_23[0] = (uint32_t)((double)period * (1-percentDutyCycle));
			TIMA0->COUNTERREGS.CCCTL_23[0] = 0;
			TIMA0->COMMONREGS.CCPD |= 4;
			TIMA0->COUNTERREGS.CCACT_23[0] = GPTIMER_CCACT_23_LACT_CCP_HIGH | GPTIMER_CCACT_23_CDACT_CCP_LOW;
			TIMA0->COUNTERREGS.OCTL_23[0] = 0;
			
			// Enable TimerA0 Channel 2 (starts integration timing)
			TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
		}
		else if (pin ==3){
			// Configure PB13 as output (no input, no inversion)
			IOMUX->SECCFG.PINCM[IOMUX_PINCM30] |= (IOMUX_PINCM30_PF_TIMA0_CCP3 | IOMUX_PINCM_PC_CONNECTED);
			IOMUX->SECCFG.PINCM[IOMUX_PINCM30] &= ~IOMUX_PINCM_INENA_ENABLE;
			GPIOB->DOESET31_0 |= (1 << 13);
			
			TIMA0->COUNTERREGS.CC_23[1] = (uint32_t)((double)period * (1-percentDutyCycle));
			TIMA0->COUNTERREGS.CCCTL_23[1] = 0;
			TIMA0->COMMONREGS.CCPD |= 8;
			TIMA0->COUNTERREGS.CCACT_23[1] = GPTIMER_CCACT_23_LACT_CCP_HIGH | GPTIMER_CCACT_23_CDACT_CCP_LOW;
			TIMA0->COUNTERREGS.OCTL_23[1] = 0;
			
			// Enable TimerA0 Channel 3 (starts integration timing)
			TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
		}
		else{
			UART0_put((uint8_t*)("Error. TIMA0 Pin does not exist :(\r\n"));
		}
}

// Channel 0 - PB4
void TIMA1_PWM_init(uint8_t pin, uint32_t period, uint32_t prescaler, double percentDutyCycle) {
	// Ensure TIMG6 is powered and reset properly
    if (!(TIMA1->GPRCM.PWREN & GPTIMER_PWREN_ENABLE_ENABLE)) {
        // Reset the module
        TIMA1->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W | GPTIMER_RSTCTL_RESETASSERT_ASSERT;
        TIMA1->GPRCM.RSTCTL &= ~GPTIMER_RSTCTL_KEY_UNLOCK_W;

        // Enable power
        TIMA1->GPRCM.PWREN = GPTIMER_PWREN_KEY_UNLOCK_W | GPTIMER_PWREN_ENABLE_ENABLE;
        TIMA1->GPRCM.PWREN &= ~GPTIMER_PWREN_KEY_UNLOCK_W;
    }

    // Select BUSCLK as the clock source
    TIMA1->CLKSEL |= GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;

    // Configure timer behavior
    TIMA1->COUNTERREGS.CTRCTL = 0;
    TIMA1->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CM_DOWN;
    TIMA1->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_CVAE_LDVAL;
    TIMA1->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_REPEAT_REPEAT_1;

    // Set clock divider and prescaler
    TIMA1->CLKDIV |= GPTIMER_CLKDIV_RATIO_DIV_BY_8;
    TIMA1->COMMONREGS.CPS = prescaler;

    // Set period (reload value)
    TIMA1->COUNTERREGS.LOAD = period;

    // Enable timer clock
    TIMA1->COMMONREGS.CCLKCTL |= GPTIMER_CCLKCTL_CLKEN_ENABLED;
		
				if (pin <= 3){
			// Enable and configure GPIOB peripheral if not already active
			if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOB->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOB->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOB->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
			}
		}
		
		if (pin == 0){
			// Configure PB4 as output (no input, no inversion)
			IOMUX->SECCFG.PINCM[IOMUX_PINCM17] |= IOMUX_PINCM17_PF_TIMA1_CCP0 | IOMUX_PINCM_PC_CONNECTED;
			IOMUX->SECCFG.PINCM[IOMUX_PINCM17] &= ~IOMUX_PINCM_INENA_ENABLE;
			GPIOB->DOESET31_0 |= (1 << 17);
			
			TIMA1->COUNTERREGS.CC_01[0] = (uint32_t)((double)period * (1-percentDutyCycle));
			TIMA1->COUNTERREGS.CCCTL_01[0] = 0;
			TIMA1->COMMONREGS.CCPD |= 1;
			TIMA1->COUNTERREGS.CCACT_01[0] = GPTIMER_CCACT_01_LACT_CCP_HIGH | GPTIMER_CCACT_01_CDACT_CCP_LOW;
			TIMA1->COUNTERREGS.OCTL_01[0] = 0;

			// Enable TimerA0 Channel 0 (starts integration timing)
			TIMA1->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
			}
		else{
			UART0_put((uint8_t*)("Error. TIMA1 Pin does not exist :(\r\n"));
		}
}

	/**
 * @brief Update PWM duty cycle for Timer A0 channel.
 * @param pin Channel index (0–3)
 * @param percentDutyCycle Duty cycle (0.0–1.0)
 */
void TIMA0_PWM_DutyCycle(uint8_t pin, double percentDutyCycle) {
    uint32_t period = TIMA0->COUNTERREGS.LOAD;

    if (pin == 0)
        TIMA0->COUNTERREGS.CC_01[0] = (uint32_t)((double)period * (1 - percentDutyCycle));
    else if (pin == 1)
        TIMA0->COUNTERREGS.CC_01[1] = (uint32_t)((double)period * (1 - percentDutyCycle));
    else if (pin == 2)
        TIMA0->COUNTERREGS.CC_23[0] = (uint32_t)((double)period * (1 - percentDutyCycle));
    else if (pin == 3)
        TIMA0->COUNTERREGS.CC_23[1] = (uint32_t)((double)period * (1 - percentDutyCycle));
}


/**
 * @brief Update PWM duty cycle for Timer A1 (channel 0 on PB4)
 * @param pin Channel index (0)
 * @param percentDutyCycle Duty cycle (0.0–1.0)
 */
void TIMA1_PWM_DutyCycle(uint8_t pin, double percentDutyCycle) {
    if (pin != 0) return; // only channel 0 exists
    uint32_t period = TIMA1->COUNTERREGS.LOAD;
    TIMA1->COUNTERREGS.CC_01[0] = (uint32_t)((double)period * (1 - percentDutyCycle));
}
