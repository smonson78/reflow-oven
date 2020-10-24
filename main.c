#include <string.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#include <util/delay_basic.h>
#include <stdint.h>

#include "clock.h"
#include "buttons.h"
#include "ssd1306.h"
#include "spi.h"
#include "font.h"

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
	drawstring(0, 0, string_test1);
	drawstring(0, 9, string_test2);

	ssd1306_update();

	while (1)
		;

	return 0;
}
