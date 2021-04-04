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

const uint8_t mode_string[] PROGMEM = {
  LCD_M, LCD_O, LCD_D, LCD_E, LCD_COLON, LCD_END
};

const uint8_t toggle_string[] PROGMEM = {
  LCD_T, LCD_O, LCD_G, LCD_G, LCD_L, LCD_E, LCD_END
};

const uint8_t quanta_string[] PROGMEM = {
  LCD_Q, LCD_U, LCD_A, LCD_N, LCD_T, LCD_A, LCD_END
};

uint32_t read_clock() {
	cli();
	uint32_t result = ticks;
	sei();
	return result;
}

void heat_on() {
	RELAY_PORT_REG |= _BV(RELAY_PIN);
}

void heat_off() {
	RELAY_PORT_REG &= ~_BV(RELAY_PIN);
}

int main()
{
	setup();

  /* Allow voltages to settle */
	_delay_ms(200);

	start_clock();

	// Heating on or off
	uint8_t heating = 0;

	// This is the 0 point for displayed time.
	uint32_t clock_base = 0;

	// UI mode
	enum {
		MODE_TOGGLE,
		MODE_QUANTA,
	} mode = MODE_TOGGLE;

	// Oven temperature and AVR core temp
	uint16_t temp = 0;
	uint16_t temp2 = 0;

	// When to next refresh the temperature
	uint32_t next_temp = 0;

	// When to next redraw the screen
	uint32_t next_redraw = 0;

	// Quanta end time
	uint32_t quanta_end;

	// Main loop
	while (1) {
		uint32_t time = read_clock();

		// Refresh the temperatures
		if (next_temp <= time) {
			temp = max6675_read_raw();

			// Clip temperature to displayable value 999.75
			if (temp > 3999) {
				temp = 3999;
			}

			temp2 = adc_read(8); // Internal temperature sensor
			temp2 -= 345;

			// Read the temperature again in the future
			next_temp = time + 50;
		}

		// Redraw screen --------------------------------
		if (next_redraw <= time) {
			// Clear everything
			video_rect(0, 0, 128, 64, 0);

			// Oven temperature
			drawstring(0, 0, oven_string);
			print_num(31, 0, temp / 4, 3, 0);
			drawletter(31 + 15, 0, LCD_PERIOD);
			print_num(31 + 18, 0, 25 * (temp % 4), 2, 1);
			drawletter(31 + 28, 0, LCD_DEGREES);
			drawletter(31 + 32, 0, LCD_C);

			// Mode
			drawstring(0, 24, mode_string);
			if (mode == MODE_TOGGLE) {
				drawstring(32, 24, toggle_string);
			} else if (mode == MODE_QUANTA) {
				drawstring(32, 24, quanta_string);
			}

			// AVR internal core temperature sensor
			print_num(74, 0, temp2, 3, 0);
			drawletter(74 + 15, 0, LCD_DEGREES);
			drawletter(74 + 19, 0, LCD_C);

			video_hline(0, 9, 128, 1);

			if (heating) {
				drawstring(0, 11, heat_string);
			}

			// Draw the heating/cooling time
			uint32_t ticks_copy = read_clock();
			ticks_copy -= clock_base;
			ticks_copy /= 125;
			print_num(105, 0, ticks_copy / 60, 2, 0);
			drawletter(115, 0, LCD_COLON);
			print_num(118, 0, ticks_copy % 60, 2, 1);

			ssd1306_update();

			// Read the temperature again in the future
			next_redraw = time + 25;
		}

		// Handle events --------------------------------

		if (mode == MODE_TOGGLE) {
			// Start/stop
			if (test_button(BUTTON_1, 0)) {
				heating = heating ? 0 : 1;
				if (heating) {
					heat_on();
				} else {
					heat_off();
				}
				clock_base = time;
			}
		} else if (mode == MODE_QUANTA) {
			// 30s heater "burn"
			if (test_button(BUTTON_1, 0) && heating == 0) {
				heat_on();
				heating = 1;
				clock_base = time;
				quanta_end = time + (125 * 30);
			}

			if (heating && quanta_end <= time) {
				heat_off();
				heating = 0;
				clock_base = time;
			}
		}

		// MODE button
		if (test_button(BUTTON_2, 1)) {
			if (mode == MODE_TOGGLE) {
				mode = MODE_QUANTA;
			} else {
				mode = MODE_TOGGLE;
			}
		}

		// No need to update continually
		//_delay_ms(10);
	};

	return 0;
}
