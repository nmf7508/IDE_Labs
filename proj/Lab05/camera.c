#include "lab5/camera.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"
#include "lab5/timers.h"
#include "lab5/adc12.h"

static volatile uint8_t cameraData_complete = 0;
static volatile int pixelCounter = 0;       // counts CLK edges (including dummy cycles)
static uint32_t cameraData[128];            // store 128 pixels

// PB12 (PINCM29) ? CLK
void init_CLK(void) {
    TIMG0_init(320, 0);   // Adjust divider for pixel clock speed (try ~100kHz first)

    // Configure GPIOB12 as output if needed
    if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOB->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOB->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOB->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    IOMUX->SECCFG.PINCM[IOMUX_PINCM29] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM29] &= ~IOMUX_PINCM_INENA_ENABLE;
    IOMUX->SECCFG.PINCM[IOMUX_PINCM29] |= IOMUX_PINCM_INV_ENABLE;
}


// PB16 (PINCM33) ? SI
void init_SI(void) {
    TIMG6_init(31250, 255);   // frame rate timer (sets integration time)

    if (!(GPIOB->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOB->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOB->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOB->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOB->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    IOMUX->SECCFG.PINCM[IOMUX_PINCM33] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM33] &= ~IOMUX_PINCM_INENA_ENABLE;
    IOMUX->SECCFG.PINCM[IOMUX_PINCM33] |= IOMUX_PINCM_INV_ENABLE;
}

/**
 * @brief Initialize camera associated components
*/
void Camera_init(void) {
    ADC0_init();
    init_CLK();
    init_SI();
    cameraData_complete = 0;
    pixelCounter = 0;

    // Make sure CLK is disabled until SI starts a frame
    TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED;
}

/**
 * CLK ISR (TIMG0) — shifts pixels out of the camera
 */
void TIMG0_IRQHandler(void) {
    TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

    pixelCounter++;

    // Skip first 18 dummy cycles, then read 128 pixels
    if (pixelCounter > 18 && pixelCounter <= (18 + 128)) {
        int idx = pixelCounter - 19;
        cameraData[idx] = ADC0_getVal();
    }

    // Done capturing a full line
    if (pixelCounter >= (18 + 128)) {
        cameraData_complete = 1;
        pixelCounter = 0;
        TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED;  // stop CLK until next SI
    }
}

/**
 * SI ISR (TIMG6) — pulses SI and starts a new frame
 */
void TIMG6_IRQHandler(void) {
    TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

    if (!cameraData_complete) {
        // Pulse SI high then low
        GPIOB->DOUTSET31_0 = (1 << 16);
        GPIOB->DOUTCLR31_0 = (1 << 16);

        // Reset counters and start CLK
        pixelCounter = 0;
        TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
    }
}

uint8_t Camera_isDataReady(void) {
    return cameraData_complete;
}

uint16_t* Camera_getData(void) {
    cameraData_complete = 0;   // reset flag after user fetch
    return (uint16_t*)cameraData;
}
