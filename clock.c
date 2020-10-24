#include <stdint.h>
#include <avr/interrupt.h>

#include "clock.h"

volatile uint32_t ticks = 0, ticks_copy;

ISR(TIMER0_COMPA_vect)
{
	ticks++;	
}

/* Maintain a timer */
void init_clock(unsigned int frequency)
{
	/* Get timer1 into CTC mode (clear on compare match) */
	TCCR0A |= _BV(1); // 0 remains clear

	/* Select clock/8 prescaler */
	TCCR0B &= ~(_BV(0) & _BV(2));
	TCCR0B |= _BV(1); // 0 and 2 remain clear

	/* Frequency of clock is F_CPU / 8 / OCR0A */
	OCR0A = 200; /* 5000Hz for 8MHz (200uS period) */
}

void start_clock()
{
	/* Start clock */
	ticks = 0;
	TCNT0 = 0;

	/* Enable OCR0A compare match interrupt */
	TIMSK0 |= _BV(1);
}

