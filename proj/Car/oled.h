/**
 * ******************************************************************************
 * @file    : oled.h
 * @brief   : OLED module header file
 * @details : Header instantiation for OLED functions
 * 
 * @author RAJ JOHRI
 * @author Louis J. Beato
 * @author anc2613
 * @author tlk1160
 * @date 8/13/2025
 * Created On: Mar 12, 2019
 * ******************************************************************************
*/

#include <ti/devices/msp/msp.h>
// #include "fontpack.h"
#ifndef _OLED_H_
#define _OLED_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "math.h"

// this is the predefined i2c address for the 1306 OLED display
#define SSD1306_ADDR                0x3C

#define SSD1306_DISPLAYON           0xAF
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_MULTIPLEX_RATIO     0xA8

// registers
#define SSD1306_CONTROL_REG         0x00  // Co = 0, DC = 0
#define SSD1306_DATA_REG            0x40

// SSD1306 PARAMETERS
#define SSD1306_LCDWIDTH            128
#define SSD1306_LCDHEIGHT           64
#define SSD1306_MAXROWS             7
#define SSD1306_MAXCONTRAST         0xFF

    // command table
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON        0xA5
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF

    // scrolling commands
#define SSD1306_SCROLL_RIGHT        0x26
#define SSD1306_SCROLL_LEFT         0X27
#define SSD1306_SCROLL_VERT_RIGHT   0x29
#define SSD1306_SCROLL_VERT_LEFT    0x2A
#define SSD1306_SET_VERTICAL        0xA3

    // address setting
#define SSD1306_SETLOWCOLUMN        0x00
#define SSD1306_SETHIGHCOLUMN       0x10
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_COLUMNADDRESS       0x21
#define SSD1306_COLUMNADDRESS_MSB   0x00
#define SSD1306_COLUMNADDRESS_LSB   0x7F
#define SSD1306_PAGEADDRESS         0x22
#define SSD1306_PAGE_START_ADDRESS  0xB0

    // hardware configuration
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_SEGREMAP            0xA1
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETCOMPINS          0xDA

    // timing and driving
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE       0xD9
#define SSD1306_SETVCOMDETECT      0xDB
#define SSD1306_NOP                0xE3

    // power supply configuration
#define SSD1306_CHARGEPUMP         0x8D
#define SSD1306_EXTERNALVCC        0x10
#define SSD1306_SWITCHCAPVCC       0x20

extern bool DisplayCameraData;
extern bool DisplayThreshCameraData;
extern unsigned char OLED_TEXT_ARR[1024];
extern unsigned char OLED_GRAPH_ARR[1024];
static char text[4][15] = {0};
static int textIndex = 0;

extern void OLED_sendCommand(unsigned char);
extern void OLED_write_display(unsigned char *data);
extern void OLED_Init(void);
extern void OLED_Print(int row, int col, char *txt);
extern void OLED_draw(int x, int y, unsigned char ascii);
extern void OLED_draw_extended(int x, int y, unsigned char ascii);
extern void OLED_PrintLine(char *str);
extern void draw_1d_data(uint16_t *camera_in_array);
void oled_set_cursor(uint8_t x, uint8_t y);
void oled_write_char(char c, uint8_t x, uint8_t y);
void oled_data(uint8_t *data, uint32_t size);
void oled_update_screen(void) ;
void oled_write_string(char *str, uint8_t x, uint8_t y);
void OLED_draw_smallfont(int x, int y, unsigned char ascii);
void OLED_PrintBlock(int startRow, int startCol, char *lines[], int numLines);
//************************logo_array****************************/
extern void OLED_display_logos(int x, int y, unsigned char ascii_index);

extern void OLED_draw_line(int x ,int y,unsigned char ascii_str[16]);
extern void OLED_ClearTextArr(void);

/* Set Entire Display OFF */
extern void OLED_display_off(void);
/* Set Entire Display ON */
extern void OLED_display_on(void);
/* Clear display */
extern void OLED_display_clear(void);
extern void OLED_DisplayCameraData(uint16_t line[]);

#endif
