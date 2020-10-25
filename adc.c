#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"

// Internal 1.1v reference
#define ADC_REFERENCE (_BV(REFS1) | _BV(REFS0))

void adc_init()
{
	/* Use AREF reference, ADC channel 1 */
	ADMUX = ADC_REFERENCE;
	/* Use Vcc reference, ADC channel 0 */
	//ADMUX = _BV(REFS0);

	/* No pullups */
	//PORTC = 0;
	//DDRC = 0;

	/* Turn off digital input buffers on ADC0 and ADC1 */
	//DIDR0 |= _BV(ADC0D) | _BV(ADC1D);
	
	// Frequency range must be 50-200KHz.
	// 20MHz / 128 = 156.25KHz
	// 16MHz / 128 = 125KHz
	/* Enable ADC hardware (prescaler=128) and do first conversion */
	ADCSRA |= _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

	ADCSRA |= _BV(ADSC);
	while (ADCSRA & _BV(ADSC))
		;
}

uint16_t adc_read(uint8_t pin)
{
	ADMUX = ADC_REFERENCE | pin;
	/* Start conversion */
	ADCSRA |= _BV(ADSC);
	while (ADCSRA & _BV(ADSC))
		;

	return ADCW;
}

