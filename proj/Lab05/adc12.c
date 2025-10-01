#include "/lab5/adc12.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"

void ADC0_init(void) {
    // Reset and power up ADC0
    if (!(ADC0->ULLMEM.GPRCM.PWREN & ADC12_PWREN_ENABLE_ENABLE)) {
        ADC0->ULLMEM.GPRCM.RSTCTL = ADC12_RSTCTL_KEY_UNLOCK_W | ADC12_RSTCTL_RESETASSERT_ASSERT;
        ADC0->ULLMEM.GPRCM.RSTCTL &= ~ADC12_RSTCTL_KEY_UNLOCK_W;

        ADC0->ULLMEM.GPRCM.PWREN = ADC12_PWREN_KEY_UNLOCK_W | ADC12_PWREN_ENABLE_ENABLE;
        ADC0->ULLMEM.GPRCM.PWREN &= ~ADC12_PWREN_KEY_UNLOCK_W;
    }

    // Configure PA27 (ADC0.0) pin for analog
    IOMUX->SECCFG.PINCM[IOMUX_PINCM60] = 0;

    // Select clock (ULPCLK is fine, highest frequency range)
    ADC0->ULLMEM.GPRCM.CLKCFG |= ADC12_CLKCFG_SAMPCLK_ULPCLK;
		ADC0->ULLMEM.CLKFREQ |= ADC12_CLKFREQ_FRANGE_RANGE40TO48;

    // Configure for single channel, software trigger
    ADC0->ULLMEM.CTL0 = 0;
		ADC0->ULLMEM.CTL0 |= ADC12_CTL0_SCLKDIV_DIV_BY_8;
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_PWRDN_MANUAL;       // disable power down
		
		ADC0->ULLMEM.MEMCTL[0] = ADC12_MEMCTL_CHANSEL_CHAN_0;
		
		ADC0->ULLMEM.CTL1 |= ADC12_CTL1_CONSEQ_SINGLE;
		ADC0->ULLMEM.CTL2 |= ADC12_CTL2_STARTADD_ADDR_00;
		ADC0->ULLMEM.CTL1 |= ADC12_CTL1_TRIGSRC_SOFTWARE;
		ADC0->ULLMEM.CTL1 |= ADC12_CTL1_SAMPMODE_AUTO;
		
		//doesnt look like you can set this reg
    //ADC0->ULLMEM.EVT_MODE |= ADC12_EVT_MODE_INT0_CFG_SOFTWARE;       // software trigger

    // Map channel 0 (A0) to MEMRES0
    //ADC0->ULLMEM.MEMCTL[ADC12_MEMCTL_CHANSEL_CHAN_0] = 0;   // use channel 0
}

uint32_t ADC0_getVal(void) {
    // Start conversion
		ADC0->ULLMEM.CTL0 |= ADC12_CTL0_ENC_ON;
		ADC0->ULLMEM.CTL1 |= ADC12_CTL1_SC_START;

    // Wait until conversion complete
    while (!(ADC0->ULLMEM.STATUS & ADC12_STATUS_BUSY_ACTIVE)) {
        __asm("nop");
    }

    // Read result
    return (uint32_t)(ADC0->ULLMEM.MEMRES[0] & 0x0FFF);
}
