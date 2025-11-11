/**
 ******************************************************************************
 * @file    adc12.c
 * @brief   Configures and operates the ADC12 peripheral for the Lab 8 Monitor.
 * @details
 * This file initializes the ADC0 module (PA27) to be used by
 * the Lab 8 Heart Rate Monitor.
 *
 * The `ADC0_init` function configures the ADC for single, software-triggered
 * conversions.
 *
 * The `ADC0_getVal` function is a blocking function called by the
 * TIMG6 1000 Hz interrupt to acquire a single sample of the analog
 * heart rate signal.
 *
 * @authors 
 * Nick Fair  
 * Nathan Winiarski
 *
 * @date    November 10, 2025
 ******************************************************************************
 */

#include "/lab5/adc12.h"
#include "uart_extras.h"
#include "lab4/uart.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"

/**
 * @brief Initializes ADC0 for the Heart Rate Monitor.
 *
 * @details Configures ADC0 (PA27) as the analog input pin. It sets the
 * ADC clock, disables power down, and sets the conversion mode to
 * single-channel, software-triggered. This function is called once from main.
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

    // Configure PA27 as an analog input pin
    IOMUX->SECCFG.PINCM[IOMUX_PINCM60] = 0;

    // Select clock source for ADC0
    // Use ULPCLK and configure frequency range
    ADC0->ULLMEM.GPRCM.CLKCFG |= ADC12_CLKCFG_SAMPCLK_ULPCLK;
    ADC0->ULLMEM.CLKFREQ |= ADC12_CLKFREQ_FRANGE_RANGE40TO48;

    // Set up ADC control registers
    ADC0->ULLMEM.CTL0 = 0; // Clear CTL0 before configuration
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_SCLKDIV_DIV_BY_8;      // Sample clock divided by 8
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_PWRDN_MANUAL;        // Disable automatic power down

    // Configure memory control register 0 to use channel 0
    ADC0->ULLMEM.MEMCTL[0] = ADC12_MEMCTL_CHANSEL_CHAN_0;
    ADC0->ULLMEM.MEMCTL[0] = ADC12_MEMCTL_STIME_SEL_SCOMP0;

    // Set sample time (higher value = longer sampling window)
    ADC0->ULLMEM.SCOMP0 = 128;

    // Configure conversion sequence and trigger source
    ADC0->ULLMEM.CTL1 |= ADC12_CTL1_CONSEQ_SINGLE;        // Single conversion mode
    ADC0->ULLMEM.CTL2 |= ADC12_CTL2_STARTADD_ADDR_00;      // Start address for conversion results
    ADC0->ULLMEM.CTL1 |= ADC12_CTL1_TRIGSRC_SOFTWARE;    // Software-triggered conversion
    ADC0->ULLMEM.CTL1 |= ADC12_CTL1_SAMPMODE_AUTO;        // Automatic sampling mode

}

/**
 * @brief Performs a single ADC conversion and returns the 12-bit result.
 *
 * @details This is a blocking (polling) function that starts a software-triggered
 * conversion and waits for it to complete. It is called by the
 * TIMG6 1000 Hz interrupt (TIMG6_IRQHandler) to sample the analog
 * heart rate signal.
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
