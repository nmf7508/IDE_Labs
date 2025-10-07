#include "lab5/camera.h"
#include <ti/devices/msp/msp.h>
#include "sysctl.h"
#include "lab5/timers.h"
#include "lab5/adc12.h"
#include "lab4/uart.h"
#include "isrs.h"


// PA12 (PINCM34) CLK
void init_CLK(void) {
    TIMG0_init(320, 0);   // Adjust divider for pixel clock speed (try ~100kHz first)

    // Configure PA12 as output
    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    IOMUX->SECCFG.PINCM[IOMUX_PINCM34] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM34] &= ~IOMUX_PINCM_INENA_ENABLE;
    //IOMUX->SECCFG.PINCM[IOMUX_PINCM34] |= IOMUX_PINCM_INV_ENABLE;
		
		//TIMG0->COUNTERREGS.CTRCTL ^= GPTIMER_CTRCTL_EN_ENABLED;
		GPIOA->DOESET31_0 |= (1 << 12);
}


// PA28 (PINCM3) SI
void init_SI(void) {
    TIMG6_init(100, 255);   // sets integration time

    if (!(GPIOA->GPRCM.PWREN & GPIO_PWREN_ENABLE_ENABLE)) {
        GPIOA->GPRCM.RSTCTL = GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETASSERT_ASSERT;
        GPIOA->GPRCM.RSTCTL &= ~GPIO_RSTCTL_KEY_UNLOCK_W;
        GPIOA->GPRCM.PWREN = GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE;
        GPIOA->GPRCM.PWREN &= ~GPIO_PWREN_KEY_UNLOCK_W;
    }

    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] |= (0x80 | 0x01);
    IOMUX->SECCFG.PINCM[IOMUX_PINCM3] &= ~IOMUX_PINCM_INENA_ENABLE;
    //IOMUX->SECCFG.PINCM[IOMUX_PINCM3] |= IOMUX_PINCM_INV_ENABLE;
		
		TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
		GPIOA->DOESET31_0 |= (1 << 28);
}

/**
 * @brief Initialize camera associated components
*/
void Camera_init(void) {
    init_CLK();
    init_SI();
    cameraData_complete = 0;
    pixelCounter = 0;
    // Make sure CLK is disabled until SI starts a frame
    TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED;
}

uint8_t Camera_isDataReady(void) {
    return cameraData_complete;
}

uint16_t* Camera_getData(void) {
    cameraData_complete = 0;   // reset flag after user fetch
    return (uint16_t*)cameraData;
}
