#ifndef __ADC_H
#define __ADC_H
#include <stdint.h>

void adc_init();
uint16_t adc_read(uint8_t pin);

#endif
