#ifndef __MAX6675_H
#define __MAX6675_H

// Pin definitions - PORTC
// Pin C5 is Arduino pin analogue in 5
#define MAX6675_CS_PIN 4
// Pin C3 is Arduino analogue in 3
#define MAX6675_SCK_PIN 5
// Pin C4 is Arduino pin analogue in 4
#define MAX6675_SO_PIN 3

void max6675_init();
uint16_t max6675_read_raw();

#endif