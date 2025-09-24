#include "lab4/I2C/oled.h"
#include "lab1/switches.h"
/*
int main() {
	OLED_Init();
	while(1) {
		OLED_Print(1, 1, "Hello World");
		OLED_Print(2, 2, "How are you");
		OLED_Print(3, 3, "Goodbye");
		for (volatile int i = 0; i < 3000000; i++) {
		}
		OLED_display_clear();
		uint16_t cameraLine[128];
		for (uint16_t i = 0; i < 128; i++) {
				cameraLine[i] = (i*i);
		}

		OLED_DisplayCameraData(cameraLine);
		for (volatile int i = 0; i < 3000000; i++) {
		}
		OLED_display_clear();
		OLED_PrintLine("Nick Fair");
		for (volatile int i = 0; i < 3000000; i++) {
		}
		OLED_display_clear();
	}
}
*/

int main() {
	OLED_Init();
	S1_init();
	while (1) {
		OLED_display_clear();
		OLED_Print(1, 5, "   _   ");
		OLED_Print(2, 5, " _|#|_ ");
		OLED_Print(3, 5, "| (0) |");
		OLED_Print(4, 5, " ----- ");
		while(!S1_pressed());
		OLED_display_clear();
		OLED_Print(1, 3, " _____       ");
		OLED_Print(2, 3, "(.---.)-._.-.");
		OLED_Print(3, 3, " /:::\\ _.---'");
		OLED_Print(4, 3, "'-----'      ");
		while(!S1_pressed());
		OLED_display_clear();
		OLED_Print(1, 2, "  _   ,_,   _");
		OLED_Print(2, 2, " / `'=) (='` \\");
		OLED_Print(3, 2, "/.-.-.\\ /.-.-.\\");
		OLED_Print(4, 2, "`      \"      `");
		while(!S1_pressed());
	}
}

	
