#include <avr/io.h>

#include "spi.h"

void spi_init()
{
	/* Setup MOSI, SS and SCK pins for output. SS Must always be an output even if it 
	   isn't used. */
	DDRB |= _BV(2) | _BV(3) | _BV(5);

	/* Setup SPI hardware */
	SPCR = _BV(SPE) | _BV(MSTR);
	SPSR = _BV(SPI2X); // double speed
}

uint8_t spi_transfer(uint8_t send)
{
	SPDR = send;
	while (!(SPSR & _BV(SPIF)))
		;
	return SPDR;
}

uint16_t spi_transfer16(uint16_t send)
{
	uint16_t result;

	SPDR = (send >> 8) & 0xFF;
	while (!(SPSR & _BV(SPIF)))
		;
	result = SPDR;
	result <<= 8;
	SPDR = send & 0xFF;
	while (!(SPSR & _BV(SPIF)))
		;
	return result | SPDR;
}
