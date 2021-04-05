#ifndef __PREDICT_H
#define __PREDICT_H

#include <stdint.h>

float predict_future_temp(int temp, int seconds, uint16_t *temp_history, uint8_t temp_history_start);

#endif