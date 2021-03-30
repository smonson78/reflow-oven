#include <stdint.h>
#include <avr/interrupt.h>

#include "clock.h"
#include "buttons.h"

volatile uint32_t ticks = 0, ticks_copy;

ISR(TIMER0_COMPA_vect)
{
	ticks++;
	read_buttons();
}

/* Maintain a timer */
void init_clock()
{
	/* Get timer0 into CTC mode (clear on compare match) */
	TCCR0A |= _BV(1); // 0 remains clear

	/* Select clock prescaler */
	//TCCR0B &= ~(_BV(CS00) | _BV(CS01) | _BV(CS02)); // Clear prescaler bits
	TCCR0B |= _BV(CS00) | _BV(CS02); // Set new prescaler bits - clock/1024

	/* Frequency of clock is F_CPU / 16000000 / OCR0A */
	OCR0A = 125; /* 16MHz / 1024 / 125 = 125Hz */
}

void start_clock()
{
	/* Start clock */
	ticks = 0;
	TCNT0 = 0;

	/* Enable OCR0A compare match interrupt */
	TIMSK0 |= _BV(1);
}

