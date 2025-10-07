#ifndef _ISRS_H_
#define _ISRS_H_

#define MODE 0 //0 -> Part 1, 1 -> Part 2 (Light Sensor), 2 -> Part 2 (Temp Sensor), 3 -> Part 3

void GROUP1_IRQHandler(void);
void TIMG0_IRQHandler(void);
void TIMG6_IRQHandler(void);
void TIMG12_IRQHandler(void);


#endif
