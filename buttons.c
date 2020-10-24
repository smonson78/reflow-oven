#include <stdint.h>
#include "buttons.h"

uint8_t button_flags = 0;
uint16_t ontime[6] = {0, 0, 0, 0, 0, 0};

uint8_t test_button(uint8_t mask, uint8_t index)
{
	if ((button_flags & mask) && (ontime[index] == 0))
	{
		/* Clear button flag */
		button_flags &= ~mask;
		return 1;
	}

	return 0;
}

void check_button(uint8_t index, uint8_t mask, uint8_t pin)
{
	if (pin)
	{
		/* Count for 4 seconds then stop to avoid over
		   flowing the integer */
		if (ontime[index] < 20000)
			ontime[index]++; /* Keep counting */

		/* This means the button has been marked as
		   "pressed" - it will be cleared when the pin
		   is no longer pressed and something happens.
		   Only mark this once - at the beginning. */
		if (ontime[index] == DEBOUNCE_DELAY)
			button_flags |= mask;
	}
	else
	{
		if (ontime[index] > DEBOUNCE_DELAY)
			ontime[index] = 0;
		else if (ontime[index] > 0)
			ontime[index]++;
	}
}

void read_buttons()
{
	/* BUTTONS ARE ACTIVE LOW */

	/* De-bounce the physical buttons */
	check_button(0, BUTTON_1,
		(BUTTON_PINREG & _BV(BUTTON_1_PIN)) == 0);
	check_button(1, BUTTON_2,
		(BUTTON_PINREG & _BV(BUTTON_2_PIN)) == 0);
	check_button(2, BUTTON_3,
		(BUTTON_PINREG & _BV(BUTTON_3_PIN)) == 0);

	/* 2 & 3 held down = button 4 */
	if ((ontime[1] == 1500 && ontime[2] > 1500)
		|| (ontime[1] > 1500 && ontime[2] == 1500)) 
	{
		/* Set flag for button pressed */
		button_flags |= BUTTON_4;

		/* Clear other flags */
		button_flags &= ~(BUTTON_2 | BUTTON_3);
	}

	/* 1, 2 & 3 held down = button 6 */
	if (ontime[0] >= 1500 && ontime[1] >= 1500
		&& ontime[2] >= 1500)
	{
		/* Set flag for button pressed if one 
		   of the buttons is exactly 1500 - to
		   prevent multiple presses. */
		if (ontime[0] == 1500 || ontime[1] == 1500
			|| ontime[2] == 1500)
		{
			button_flags |= BUTTON_6;

			/* Clear other flags */
			button_flags &= ~(BUTTON_1 | BUTTON_2 | BUTTON_3);
		}
	}
}

