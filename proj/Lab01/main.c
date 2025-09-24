/**
 * ******************************************************************************
 * @file    : main.c
 * @brief   : main function for Lab 1
 * @details : Reads switch input and sets LEDs outputs based on the inputs
 * 
 * @author  : Nicholas Fair
 * @date    : 09/01/2025
 * ******************************************************************************
*/

#include "lab1/leds.h"
#include "lab1/switches.h"

int main() {
	//Initialize leds and switches
	LED1_init();
	LED2_init();
	S1_init();
	S2_init();
	
	//Order of light colors on LED2
	LED2State LED2Order[10] = {LED2_RED, LED2_GREEN, LED2_BLUE, LED2_OFF,
	LED2_CYAN, LED2_MAGENTA, LED2_YELLOW, LED2_OFF, LED2_WHITE, LED2_OFF};
	
	int animation_active = 0;
	int index = 0;
	int position = 0;
	while (1) {
		//if switch 1 is pressed turn LED1 on, else turn LED1 off (or keep it off)
		if (S1_pressed()) {
			LED1_set(LED1_ON);
		}
		else {
			LED1_set(LED1_OFF);
		}
		//initalize animation variables used for LED2 color cycle
		if (S2_pressed() && !animation_active) {
			animation_active = 1;
			index = 0;
			position = position % 10;
		}
		
		if (animation_active) {
			//after 1000000 loops increment position and set LED2 to next color in the array
			if (index % 250000 == 0) {
				LED2_set(LED2Order[position%10]);
				
				//if the current position in the array is LED2 Off end the animation set LED to off 
				if (LED2Order[position%10] == LED2_OFF) {
					animation_active = 0;
					LED2_set(LED2_OFF);
				}
				//increment position
				position++;
			}
			index++;
		}
	}
}
