#ifndef __FONT_H
#define __FONT_H

#include <stdint.h>
#include <avr/pgmspace.h>

typedef enum {
  LCD_A = 0, LCD_B, LCD_C, LCD_D, LCD_E, LCD_F, LCD_G, LCD_H, LCD_I, LCD_J,
	LCD_K, LCD_L, LCD_M, LCD_N, LCD_O, LCD_P, LCD_Q, LCD_R, LCD_S, 
	LCD_T, LCD_U, LCD_V, LCD_W, LCD_X, LCD_Y, LCD_Z,
	LCD_0, LCD_1, LCD_2, LCD_3, LCD_4,
	LCD_5, LCD_6, LCD_7, LCD_8, LCD_9, LCD_DEGREES,
	LCD_COLON, LCD_PERIOD, LCD_COMMA,
	LCD_SPACE, LCD_END = 0xFF 
} font_chars_t;

void drawstring(uint8_t x, uint8_t y, const uint8_t *str);
void drawletter(uint8_t x, uint8_t y, uint8_t c);

#endif