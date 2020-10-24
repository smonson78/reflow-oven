#include <stdint.h>
#include <avr/io.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "ssd1306.h"

#define STARTADDR(x) do {\
  command(CMD_LOWER_COLUMN_ADDRESS & x);\
  command(CMD_HIGHER_COLUMN_ADDRESS | (x >> 4));\
  } while(0);

// Frame buffer
uint8_t video_buffer[8][128];

// Device-specific initialisation sequence
static const uint8_t initdata[] PROGMEM = {
  CMD_DISPLAY_OFF, 
  CMD_DISPLAY_CLOCK_RATIO, 0x80,
  CMD_MULTIPLEX_RATIO, 0x3f,    
  CMD_DISPLAY_OFFSET, 0x00,
  CMD_DISPLAY_START_LINE | 0,
  CMD_CHARGE_PUMP, 0x14,
  CMD_MEMORY_ADDRESSING_MODE, 0x00,
  CMD_SEGMENT_REMAP_ON,
  CMD_COM_OUT_SCAN_DIR_REMAP,
  CMD_COM_PINS_HW_CONFIG, 0x12,
  CMD_CONTRAST_CONTROL, 0x7f,
  CMD_PRECHARGE_PERIOD, 0xf1,
  CMD_VCOMH_DESELECT_LEVEL, 0x40,
  CMD_INVERSE_DISPLAY_OFF,
  CMD_DEACTIVATE_SCROLL,
  CMD_DISPLAY_ON
};

uint8_t data(uint8_t send) {
  cli();
  // Select data transfer
  PORTB |= _BV(PIN_CD); 
  uint8_t result = spi_transfer(send);
  sei();
  return result;
}

uint8_t command(uint8_t send) {
  cli();
  // Select command transfer
  PORTB &= ~(_BV(PIN_CD));  
  uint8_t result = spi_transfer(send);
  sei();
  return result;
}

void ssd1306_init() {

  spi_init();
  
  // Setup pin output direction for LED display reset and data/command select
  DDRB |= _BV(PIN_CD) | _BV(PIN_RESET);
  
  // Set RESET line HIGH
  PORTB |= _BV(PIN_RESET);

  ssd1306_reset();
}

void ssd1306_reset() {
  // Toggle the reset pin
  PORTB |= _BV(PIN_RESET);
  _delay_loop_2(F_CPU / 1000); // 2ms
  PORTB &= ~(_BV(PIN_RESET));
  _delay_loop_2(F_CPU / 400); // 10ms
  PORTB |= _BV(PIN_RESET);

  // Load the initialisation data into the device
  uint8_t i;
  for (i = 0; i < sizeof(initdata); i++) {
    command(pgm_read_byte(initdata + i));
  }
}

void ssd1306_clear()
{
  int bank, col;
  for (bank = 0; bank < 7; bank++)
  {
    for (col = 0; col < 128; col++)
      video_buffer[bank][col] = 0;
  }
  ssd1306_update();
}

void ssd1306_update_bank(uint8_t bank) {
  command(CMD_SELECT_BANK | bank);
  STARTADDR(0);
  for (uint8_t x = 0; x < 128; x++)
    data(video_buffer[bank][x]);
}

// Copy frame buffer from Arduino RAM into the OLED1306 memory
void ssd1306_update()
{
  for (uint8_t i = 0; i < 8; i++)
    ssd1306_update_bank(i);
}

void hscroll_bank_left(uint8_t bank, uint8_t start, uint8_t stop, int8_t dist, uint8_t mask)
{
  for (uint8_t col = start; col < stop; col++)
  {
    uint8_t result = video_buffer[bank][col] & ~mask;
    result |= video_buffer[bank][col - dist] & mask;
    video_buffer[bank][col] = result;
  }
}

void video_hscroll(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t dist)
{
  uint8_t bank = y / 8;
  
  // First row is masked
  hscroll_bank_left(bank, x, x + w, dist, 0xff << (y % 8));
  bank++;
  while (bank < (y + h - 1) / 8)
  {
    hscroll_bank_left(bank, x, x + w, dist, 0xff);
    bank++;
  }

  // Last row is masked
  hscroll_bank_left(bank, x, x + w, dist, 0xff >> ((y + h) % 8));
}

void video_hline(uint8_t x, uint8_t y, uint8_t w, uint8_t val)
{
  uint8_t bank = y / 8;
  uint8_t mask = 1 << (y % 8);

  for (uint8_t col = x; col < x + w; col++)
  {
    if (val)
      video_buffer[bank][col] |= mask;
    else
      video_buffer[bank][col] &= ~mask;
  }    
}

void video_vline(uint8_t x, uint8_t y, uint8_t h, uint8_t val)
{
  while (h-- > 0)
  {
    video_buffer[(y + h) / 8][x] |= 1 << ((y + h) % 8);
  }
}

void video_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t val)
{
  // This could be faster
  while (h-- > 0)
  {
    video_hline(x, y + h, w, val);
  }
}

// Put 8 pixels onto the screen in a vertical row
void video_drawbits(uint8_t x, uint8_t y, uint8_t bits)
{
  uint8_t row = y / 8;
  uint8_t offs = y % 8;

  video_buffer[row][x] |= bits << offs;
  if (offs)
    video_buffer[row + 1][x] |= bits >> (8 - offs);

}

// Src height must be a multiple of 8.
void video_blit(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t src_h, const uint8_t *src)
{
  uint8_t hbytes = (src_h + 7) / 8;
  uint8_t offset = (hbytes * 8) - src_h;

  for (uint8_t col = 0; col < w; col++)
  {
    const uint8_t *ptr = src + (hbytes * (col + 1)) - 1;
    
    for (uint8_t row = y; row < y + h; row += 8)
    {
      video_drawbits(x + col, row - offset, pgm_read_byte(ptr--));
    }
    
  }
}

void video_invert(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  while (h--)
  {
    uint8_t i;
    for (i = 0; i < w; i++)
    {
      uint8_t mask = 1 << ((y + h) % 8);
      video_buffer[(y + h) / 8][x + i] ^= mask;
    }
  }
}


