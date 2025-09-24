#include <ti/devices/msp/msp.h>
#include "lab4/I2C/i2c.h"
#include "sysctl.h"

void I2C1_init(uint16_t targetAddress) {
	uint32_t pwren = I2C1->GPRCM.PWREN;
	if(!(pwren & I2C_PWREN_ENABLE_ENABLE)) {
		//Unlock RSTCTL assert reset then lock RSTCTL again
		I2C1->GPRCM.RSTCTL = I2C_RSTCTL_KEY_UNLOCK_W | I2C_RSTCTL_RESETASSERT_ASSERT;
		I2C1->GPRCM.RSTCTL &= ~I2C_RSTCTL_KEY_UNLOCK_W;
		
		//Unlock PWREN assert power enable then lock PWREN again
		I2C1->GPRCM.PWREN = I2C_PWREN_KEY_UNLOCK_W | I2C_PWREN_ENABLE_ENABLE;
		I2C1->GPRCM.PWREN &= ~I2C_PWREN_KEY_UNLOCK_W;
	}
	
	IOMUX->SECCFG.PINCM[IOMUX_PINCM15] = 0x80 |
		IOMUX_PINCM15_PF_I2C1_SCL |
		IOMUX_PINCM_INENA_ENABLE |
		IOMUX_PINCM_HIZ1_ENABLE |
		IOMUX_PINCM_PIPU_ENABLE;
	IOMUX->SECCFG.PINCM[IOMUX_PINCM16] = 0x80 |
		IOMUX_PINCM16_PF_I2C1_SDA |
		IOMUX_PINCM_INENA_ENABLE |
		IOMUX_PINCM_HIZ1_ENABLE |
		IOMUX_PINCM_PIPU_ENABLE;
	
	//Set Clock Source as BUS_CLK
	I2C1->CLKSEL = I2C_CLKSEL_BUSCLK_SEL_ENABLE;
	
	//Set Clock Div to 1
	I2C1->CLKDIV = I2C_CLKDIV_RATIO_DIV_BY_1;
	
	//Disable Analog Glitch Suppression
	I2C1->GFCTL &= ~I2C_GFCTL_AGFEN_ENABLE;
	
	I2C1->MASTER.MCTR = 0U;
	
	//get ULPClkRate
	unsigned int ULPCLKRate = SYSCTL_SYSCLK_getULPCLK();
	unsigned int i2c_freq = 400000;
	unsigned int TPR = (ULPCLKRate / (i2c_freq * (4+6))) - 1;
	
	I2C1->MASTER.MTPR = (uint8_t) TPR;
	
	I2C1->MASTER.MFIFOCTL = I2C_MFIFOCTL_RXTRIG_LEVEL_1 |
													I2C_MFIFOCTL_TXTRIG_EMPTY;
	
	I2C1->MASTER.MCR &= ~I2C_MCR_CLKSTRETCH_ENABLE;
	
	I2C1->MASTER.MSA = (uint8_t)(targetAddress << I2C_MSA_SADDR_OFS);
	
	I2C1->MASTER.MCR |= I2C_MCR_ACTIVE_ENABLE;
}

void I2C1_putchar(unsigned char ch) {
	while ((I2C1->MASTER.MFIFOSR & I2C_MFIFOSR_TXFIFOCNT_MASK) == 0) {
		__ASM("nop");
	}
	I2C1->MASTER.MTXDATA = ch;
}

void I2C1_put(unsigned char *data, uint16_t data_size) {
	I2C1->MASTER.MCTR	= 
		(((uint32_t)data_size << I2C_MCTR_MBLEN_OFS) & I2C_MCTR_MBLEN_MASK) |
		I2C_MCTR_START_ENABLE |
		I2C_MCTR_STOP_ENABLE |
		I2C_MCTR_BURSTRUN_ENABLE;
	for (uint16_t i = 0; i < data_size; i++) {
		I2C1_putchar(data[i]);
	}
	while (!(I2C1->MASTER.MSR & I2C_MSR_IDLE_SET)) {
    __ASM("nop");
	}
	I2C1->MASTER.MCTR &= ~I2C_MCTR_BURSTRUN_ENABLE;
}
