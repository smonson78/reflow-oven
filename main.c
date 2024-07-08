#include <string.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/boot.h>

#include <util/delay_basic.h>
#include <util/delay.h>

#include <stdint.h>
#include <string.h>

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

const uint8_t temp_profile[] = {25, 100, 150, 183, 235, 183};
const uint16_t temp_profile_time[] = {0, 30, 120, 150, 210, 240};
#define temp_profile_stages 6

// Heating on or off
uint8_t heating = 0;

// Find the total time of profile1
uint16_t total_time;

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

const uint8_t profile1_string[] PROGMEM = {
  LCD_P, LCD_R, LCD_O, LCD_F, LCD_I, LCD_L, LCD_E, LCD_SPACE, LCD_1, LCD_END
};

uint32_t read_clock() {
	cli();
	uint32_t result = ticks;
	sei();
	return result;
}

// Oven temperature and AVR core temp
uint16_t temp = 0;
uint16_t temp2 = 0;

//typedef struct {
//	uint16_t temperature;
//	uint8_t heat;
//} temp_history_t;

typedef uint16_t temp_history_t;

// Temperature history FIFO, 120s
temp_history_t temp_history[60];
uint8_t temp_history_start = 0;
uint32_t next_temp_history_unit;

void heat_on() {
	// Turn on heater pin
	RELAY_PORT_REG |= _BV(RELAY_PIN);

	// Reset the temperature FIFO
	memset(temp_history, 0, sizeof(temp_history));
	temp_history_start = 0;

	// This will cause the first temperature unit to be stored immediately
	next_temp_history_unit = 0;
	heating = 1;
}

void heat_off() {
	RELAY_PORT_REG &= ~_BV(RELAY_PIN);
	heating = 0;
}

// Foolow temperature profile on/off
uint8_t follow = 0;

uint8_t temp_at_position(uint16_t second) {

	// Which stage of the temperature profile is that?
	uint8_t stage = 0;
	while (stage < temp_profile_stages - 1 && second >= temp_profile_time[stage + 1]) {
		stage++;
	}

	// What's the target temperature at that point?
	uint8_t start_degrees = 0;
	uint8_t stop_degrees;
	start_degrees = temp_profile[stage];
	if (stage < temp_profile_stages) {
		stop_degrees = temp_profile[stage + 1];
	} else {
		stop_degrees = 0;
	}
	
	// Get the number of seconds in this stage
	uint16_t stage_time = temp_profile_time[stage == temp_profile_stages ? stage : stage + 1];
	stage_time -= temp_profile_time[stage];

	// Find the proportional target temperature
	uint16_t stage_proportion = second - temp_profile_time[stage];
	stage_proportion *= 100;
	if (stage_time > 0) {
		stage_proportion /= stage_time;
	}
	// stage_proportion is now the percentage of the way through the stage

	// Find the temp
	int16_t target_temp = stop_degrees - start_degrees;
	target_temp *= stage_proportion;
	target_temp /= 100;
	target_temp += start_degrees;
	return target_temp;
}

void draw_graph_slice(uint8_t x, uint8_t temp) {
	uint16_t line_height = temp * 100;
	line_height /= (25000 / 33); // get it into the range of 0-33 for drawing

	if (line_height < 33) {
		video_vline(x, 30 + (33 - line_height), line_height, 1);
	}
}

int main()
{
	setup();

  /* Allow voltages to settle */
	_delay_ms(200);

	total_time = temp_profile_time[temp_profile_stages - 1];

	start_clock();

	// This is the 0 point for displayed time.
	uint32_t clock_base = 0;

	// UI mode
	enum {
		MODE_TOGGLE,
		MODE_PROFILE1
	} mode = MODE_TOGGLE;

	// When to next refresh the temperature
	uint32_t next_temp = 0;

	// When to next redraw the screen
	uint32_t next_redraw = 0;

	memset(temp_history, 0, sizeof(temp_history));

	// Main loop
	while (1) {
		uint32_t time = read_clock();
		uint32_t op_seconds = (time - clock_base) / 125;
		uint16_t target_temp = temp_at_position(op_seconds);

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

		// Do oven logic if profile is running
		if (follow) {

			if (!heating && (temp/4) < target_temp - 2) {
				heat_on();
			} else if (heating && (temp/4) > target_temp) {
				heat_off();
			}

			// End the profile once time has elapsed
			if (op_seconds >= total_time) {
				follow = 0;
				heat_off();
			}
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

			// AVR internal core temperature sensor
			print_num(74, 0, target_temp, 3, 0);
			drawletter(74 + 15, 0, LCD_DEGREES);
			drawletter(74 + 19, 0, LCD_C);

			// Draw the heating/cooling time
			print_num(105, 0, op_seconds / 60, 2, 0);
			drawletter(115, 0, LCD_COLON);
			print_num(118, 0, op_seconds % 60, 2, 1);

			video_hline(0, 9, 128, 1);

			if (heating) {
				drawstring(84, 11, heat_string);
			}

			// Mode
			drawstring(0, 11, mode_string);
			if (mode == MODE_TOGGLE) {
				drawstring(32, 11, toggle_string);
			} else if (mode == MODE_PROFILE1) {
				drawstring(32, 11, profile1_string);
			}

			// The line over the graph
			video_hline(0, 29, 128, 1);

			// Draw the profile graph with a series of vertical lines representing the temperature
			for (uint8_t i = 0; i < 128; i++) {
				
				// Find the number of seconds into the profile we are
				uint16_t proportion = total_time * i;
				proportion /= 128;
				
				draw_graph_slice(i, temp_at_position(proportion));
			}

			// Draw current-time line on graph if profile is running
			if (follow) {
				uint16_t x = (op_seconds * 128) / total_time;
				if (x < 128) {
					video_vline(x, 30, 33, 1);
				}
			}

			ssd1306_update();

			// Update the display again in the future
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
		} else if (mode == MODE_PROFILE1) {
			if (test_button(BUTTON_1, 0)) {
				if (follow == 0) {
					// Start the reflow profile
					clock_base = time;
				follow = 1;
				} else {
					follow = 0;
					clock_base = time;
				}
			}
		}

		// MODE button
		if (test_button(BUTTON_2, 1)) {
			if (mode == MODE_TOGGLE) {
				mode = MODE_PROFILE1;
			} else {
				mode = MODE_TOGGLE;
			}
		}

		// No need to update continually
		//_delay_ms(10);
	};

	return 0;
}
