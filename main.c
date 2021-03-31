#include <string.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/boot.h>

#include <util/delay_basic.h>
#include <util/delay.h>
#include <stdint.h>

#include "clock.h"
#include "buttons.h"
#include "ssd1306.h"
#include "max6675.h"
#include "spi.h"
#include "font.h"
#include "adc.h"

#define RELAY_DDR_REG DDRD
#define RELAY_PORT_REG PORTD
#define RELAY_PIN 2

void setup() {
	// Initialise the display hardware
	ssd1306_init();
	init_clock();
	max6675_init();
	adc_init();

	// User buttons pull-ups
	PORTC |= _BV(BUTTON_1_PIN) | _BV(BUTTON_2_PIN) | _BV(BUTTON_2_PIN);

	// Relay actuator pin
	RELAY_DDR_REG |= _BV(2);
}

void print_num(uint8_t x, uint8_t y, uint16_t num, uint8_t digits, uint8_t leading_zeros) {
	uint8_t x_coord = x + (digits * 5) - 5;

	for (int i = 0; i < digits; i++) {
		uint8_t digit = num % 10;
		num /= 10;
		if (num != 0 || digit != 0 || leading_zeros || i == 0) {
			drawletter(x_coord, y, LCD_0 + digit);
		}

		x_coord -= 5;
	}
}

const uint8_t oven_string[] PROGMEM = {
  LCD_O, LCD_V, LCD_E, LCD_N, LCD_COLON, LCD_END
};

const uint8_t heat_string[] PROGMEM = {
  LCD_H, LCD_E, LCD_A, LCD_T, LCD_SPACE, LCD_O, LCD_N, LCD_END
};

uint8_t heating = 0;

int main()
{
	setup();

  /* Allow voltages to settle */
	_delay_ms(200);

	start_clock();

	//video_vline(20, 20, 40, 1);

	uint8_t count = 0;
	while (1) {
		uint16_t temp = max6675_read_raw();

		uint16_t temp2 = adc_read(8); // Internal temperature sensor

		// Clip temperature to displayable value 999.75
		if (temp > 3999) {
			temp = 3999;
		}

		// Redraw screen --------------------------------
		
		// Clear everything
		video_rect(0, 0, 128, 64, 0);

		// Oven temperature
		drawstring(0, 0, oven_string);
		print_num(31, 0, temp / 4, 3, 0);
		drawletter(31 + 15, 0, LCD_PERIOD);
		print_num(31 + 18, 0, 25 * (temp % 4), 2, 1);
		drawletter(31 + 28, 0, LCD_DEGREES);
		drawletter(31 + 32, 0, LCD_C);

		// AVR internal core temperature sensor
		print_num(74, 0, temp2, 3, 0);
		drawletter(74 + 15, 0, LCD_DEGREES);
		drawletter(74 + 19, 0, LCD_C);

		video_hline(0, 9, 128, 1);

		if (heating) {
			drawstring(0, 11, heat_string);

		  // Draw the heating time
			cli();
			// Get the 125Hz clock
			uint32_t ticks_copy = ticks;
			sei();
			ticks_copy /= 125;
			print_num(105, 0, ticks_copy / 60, 2, 0);
			drawletter(115, 0, LCD_COLON);
			print_num(118, 0, ticks_copy % 60, 2, 1);
		}

		print_num(0, 49, count++, 5, 0);

		// Simulated buttons
		drawletter(64, 38, button_flags & BUTTON_1 ? LCD_O : LCD_PERIOD);
		drawletter(64 + 10, 38, button_flags & BUTTON_2 ? LCD_O : LCD_PERIOD);
		// drawletter(64 + 20, 38, button_flags & BUTTON_3 ? LCD_O : LCD_PERIOD);

		ssd1306_update();

		// Handle events --------------------------------

		// TODO: this is better as a toggle on/off
		// Start
		if (test_button(BUTTON_1, 0)) {
			heating = 1;
			RELAY_PORT_REG |= _BV(RELAY_PIN);
			clear_clock();
		}

		// Stop
		if (test_button(BUTTON_2, 1)) {
			heating = 0;
			RELAY_PORT_REG &= ~_BV(RELAY_PIN);
		}


		// No need to update the screen all that often.
		_delay_ms(100);
	};

	return 0;
}
