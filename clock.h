#ifndef __CLOCK_H
#define __CLOCK_H

#include <stdint.h>

extern volatile uint32_t ticks, ticks_copy;

void init_clock(unsigned int frequency);
void start_clock();

#endif
