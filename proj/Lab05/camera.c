#include "lab5/camera.h"
#include <ti\devices\msp\msp.h>
#include "sysctl.h"
#include "lab5/timers.h"
#include "lab5/adc12.h"

static bool cameraData_complete = 0;
static int cameraData[128];
static int pixelCounter = 0;

/**
 * @brief Initialize camera associ = 0;ated components
*/
void Camera_init(void){
	ADC0_init();
	TIMG0_init(320, 0);
	TIMG6_init(31250, 255);
	//TIMG12_init(32000);
	// Disable TIMG0 (CLK)
	TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_DISABLED;
}

void TIMG0_IRQHandler(void) {
	TIMG0->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

	// pulse the CLK
	
	Camera_getData()
}

void TIMG6_IRQHandler(void) {
	TIMG6->CPU_INT.ICLR |= GPTIMER_GEN_EVENT1_ICLR_Z_CLR;

	// Disable TIMG0 (CLK)
	TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_DISABLED;
	
	// Check if data is bring processed. 0 = False, 1 = True
	if(cameraData_complete == 0){
		// trigger SI first then clock. then SI back to 0 before next rising clock edge.
		
		// Disable TIMG0 (CLK)
		TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_ENABLED;
		
		
}


/**
 * @brief Checks whether camera data is ready to retrieve
 * @note Make sure to check all data is available
 * @return True(1)/False(0) if camera data is ready
*/
uint8_t Camera_isDataReady(void){
	
	
}


/**
 * @brief Retrieves pointer to camera data array
 * @return Pointer to global data array stored locally in this file
*/
uint16_t* Camera_getData(void){
	
}