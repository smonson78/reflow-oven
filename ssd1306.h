#ifndef __ssd1306_H
#define __ssd1306_H

#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>

/* Pin designations */

// No chip select for EmbeddedAdventures OLED screen
//#undef PIN_CS
#define PIN_RESET       0
#define PIN_CD          1
#define PIN_MOSI        3
#define PIN_CLK         5

/* SPI commands */
#define CMD_LOWER_COLUMN_ADDRESS    0x00
#define CMD_HIGHER_COLUMN_ADDRESS   0x10
#define CMD_DISPLAY_OFF             0xAE
#define CMD_DISPLAY_ON              0xAF
#define CMD_SELECT_BANK             0xB0
#define CMD_CONTRAST_CONTROL        0x81
#define CMD_DISPLAY_FROM_RAM        0xA4 // default
#define CMD_ENTIRE_DISPLAY_ON       0xA5
#define CMD_INVERSE_DISPLAY_OFF     0xA6
#define CMD_INVERSE_DISPLAY_ON      0xA7
#define CMD_MEMORY_ADDRESSING_MODE  0x20
#define CMD_COLUMN_ADDRESS          0x21
#define CMD_PAGE_ADDRESS            0x22
#define CMD_PAGE_START              0xB0
#define CMD_DISPLAY_START_LINE      0x40 // parameter mask 0b00111111, default 0
#define CMD_SEGMENT_REMAP_OFF       0xA0
#define CMD_SEGMENT_REMAP_ON        0xA1
#define CMD_MULTIPLEX_RATIO         0xA8 // parameter mask 0b00111111, default 63
#define CMD_COM_OUT_SCAN_DIR_NORMAL 0xC0 // default
#define CMD_COM_OUT_SCAN_DIR_REMAP  0xC8
#define CMD_DISPLAY_OFFSET          0xD3
#define CMD_COM_PINS_HW_CONFIG      0xDA
#define CMD_DISPLAY_CLOCK_RATIO     0xD5
#define CMD_PRECHARGE_PERIOD        0xD9
#define CMD_VCOMH_DESELECT_LEVEL    0xDB
#define CMD_DEACTIVATE_SCROLL       0x2E
#define CMD_ACTIVATE_SCROLL         0x2F
#define CMD_CHARGE_PUMP             0x8D


class SSD1306 {
  private:
    static uint8_t data(uint8_t send);
    static uint8_t command(uint8_t send);
    
  public:
    // This is the video frame buffer in RAM, which will be copied into the OLED screen.
    // It is 1024 bytes in size.
    uint8_t buffer[8][128];

    void init();
    void reset();
    void clear();
    void update_bank(uint8_t bank);
    void update();

    // Video functions
  private:
    void drawbits(uint8_t x, uint8_t y, uint8_t bits);
    void hscroll_bank_left(uint8_t bank, uint8_t start, uint8_t stop, 
      int8_t dist, uint8_t mask);
  public:  
    void hline(uint8_t x, uint8_t y, uint8_t w, uint8_t val);
    void vline(uint8_t x, uint8_t y, uint8_t h, uint8_t val);
    void rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t val);
    void hscroll(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t dist);
    void blit(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t src_h, 
      const uint8_t *src);
    void invert(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

};

extern SSD1306 screen;

#define STARTADDR(x) do {\
  command(CMD_LOWER_COLUMN_ADDRESS & x);\
  command(CMD_HIGHER_COLUMN_ADDRESS | (x >> 4));\
  } while(0);

#endif
