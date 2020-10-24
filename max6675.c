#include <avr/io.h>
#include <util/delay_basic.h>
#include "max6675.h"

void max6675_init() {
  // Setup output pins. 
  // FIXME: make the port configurable as well
  DDRC |= _BV(MAX6675_CS_PIN) | _BV(MAX6675_SCK_PIN);

  // Set CS high
  PORTC |= _BV(MAX6675_CS_PIN);
}

uint16_t max6675_read_raw() {
  // Lower CS
  PORTC &= ~_BV(MAX6675_CS_PIN);

  // Wait 1us (from datasheet: at least 100ns)
  _delay_loop_2(F_CPU / 1000000 / 4);

  uint16_t acc = 0;

  // 16 bits of data
  for (uint8_t i = 0; i < 16; i++) {
    // Lower SCK
    PORTC &= ~_BV(MAX6675_SCK_PIN);

    // Move acc up one bit
    acc = acc << 1;

    // 4 clock cycles at 16MHz is 250ns (datasheet: at least 100ns)
    _delay_loop_2(10);

    // Sample data on the falling edge of SCK
    if (PINC & _BV(MAX6675_SO_PIN)) {
      acc |= 1;
    } 
    //acc |= (PINC & _BV(MAX6675_SO_PIN)) >> MAX6675_SO_PIN;

    // Raise SCK
    PORTC |= _BV(MAX6675_SCK_PIN);


    // 4 clock cycles at 16MHz is 250ns (datasheet: at least 100ns)
    _delay_loop_2(10);
  }

  // Raise CS
  PORTC |= _BV(MAX6675_CS_PIN);

  return acc; //(acc >> 3) & 0xfff; // 12 bit result
}