/**
 * @file adc12.c
 * @brief Configures and operates the ADC12 peripheral for analog-to-digital conversion.
 *
 * This file initializes the ADC0 module on the MSP microcontroller,
 * sets up its clock, sampling mode, and input channel configuration,
 * and provides a function to perform and retrieve a single conversion result.
 */

#include "/lab5/adc12.h"
#include "uart_extras.h"
#include "lab4/uart.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"

/**
 * @brief Initializes ADC0 for single-channel, software-triggered analog-to-digital conversions.
 *
 * Configures ADC0.0 (PA27) as the input pin, sets the sampling clock source and frequency,
 * and defines sampling parameters such as timing and conversion mode.
 */
void ADC0_init(void) {
    // Check if ADC0 is already powered on; if not, perform power and reset sequence
    if (!(ADC0->ULLMEM.GPRCM.PWREN & ADC12_PWREN_ENABLE_ENABLE)) {
        // Assert reset to ADC0
        ADC0->ULLMEM.GPRCM.RSTCTL = ADC12_RSTCTL_KEY_UNLOCK_W | ADC12_RSTCTL_RESETASSERT_ASSERT;
        ADC0->ULLMEM.GPRCM.RSTCTL &= ~ADC12_RSTCTL_KEY_UNLOCK_W;

        // Enable power to ADC0
        ADC0->ULLMEM.GPRCM.PWREN = ADC12_PWREN_KEY_UNLOCK_W | ADC12_PWREN_ENABLE_ENABLE;
        ADC0->ULLMEM.GPRCM.PWREN &= ~ADC12_PWREN_KEY_UNLOCK_W;
    }

    // Configure PA27 (ADC0.0) as an analog input pin
    IOMUX->SECCFG.PINCM[IOMUX_PINCM60] = 0;

    // Select clock source for ADC0
    // Use ULPCLK (Ultra-Low-Power Clock) and configure frequency range
    ADC0->ULLMEM.GPRCM.CLKCFG |= ADC12_CLKCFG_SAMPCLK_ULPCLK;
    ADC0->ULLMEM.CLKFREQ |= ADC12_CLKFREQ_FRANGE_RANGE40TO48;

    // Set up ADC control registers
    ADC0->ULLMEM.CTL0 = 0; // Clear CTL0 before configuration
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_SCLKDIV_DIV_BY_8;     // Sample clock divided by 8
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_PWRDN_MANUAL;         // Disable automatic power down

    // Configure memory control register 0 to use channel 0 (A0)
    ADC0->ULLMEM.MEMCTL[0] = ADC12_MEMCTL_CHANSEL_CHAN_0;
    ADC0->ULLMEM.MEMCTL[0] = ADC12_MEMCTL_STIME_SEL_SCOMP0;

    // Set sample time (higher value = longer sampling window)
    ADC0->ULLMEM.SCOMP0 = 128;

    // Configure conversion sequence and trigger source
    ADC0->ULLMEM.CTL1 |= ADC12_CTL1_CONSEQ_SINGLE;         // Single conversion mode
    ADC0->ULLMEM.CTL2 |= ADC12_CTL2_STARTADD_ADDR_00;      // Start address for conversion results
    ADC0->ULLMEM.CTL1 |= ADC12_CTL1_TRIGSRC_SOFTWARE;      // Software-triggered conversion
    ADC0->ULLMEM.CTL1 |= ADC12_CTL1_SAMPMODE_AUTO;         // Automatic sampling mode

    // Optional: Event mode register (not available on some variants)
    // ADC0->ULLMEM.EVT_MODE |= ADC12_EVT_MODE_INT0_CFG_SOFTWARE;
}

/**
 * @brief Performs a single ADC conversion and returns the digital result.
 *
 * Starts a new conversion using the software trigger and waits until it completes.
 *
 * @return uint32_t 12-bit digital result from ADC0 channel 0 (range: 0–4095)
 */
uint32_t ADC0_getVal(void) {
    // Enable conversion
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_ENC_ON;

    // Start conversion using software trigger
    ADC0->ULLMEM.CTL1 |= ADC12_CTL1_SC_START;

    // Wait for conversion to complete (BUSY flag clears when done)
    while (!(ADC0->ULLMEM.STATUS & ADC12_STATUS_BUSY_ACTIVE)) {
        __asm("nop"); // Do nothing while waiting
    }

    // Read and return 12-bit ADC result
    return (uint32_t)(ADC0->ULLMEM.MEMRES[0] & 0x0FFF);
}
