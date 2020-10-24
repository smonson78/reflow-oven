#include "font.h"

#include "ssd1306.h"

#define FONT_PTR _binary_font8_tif_raw_start
extern const uint8_t FONT_PTR[];

// Fixme: this can be moved to progmem
// Font character offsets and widths - FIXME should generate this at compile time from widths only.
struct {
	uint16_t start;
	uint8_t width;
} chars[] = {
	// A B C D E F
	{0, 7}, {7, 5}, {12, 6}, {18, 6}, {24, 5}, {29, 5},
	// G H I J K L
	{34, 7}, {41, 6}, {47, 1}, {48, 6}, {54, 6}, {60, 6},
	// M N O P Q R
	{66, 7}, {73, 6}, {79, 7}, {86, 5}, {91, 7}, {98, 5},
	// S T U V W X Y Z
	{103, 6}, {109, 5}, {114, 6}, {120, 6}, {126, 7}, {133, 7}, {140, 7}, {147, 6},
	// 0123456789
	{153, 4}, {157, 4}, {161, 4}, {165, 4}, {169, 4}, 
	{173, 4}, {177, 4}, {181, 4}, {185, 4}, {189, 4},
  // degrees, colon, period, comma
  {193, 3}, {196, 1}, {197, 2}, {199, 2}
};

// FIXME move all the font stuff into its own file
inline uint8_t get_charslice(uint8_t c, uint8_t pos)
{
	return pgm_read_byte(FONT_PTR + chars[c].start + pos);
}

// Example of text string
const uint8_t string_test1[] PROGMEM = {
  LCD_A, LCD_B, LCD_C, LCD_D, LCD_E, LCD_F, LCD_G, LCD_H, LCD_I, LCD_J, LCD_K, LCD_L, LCD_M, 
  LCD_N, LCD_O, LCD_P, LCD_Q, LCD_R, LCD_S, LCD_END
};

const uint8_t string_test2[] PROGMEM = {
  LCD_T, LCD_U, LCD_V, LCD_W, LCD_X, LCD_Y, LCD_Z, LCD_0, LCD_1, LCD_2, LCD_3, LCD_4, LCD_5, LCD_6, 
  LCD_7, LCD_8, LCD_9, LCD_DEGREES, LCD_END
};

const uint8_t string_test3[] PROGMEM = {
  LCD_V, LCD_A, LCD_L, LCD_U, LCD_E, LCD_COLON, LCD_SPACE, LCD_3, LCD_7, LCD_PERIOD, LCD_5, LCD_DEGREES, LCD_COMMA, 
	LCD_SPACE, LCD_6, LCD_2, LCD_DEGREES, LCD_END
};


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