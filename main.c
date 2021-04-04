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


uint8_t heating = 0;

int main()
{
	setup();

  /* Allow voltages to settle */
	_delay_ms(200);

	start_clock();

	//video_vline(20, 20, 40, 1);

	uint8_t count = 0;
	enum {
		MODE_TOGGLE,
		MODE_QUANTA,
	} mode = MODE_TOGGLE;

	while (1) {
		uint16_t temp = max6675_read_raw();

		uint16_t temp2 = adc_read(8); // Internal temperature sensor
		temp2 -= 345;

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
		cli();
		// Get the 125Hz clock
		uint32_t ticks_copy = ticks;
		sei();
		ticks_copy /= 125;
		print_num(105, 0, ticks_copy / 60, 2, 0);
		drawletter(115, 0, LCD_COLON);
		print_num(118, 0, ticks_copy % 60, 2, 1);

		//print_num(0, 49, count++, 5, 0);

		// Simulated buttons
		//drawletter(64, 38, button_flags & BUTTON_1 ? LCD_O : LCD_PERIOD);
		//drawletter(64 + 10, 38, button_flags & BUTTON_2 ? LCD_O : LCD_PERIOD);
		// drawletter(64 + 20, 38, button_flags & BUTTON_3 ? LCD_O : LCD_PERIOD);

		ssd1306_update();

		// Handle events --------------------------------

		if (mode == MODE_TOGGLE) {
			// Start/stop
			if (test_button(BUTTON_1, 0)) {
				heating = heating ? 0 : 1;
				if (heating) {
					RELAY_PORT_REG |= _BV(RELAY_PIN);
				} else {
					RELAY_PORT_REG &= ~_BV(RELAY_PIN);	
				}
				clear_clock();
			}
		} else if (mode == MODE_QUANTA) {
			// 10s heater "burn"
			if (test_button(BUTTON_1, 0) && heating == 0) {
				RELAY_PORT_REG |= _BV(RELAY_PIN);
				heating = 1;
				count = 20;
				clear_clock();
			}

			if (heating) {
				if (count == 0) {
					heating = 0;
					RELAY_PORT_REG &= ~_BV(RELAY_PIN);	
					clear_clock();
				} else {
					count -= 1;
				}
			}
		}

		// Stop
		if (test_button(BUTTON_2, 1)) {
			// mode button
			if (mode == MODE_TOGGLE) {
				mode = MODE_QUANTA;
			} else {
				mode = MODE_TOGGLE;
			}
		}


		// No need to update the screen all that often.
		_delay_ms(500);
	};

	return 0;
}
