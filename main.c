#include <string.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#include <util/delay_basic.h>
#include <util/delay.h>
#include <stdint.h>

#include "clock.h"
//#include "buttons.h"
#include "ssd1306.h"
#include "max6675.h"
#include "spi.h"
#include "font.h"

void setup() {
	// Initialise the display hardware
	ssd1306_init();
	init_clock();

	max6675_init();
}

void print_num(uint8_t y, uint16_t num) {
	uint8_t x = 20;

	for (int i = 0; i < 5; i++) {
		uint8_t digit = num % 10;
		num /= 10;

		drawletter(x, y, LCD_0 + digit);

		x -= 5;
	}
}

int main()
{
	setup();

  /* Allow voltages to settle */
	_delay_ms(200);

	start_clock();

	//video_vline(20, 20, 40, 1);
	drawstring(0, 0, string_test1);
	drawstring(0, 9, string_test2);
	drawstring(0, 22, string_test3);

	uint8_t count = 0;
	while (1) {
		uint16_t temp = max6675_read_raw();
		video_rect(0, 38, 36, 19, 0);
		print_num(38, temp);
		print_num(49, count++);
		ssd1306_update();

		// at least 250mS between reads.
		_delay_ms(500);
	};

	return 0;
}
