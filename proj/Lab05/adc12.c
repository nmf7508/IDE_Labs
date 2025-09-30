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
    IOMUX->SECCFG.PINCM[IOMUX_PINCM60] = (IOMUX_PINCM_PC_CONNECTED | IOMUX_PINCM60_PF_ADC12_A0);

    // Select clock (ULPCLK is fine, highest frequency range)
    ADC0->ULLMEM.GPRCM.CLKCFG = ADC12_CLKCFG_SAMPCLK_ULPCLK;
    ADC0->ULLMEM.CTL0 = ADC12_CTL0_SCLKDIV_DIV_BY_1;

    // Configure for single channel, software trigger
    ADC0->ULLMEM.CTL0 = 0;
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_PWREN_ENABLE;       // enable power
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_CONVMODE_ONESHOT;   // one-shot conversion
    ADC0->ULLMEM.CTL0 |= ADC12_EVT_MODE_INT0_CFG_SOFTWARE;       // software trigger

    // Map channel 0 (A0) to MEMRES0
    ADC0->ULLMEM.MEMCTL = 0;   // use channel 0
}

uint32_t ADC0_getVal(void) {
    // Start conversion
    ADC0->ULLMEM.CTL0 |= ADC12_CTL0_START_START;

    // Wait until conversion complete
    while (!(ADC0->ULLMEM.STAT & ADC12_STAT_EOC0_MASK)) {
        __asm("nop");
    }

    // Read result
    return (uint32_t)(ADC0->ULLMEM.MEMRES[0] & 0x0FFF);
}
