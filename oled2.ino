#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stdint.h>

#include "ssd1306.h"

#include "stable.h"
INCLUDE_IMAGE(stable)

#include "pulse.h"
INCLUDE_IMAGE(pulse)

#include "font13.h"
INCLUDE_IMAGE(font13)
const uint8_t font13_widths[] PROGMEM = {7, 6, 6, 7, 7, 7, 7, 7, 7, 6, 8, 7, 4};

#include "weapon_ready.h"
INCLUDE_IMAGE(weapon_ready)

#include "zero.h"
INCLUDE_IMAGE(zero)

#include "scanning.h"
INCLUDE_IMAGE(scanning)

#include "blobs.h"
INCLUDE_IMAGE(blobs)

void font13_char(uint8_t x, uint8_t y, uint8_t c)
{
  uint16_t pos = 0;

  for (uint8_t i = 0; i < c; i++)
    pos += pgm_read_byte(&font13_widths[i]);
    
  screen.blit(x, y, 
    pgm_read_byte(&font13_widths[c]),
    13, 13, &img_font13[pos * 2]);  
}

void font13_num(uint8_t x, uint8_t y, uint16_t num, uint8_t len)
{
  uint8_t pos = x;
  for (uint8_t digit = 0; digit < len; digit++)
  {
    // Find value of digit
    uint8_t c = num;
    for (uint8_t i = 1; i < len - digit; i++)
    {
      c /= 10;
    }
    c %= 10;

    // Draw digit
    font13_char(pos, y, c);

    pos += pgm_read_byte(&font13_widths[c]);
    
    // Move along
    pos++;
  }
}

// "Pulse" screen code uses 1936 bytes of ROM
// This actually displays the "suit" screen
void pulse_screen()
{
  screen.blit(8, 48, 74, 8, 8, img_stable);

  // Lines
  screen.hline(3, 3, 122, 1);
  screen.hline(3, 60, 122, 1);
  screen.vline(3, 4, 56, 1);
  screen.vline(124, 4, 56, 1);
  screen.hline(7, 9, 75, 1);
  screen.hline(7, 44, 75, 1);
  screen.rect(87, 6, 2, 51, 1);
  screen.rect(90, 6, 2, 51, 1);
}

void pulse_loop()
{
  uint8_t heartbeat_phase = 0;
  uint8_t monitor_val[3] = {66, 32, 05};
  int8_t random_adjust[16] = {
    0, 1, 2, 3, 2, 4, 4, 1, 0, -1, -2, -2, -3, -2, -1, -1
  };
  uint16_t wheels[3] = {0, 0, 0};
  uint16_t wheel_max[3] = {417, 599, 749};

  while(1) {
    delay(40); // 25Hz
  
    // Update wheels
    if (++wheels[0] == wheel_max[0])
      wheels[0] = 0;
    if (++wheels[1] == wheel_max[1])
      wheels[1] = 0;
    if (++wheels[2] == wheel_max[2])
      wheels[2] = 2;
  
    // Move the heartbeat monitor along 1 pixel
    screen.hscroll(7, 10, 75, 34, -1);
  
    // Copy the next column from the heartbeat pulse image to the screen
    screen.blit(81, 12, 1, 32, 32, &img_pulse[heartbeat_phase * 4]);
  
    // Heatbeat image is 23 pixels wide.
    if (++heartbeat_phase == 23)
      heartbeat_phase = 0;
  
    // Monitor symbols
    font13_char(95, 7, 11); // +
    font13_char(95, 26, 10); // %
    font13_char(99, 45, 12); // degree
  
    // Monitor numbers
    screen.rect(105, 6, 15, 51, 0);
    font13_num(105, 7, monitor_val[0] + random_adjust[(wheels[0] * 16) / wheel_max[0]], 2);
    font13_num(105, 26, monitor_val[1] + random_adjust[(wheels[1] * 16) / wheel_max[1]], 2);
    font13_num(105, 45, monitor_val[2] + random_adjust[(wheels[2] * 16) / wheel_max[2]], 2);
  
    screen.update();
  }
}

void weapon_ready_screen() 
{
  screen.blit(14, 40, 100, 13, 13, img_weapon_ready);

  for (uint8_t i = 0; i < 10; i++)
    screen.blit(10 + (i * 11), 8, 9, 18, 18, img_zero);

  // Outer box
  screen.hline(3, 3, 122, 1);
  screen.hline(3, 60, 122, 1);
  screen.vline(3, 4, 56, 1);
  screen.vline(124, 4, 56, 1);

  // Middle lines
  screen.hline(9, 31, 109, 1);
  screen.hline(9, 33, 109, 1);
  screen.update();
}

void weapon_ready_loop()
{
  // Stop
  while (1)
    ;
}

void scanning_screen() {
  screen.blit(0, 0, 128, 64, 64, img_scanning);
  screen.update();
}

void scanning_loop() {
  static uint8_t flash_timer = 0;
  static uint8_t blobs = 0;
  while (1)
  {
    delay(40); // 25Hz

    // Flash "SCANNING"
#if 1
    // 1st version: flash the word SCANNING by toggling it on and off
    if (++flash_timer == 6) {
      screen.rect(23, 7, 80, 10, 0);
    } else if (flash_timer == 12) {
      screen.blit(23, 0, 79, 17, 64, img_scanning + (8 * 23));
      flash_timer = 0;
    }
#else
    // 2nd version: flash the word SCANNING by repeatedly reverse-videoing it
    if (++flash_timer == 6) {
      screen.invert(21, 6, 84, 12);
      flash_timer = 0;
    }
#endif

    // Jumble up the scanning symbols, whatever they are...
    if (++blobs % 2 != 0)
    {
      screen.rect(5, 44, 119, 16, 0);
      for (uint8_t i = 0; i < 7; i++)
      {
        // Pick a number from 1 to 7 that's not too obviously a pattern
        uint8_t p = (((blobs / 2) * 3) + (i * 2)) % 7;

        // Paste a "random" 16x16 block into one of the 7 positions
        screen.blit(5 + (i * 17), 44, 
          16, 16, 16, 
          &img_blobs[p * 32]);
      }
    }

    screen.update();
  }
}

void setup () {

  /* Setup hardware */
  SPI.begin();
  
  /* Allow voltages to settle */
  delay(100);

  /* Reset LED display */
  screen.init();
  screen.clear();
  
}

void loop()
{
  // These need to depend on switches or something on startup
  if (false)
  {
    pulse_screen();
    pulse_loop();
  }
  else if (false)
  {
    weapon_ready_screen();
    weapon_ready_loop();
  }
  else
  {
    scanning_screen();
    scanning_loop();
  }
}

