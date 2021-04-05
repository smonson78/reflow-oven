#include <stdio.h>
#include <string.h>
#include "predict.h"

int main(int argc, void **argv) {

  uint16_t temp_history[60];
  uint8_t temp_history_start;
  int current_temp = 31.5 * 4; // this is in quarter-degrees for the K-type thermocouple

  const int total_seconds = 60;

  printf("==== 30s burn scenario 1 (31.5 -> 59C)\n");
  memset(temp_history, 0, sizeof(temp_history));
  for (int i = 0; i < 15; i++) {
    temp_history[i] = 1;
  }
  temp_history_start = 15;
  for (int second = 0; second <= total_seconds; second += 30) {
    float future_temp = predict_future_temp(current_temp, second, temp_history, temp_history_start);
    printf("  After %d seconds, temp = %.1f C\n", second, future_temp);
  }

  printf("\n==== 30s burn scenario 2 (68 -> 92C)\n");
  current_temp = 68 * 4;
  for (int second = 0; second <= total_seconds; second += 30) {
    float future_temp = predict_future_temp(current_temp, second, temp_history, temp_history_start);
    printf("  After %d seconds, temp = %.1f C\n", second, future_temp);
  }

  printf("\n==== 30s burn scenario 3 (81.25 -> 106.5C)\n");
  current_temp = 81.25 * 4;
  for (int second = 0; second <= total_seconds; second += 30) {
    float future_temp = predict_future_temp(current_temp, second, temp_history, temp_history_start);
    printf("  After %d seconds, temp = %.1f C\n", second, future_temp);
  }

  printf("\n==== 30s burn scenario 4 (114 -> 134C)\n");
  current_temp = 114 * 4;
  for (int second = 0; second <= total_seconds; second += 30) {
    float future_temp = predict_future_temp(current_temp, second, temp_history, temp_history_start);
    printf("  After %d seconds, temp = %.1f C\n", second, future_temp);
  }

  printf("\n==== 10s burn scenario 1 (116 -> 114.75C)\n");
  memset(temp_history, 0, sizeof(temp_history));
  for (int i = 0; i < 5; i++) {
    temp_history[i] = 1;
  }
  temp_history_start = 5;

  current_temp = 116 * 4;
  for (int second = 0; second <= total_seconds; second += 30) {
    float future_temp = predict_future_temp(current_temp, second, temp_history, temp_history_start);
    printf("  After %d seconds, temp = %.1f C\n", second, future_temp);
  }

  printf("\n==== 100C scenario 1 (25 -> 100C)\n");
  memset(temp_history, 0, sizeof(temp_history));
  for (int i = 0; i < 1; i++) {
    temp_history[i] = 1;
  }
  temp_history_start = 1;

  current_temp = 25 * 4;
  for (int second = 0; second <= 90; second += 30) {
    float future_temp = predict_future_temp(current_temp, second, temp_history, temp_history_start);
    printf("  After %d seconds, temp = %.1f C\n", second, future_temp);
  }



  return 0;
}