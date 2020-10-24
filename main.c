#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#include <util/delay_basic.h>
#include <stdint.h>

#include "clock.h"
#include "buttons.h"
#include "ssd1306.h"
#include "spi.h"

#define FONT_PTR _binary_font8_tif_raw_start
extern const uint8_t FONT_PTR[] PROGMEM;

/* Font data */
enum {LCD_A = 0, LCD_C, LCD_D, LCD_E, LCD_F, LCD_G, LCD_H, LCD_I,
	LCD_K, LCD_L, LCD_M, LCD_N, LCD_O, LCD_P, LCD_R, LCD_S, 
	LCD_T, LCD_U, LCD_W, LCD_X, LCD_Y,
	LCD_ap, LCD_in, LCD_he, LCD_fmj,
	LCD_0, LCD_1, LCD_2, LCD_3, LCD_4,
	LCD_5, LCD_6, LCD_7, LCD_8, LCD_9,
	LCD_stripes, LCD_SMALLD, LCD_COLON, LCD_PERIOD, 
	LCD_SPACE, LCD_END = 0xFF 
} font_chars;

// Fixme: this can be moved to progmem
// Font character offsets and widths - FIXME should generate this at compile time from widths only.
struct {
	uint16_t start;
	uint8_t width;
} chars[] = {
	// A C D E F
	{0, 7}, {7, 6}, {13, 6}, {19, 5}, {24, 5},
	// G H I K L
	{29, 7}, {36, 6}, {42, 1}, {43, 6}, {49, 6},
	// M N O P R
	{55, 7}, {62, 6}, {68, 7}, {75, 5}, {80, 5},
	// S T U W X Y
	{85, 6}, {91, 5}, {96, 6}, {102, 7}, {109, 7}, {116, 7},
	// ap in he fmj
	{123, 9}, {132, 6}, {138, 10}, {148, 13},
	// 0123456789
	{161, 4}, {165, 4}, {169, 4}, {173, 4}, {177, 4}, 
	{181, 4}, {185, 4}, {189, 4}, {193, 4}, {197, 4},
	// stripes, small D, colon, period
	{201, 15}, {216, 5}, {221 ,2}, {223, 2}
};

// FIXME move all the font stuff into its own file
inline uint8_t get_charslice(uint8_t c, uint8_t pos)
{
	return pgm_read_byte(FONT_PTR + chars[c].start + pos);
}

// Example of text string
const uint8_t string_dnacheck[] PROGMEM = {LCD_D, LCD_N, LCD_A, 
	LCD_SPACE, LCD_C, LCD_H, LCD_E, LCD_C, LCD_K, LCD_END};

void drawletter(uint8_t x, uint8_t y, uint8_t c)
{
	int i;

	for (i = 0; i < chars[c].width; i++)
	{
		video_drawbits(x + i, y, get_charslice(c, i));
	}
}

void drawstring(uint8_t x, uint8_t y, const uint8_t *str)
{
	uint8_t c = pgm_read_byte(str++);

	while (c != LCD_END)
	{
		if (c == LCD_SPACE)
		{
			x += 3;
		}
		else
		{
	 		drawletter(x, y, c);
			x += chars[c].width + 1;
		}
		c = pgm_read_byte(str++);
	}
}

void delay(int time)
{
	while (time--)
	{
		/* 1msec delay */
		_delay_loop_2(F_CPU / 4000);
	}
}


void setup() {
	// Initialise the display hardware
	ssd1306_init();
	//init_clock();
}

int main()
{
	setup();

  /* Allow voltages to settle */
	delay(200);

	//start_clock();

	//video_vline(20, 20, 40, 1);
	drawstring(20, 6, string_dnacheck);

	ssd1306_update();

	while (1)
		;

	return 0;
}
