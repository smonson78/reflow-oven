#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#include <util/delay_basic.h>
#include <stdint.h>

#include "clock.h"
#include "buttons.h"

#define PIN_RESET       1
#define PIN_CS          2
#define PIN_MOSI        3
#define PIN_CLK         5
#define PIN_CD		6

#define LOGO_PTR _binary_logo_tif_raw_start
#define FONT_PTR _binary_font8_tif_raw_start

extern const uint8_t LOGO_PTR[] PROGMEM;
extern const uint8_t FONT_PTR[] PROGMEM;

void delay(int time)
{
	while (time--)
	{
		/* 1msec delay */
		_delay_loop_2(F_CPU / 4000);
	}
}


void pin_setup() {
	
}

int main()
{
	pin_setup();

	/* Setup pin directions for LED display */
	//DDRB = 0b01101110;

	/* Initial condition for LED display */
	//PORTB = _BV(PIN_RESET);

	/* Setup SPI hardware */
	//SPCR = _BV(SPE) | _BV(MSTR);
	//SPSR = _BV(SPI2X);

	/* Allow voltages to settle */
	delay(200);

	/* Reset LED display */
	//PORTB &= ~(_BV(PIN_RESET));
	//delay(5);
	//PORTB |= _BV(PIN_RESET);

	//display_spi(0xAF);

	/* Line on line 20 */

	//int x, y;
	//for (y = 0; y < 4; y++)
	//{
		/* Set PAGE address 2-7 */
		//display_spi(0xB3 - y);

		/* Start address (0) */
		//display_spi(0x00);
		//display_spi(0x10);

		//for (x = 0; x < 132; x++)
		//{
			//uint8_t data = pgm_read_byte(LOGO_PTR + (x * 4) + y);
			//display_data(~data);
		//}

		//delay(1000);
	//}

	while (1)
		;

	return 0;
}
