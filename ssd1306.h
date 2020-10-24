#ifndef __ssd1306_H
#define __ssd1306_H

#include <stdint.h>
#include "spi.h"

/* Pin designations */
#define PIN_RESET       0 // "RESET"
#define PIN_CD          1 // "DC"

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


// This is the video frame buffer in RAM, which will be copied into the OLED screen.
// It is 1024 bytes in size.
extern uint8_t video_buffer[8][128];

void ssd1306_init();
void ssd1306_reset();
void ssd1306_clear();
void ssd1306_update_bank(uint8_t bank);
void ssd1306_update();

void video_hline(uint8_t x, uint8_t y, uint8_t w, uint8_t val);
void video_vline(uint8_t x, uint8_t y, uint8_t h, uint8_t val);
void video_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t val);
void video_hscroll(uint8_t x, uint8_t y, uint8_t w, uint8_t h, int8_t dist);
void video_blit(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t src_h, const uint8_t *src);
void video_drawbits(uint8_t x, uint8_t y, uint8_t bits);
void video_invert(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

#endif
