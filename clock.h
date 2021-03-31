#ifndef __CLOCK_H
#define __CLOCK_H

#include <stdint.h>

extern volatile uint32_t ticks;

void init_clock();
void start_clock();
void clear_clock();

#endif
