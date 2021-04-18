#include <stdint.h>
#include <stdio.h>

#include "predict.h"

uint8_t heating_table[66] = {
	0, 6, 12, 18, 24, 30, 36, 43, 49, 55, 61, 68, 75, 82, 87, 91, 97, 101, 105, 110, 114,
	119, 124, 128, 133, 138, 142, 147, 150, 153, 155, 158, 161, 164, 169, 172, 175, 178,
	181, 185, 188, 191, 194, 197, 199, 202, 205, 208, 212, 216, 219, 221, 225, 228, 231,
	233, 234, 236, 238, 241, 244, 245, 247, 248, 248, 249
};

// Degrees gained per unit of heating
const float heat_per_unit = 3.6;

// Degrees lost from cooling per second at a given temperature
float cooldown(float temp) {
  return (0.1 / 60) * temp;
}

// Working buffer for past temperatures
float work[33];

// Return the future temperature in degrees, based on history up to this point
float predict_future_temp(int temp, int seconds, uint16_t *temp_history, uint8_t temp_history_start) {

	// Start at the present time
	float acc = temp / 4.0;

	// Loop over the temperature history 
	for (int i = 0; i < sizeof(heating_table) / 2; i++) {

		// For each time unit, go back that many ago in the history table
		int index = temp_history_start;
		index -= i;
		if (index < 0) {
			index += 60; // Size of temp history
		}

    // If heat was on for that time unit
		//if (temp_history[index].heat) {
    if (temp_history[index]) {
      // Find the amount of heat that is already registering on the temperature probe
      uint8_t amt = heating_table[i * 2];

      // And the overflow amount that will have been applied X seconds in the future
      uint8_t table_index = (i * 2) + seconds;
      uint8_t future_amt;
      if (table_index >= sizeof(heating_table)) {
        future_amt = 255;
      } else {
			  future_amt = heating_table[table_index];
      }

      float heat_applied = future_amt - amt;
      heat_applied /= 255;
      heat_applied *= heat_per_unit;

      acc += heat_applied;
      //printf("  - heat from -%dU will be %.2fC %ds in the future\n", 
      //  i, heat_applied, seconds);
		}
	}

  // Remove future heat from cooling effect
  for (int i = 0; i < seconds; i++) {
    acc -= cooldown(acc);
  }

	return acc;
}
