#include <string.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
//#include <avr/signature.h>
#include <avr/boot.h>

#include <util/delay_basic.h>
#include <util/delay.h>
#include <stdint.h>

#include "clock.h"
//#include "buttons.h"
#include "ssd1306.h"
#include "max6675.h"
#include "spi.h"
#include "font.h"
#include "adc.h"

void setup() {
	// Initialise the display hardware
	ssd1306_init();
	init_clock();
	max6675_init();
	adc_init();
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
		//uint16_t temp = max6675_read_raw();
		uint16_t temp = adc_read(8); // Internal temperature sensor
		

		// Contents of the signature bytes:
		//  0  1  2  3  4  5  6  7  8  9
		// 1e a2 95 ff 0f cc ff 26 ff 08
		// ^^    ^^    ^^                 device signature
		//    ^^                          RC calibration
		//       ^^                       temp sensor offset apparently??? clearly wrong since that's signature byte 2
		//          ^^                    temp sensor gain - also clearly wrong as it's 0xff

		// (adc - (273 + 100 - ts_offset)) * 128

		// Guess: TS_OFFSET is supposed to be address 9 and TS_GAIN is supposed to be offset 5.

		int8_t ts_offset = boot_signature_byte_get(9);
		uint8_t ts_gain = boot_signature_byte_get(5);
		int16_t top = temp - (373 - ts_offset);
		top *= 128;
		top /= ts_gain;
		top += 25;

		video_rect(0, 38, 36, 19, 0);
		print_num(38, top);
		drawletter(25, 38, LCD_DEGREES);
		drawletter(30, 38, LCD_C);
		print_num(49, count++);
		ssd1306_update();

		// at least 250mS between reads.
		_delay_ms(500);
	};

	return 0;
}
