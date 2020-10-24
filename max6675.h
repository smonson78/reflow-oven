#ifndef __MAX6675_H
#define __MAX6675_H

// Pin definitions
#define MAX6675_CS_PIN 5
#define MAX6675_SCK_PIN 6
#define MAX6675_SO_PIN 4

void max6675_init();
uint16_t max6675_read_raw();

#endif