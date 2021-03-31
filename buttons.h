#ifndef __BUTTONS_H
#define __BUTTONS_H

#include <avr/io.h>

#define DEBOUNCE_DELAY		5
#define BUTTON_PINREG	PINC
#define BUTTON_1_PIN	0
#define BUTTON_2_PIN	1
#define BUTTON_3_PIN	2

#define BUTTON_1	(1 << 0)
#define BUTTON_2	(1 << 1)
#define BUTTON_3	(1 << 2)
#define BUTTON_4	(1 << 3)
#define BUTTON_5	(1 << 4)
#define BUTTON_6	(1 << 5)

extern uint8_t button_flags;
extern uint16_t ontime[3];

uint8_t test_button(uint8_t mask, uint8_t index);
void read_buttons();

#endif
